/***************************************************************************/
/*                                                                         */
/*  ftgrays.c                                                              */
/*                                                                         */
/*    A new `perfect' anti-aliasing renderer (body).                       */
/*                                                                         */
/*  Copyright 2000-2003, 2005-2014 by                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, FTL.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

/*************************************************************************/
/*                                                                       */
/* This is a new anti-aliasing scan-converter for FreeType 2.  The       */
/* algorithm used here is _very_ different from the one in the standard  */
/* `ftraster' module.  Actually, `ftgrays' computes the _exact_          */
/* coverage of the outline on each pixel cell.                           */
/*                                                                       */
/* It is based on ideas that I initially found in Raph Levien's          */
/* excellent LibArt graphics library (see http://www.levien.com/libart   */
/* for more information, though the web pages do not tell anything       */
/* about the renderer; you'll have to dive into the source code to       */
/* understand how it works).                                             */
/*                                                                       */
/* Note, however, that this is a _very_ different implementation         */
/* compared to Raph's.  Coverage information is stored in a very         */
/* different way, and I don't use sorted vector paths.  Also, it doesn't */
/* use floating point values.                                            */
/*                                                                       */
/* This renderer has the following advantages:                           */
/*                                                                       */
/* - It doesn't need an intermediate bitmap.  Instead, one can supply a  */
/*   callback function that will be called by the renderer to draw gray  */
/*   spans on any target surface.  You can thus do direct composition on */
/*   any kind of bitmap, provided that you give the renderer the right   */
/*   callback.                                                           */
/*                                                                       */
/* - A perfect anti-aliaser, i.e., it computes the _exact_ coverage on   */
/*   each pixel cell.                                                    */
/*                                                                       */
/* - It performs a single pass on the outline (the `standard' FT2        */
/*   renderer makes two passes).                                         */
/*                                                                       */
/* - It can easily be modified to render to _any_ number of gray levels  */
/*   cheaply.                                                            */
/*                                                                       */
/* - For small (< 20) pixel sizes, it is faster than the standard        */
/*   renderer.                                                           */
/*                                                                       */
/*************************************************************************/

#include "plutovg-ft-raster.h"
// #include "plutovg-ft-math.h" //NOTE(Taylor): Everything that was in here can be found in FreeType headers like fttrigon.h, ftobjs.h, etc.

#include <setjmp.h>

#define pvg_ft_setjmp   setjmp
#define pvg_ft_longjmp  longjmp
#define pvg_ft_jmp_buf  jmp_buf

#include <stddef.h>

typedef ptrdiff_t  PVG_FT_PtrDist;

#define ErrRaster_Invalid_Mode      -2
#define ErrRaster_Invalid_Outline   -1
#define ErrRaster_Invalid_Argument  -3
#define ErrRaster_Memory_Overflow   -4
#define ErrRaster_OutOfMemory       -6

#include <stdlib.h>
#include <limits.h>

#define PVG_FT_MINIMUM_POOL_SIZE 8192

#define PVG_RAS_ARG   PVG_PWorker  worker //NOTE(Taylor): Added PVG_ prefix to avoid naming conflicts in unity build
#define PVG_RAS_ARG_  PVG_PWorker  worker, //NOTE(Taylor): Added PVG_ prefix to avoid naming conflicts in unity build

#define PVG_RAS_VAR   worker //NOTE(Taylor): Added PVG_ prefix to avoid naming conflicts in unity build
#define PVG_RAS_VAR_  worker, //NOTE(Taylor): Added PVG_ prefix to avoid naming conflicts in unity build

#define PVG_ras       (*worker) //NOTE(Taylor): Added PVG_ prefix to avoid naming conflicts in unity build

  /* must be at least 6 bits! */
#define PVG_PIXEL_BITS  8 //NOTE(Taylor): Added PVG_ prefix to avoid naming conflicts in unity build

#define PVG_ONE_PIXEL       ( 1L << PVG_PIXEL_BITS ) //NOTE(Taylor): Added PVG_ prefix to avoid naming conflicts in unity build
#define PVG_TRUNC( x )      (PVG_TCoord)( (x) >> PVG_PIXEL_BITS ) //NOTE(Taylor): Added PVG_ prefix to avoid naming conflicts in unity build
#define PVG_FRACT( x )      (PVG_TCoord)( (x) & ( PVG_ONE_PIXEL - 1 ) ) //NOTE(Taylor): Added PVG_ prefix to avoid naming conflicts in unity build

#if PVG_PIXEL_BITS >= 6
#define PVG_UPSCALE( x )    ( (x) * ( PVG_ONE_PIXEL >> 6 ) ) //NOTE(Taylor): Added PVG_ prefix to avoid naming conflicts in unity build
#define PVG_DOWNSCALE( x )  ( (x) >> ( PVG_PIXEL_BITS - 6 ) ) //NOTE(Taylor): Added PVG_ prefix to avoid naming conflicts in unity build
#else
#define PVG_UPSCALE( x )    ( (x) >> ( 6 - PVG_PIXEL_BITS ) )
#define PVG_DOWNSCALE( x )  ( (x) * ( 64 >> PVG_PIXEL_BITS ) )
#endif

/* Compute `dividend / divisor' and return both its quotient and     */
/* remainder, cast to a specific type.  This macro also ensures that */
/* the remainder is always positive.                                 */
#define PVG_FT_DIV_MOD( type, dividend, divisor, quotient, remainder ) \
FT_BEGIN_STMNT                                                   \
  (quotient)  = (type)( (dividend) / (divisor) );                \
  (remainder) = (type)( (dividend) % (divisor) );                \
  if ( (remainder) < 0 )                                         \
  {                                                              \
    (quotient)--;                                                \
    (remainder) += (type)(divisor);                              \
  }                                                              \
FT_END_STMNT

  /* These macros speed up repetitive divisions by replacing them */
  /* with multiplications and right shifts.                       */
#define PVG_FT_UDIVPREP( b )                                       \
  long  b ## _r = (long)( ULONG_MAX >> PVG_PIXEL_BITS ) / ( b )
#define PVG_FT_UDIV( a, b )                                        \
  ( ( (unsigned long)( a ) * (unsigned long)( b ## _r ) ) >>   \
    ( sizeof( long ) * CHAR_BIT - PVG_PIXEL_BITS ) )


  /*************************************************************************/
  /*                                                                       */
  /*   TYPE DEFINITIONS                                                    */
  /*                                                                       */

  /* don't change the following types to FT_Int or FT_Pos, since we might */
  /* need to define them to "float" or "double" when experimenting with   */
  /* new algorithms                                                       */

  typedef long   PVG_TCoord;   /* integer scanline/pixel coordinate */ //NOTE(Taylor): Added PVG_ prefix to avoid naming conflicts in unity build
  typedef long   PVG_TPos;     /* sub-pixel coordinate              */ //NOTE(Taylor): Added PVG_ prefix to avoid naming conflicts in unity build
  typedef long   PVG_TArea ;   /* cell areas, coordinate products   */ //NOTE(Taylor): Added PVG_ prefix to avoid naming conflicts in unity build

  /* maximal number of gray spans in a call to the span callback */
#define PVG_FT_MAX_GRAY_SPANS  256 //NOTE(Taylor): This is 256 while FT_MAX_GRAY_SPANS is 16


  typedef struct PVG_TCell_*  PVG_PCell;

  //NOTE(Taylor): Added PVG_ prefix to avoid naming conflicts in unity build
  typedef struct  PVG_TCell_
  {
    int    x;
    int    cover;
    PVG_TArea  area;
    PVG_PCell  next;

  } PVG_TCell;


  //NOTE(Taylor): Added PVG_ prefix to avoid naming conflicts in unity build
  typedef struct  PVG_TWorker_
  {
    PVG_TCoord  ex, ey;
    PVG_TPos    min_ex, max_ex;
    PVG_TPos    min_ey, max_ey;
    PVG_TPos    count_ex, count_ey;

    PVG_TArea   area;
    int     cover;
    int     invalid;

    PVG_PCell   cells;
    PVG_FT_PtrDist     max_cells;
    PVG_FT_PtrDist     num_cells;

    PVG_TPos    x,  y;

    FT_Outline  outline;
    FT_BBox     clip_box;

    int clip_flags;
    int clipping;

    PVG_FT_Span     gray_spans[PVG_FT_MAX_GRAY_SPANS];
    int         num_gray_spans;
    int         skip_spans;

    PVG_FT_Raster_Span_Func  render_span;
    void*                render_span_data;

    int  band_size;
    int  band_shoot;

    pvg_ft_jmp_buf  jump_buffer;

    void*       buffer;
    long        buffer_size;

    PVG_PCell*     ycells;
    PVG_TPos       ycount;
  } PVG_TWorker, *PVG_PWorker; //NOTE(Taylor): Added PVG_ prefix to avoid naming conflicts in unity build


  /*************************************************************************/
  /*                                                                       */
  /* Initialize the cells table.                                           */
  /*                                                                       */
  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static void
  pvg_gray_init_cells( PVG_RAS_ARG_ void*  buffer,
                            long   byte_size )
  {
    PVG_ras.buffer      = buffer;
    PVG_ras.buffer_size = byte_size;

    PVG_ras.ycells      = (PVG_PCell*) buffer;
    PVG_ras.cells       = NULL;
    PVG_ras.max_cells   = 0;
    PVG_ras.num_cells   = 0;
    PVG_ras.area        = 0;
    PVG_ras.cover       = 0;
    PVG_ras.invalid     = 1;
  }


  /*************************************************************************/
  /*                                                                       */
  /* Compute the outline bounding box.                                     */
  /*                                                                       */
  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static void
  pvg_gray_compute_cbox( PVG_RAS_ARG )
  {
    FT_Outline*  outline = &PVG_ras.outline;
    FT_Vector*   vec     = outline->points;
    FT_Vector*   limit   = vec + outline->n_points;


    if ( outline->n_points <= 0 )
    {
      PVG_ras.min_ex = PVG_ras.max_ex = 0;
      PVG_ras.min_ey = PVG_ras.max_ey = 0;
      return;
    }

    PVG_ras.min_ex = PVG_ras.max_ex = vec->x;
    PVG_ras.min_ey = PVG_ras.max_ey = vec->y;

    vec++;

    for ( ; vec < limit; vec++ )
    {
      PVG_TPos  x = vec->x;
      PVG_TPos  y = vec->y;


      if ( x < PVG_ras.min_ex ) PVG_ras.min_ex = x;
      if ( x > PVG_ras.max_ex ) PVG_ras.max_ex = x;
      if ( y < PVG_ras.min_ey ) PVG_ras.min_ey = y;
      if ( y > PVG_ras.max_ey ) PVG_ras.max_ey = y;
    }

    /* truncate the bounding box to integer pixels */
    PVG_ras.min_ex = PVG_ras.min_ex >> 6;
    PVG_ras.min_ey = PVG_ras.min_ey >> 6;
    PVG_ras.max_ex = ( PVG_ras.max_ex + 63 ) >> 6;
    PVG_ras.max_ey = ( PVG_ras.max_ey + 63 ) >> 6;
  }


  /*************************************************************************/
  /*                                                                       */
  /* Record the current cell in the table.                                 */
  /*                                                                       */
  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static PVG_PCell
  pvg_gray_find_cell( PVG_RAS_ARG )
  {
    PVG_PCell  *pcell, cell;
    PVG_TPos    x = PVG_ras.ex;


    if ( x > PVG_ras.count_ex )
      x = PVG_ras.count_ex;

    pcell = &PVG_ras.ycells[PVG_ras.ey];
    for (;;)
    {
      cell = *pcell;
      if ( cell == NULL || cell->x > x )
        break;

      if ( cell->x == x )
        goto Exit;

      pcell = &cell->next;
    }

    if ( PVG_ras.num_cells >= PVG_ras.max_cells )
      pvg_ft_longjmp( PVG_ras.jump_buffer, 1 );

    cell        = PVG_ras.cells + PVG_ras.num_cells++;
    cell->x     = x;
    cell->area  = 0;
    cell->cover = 0;

    cell->next  = *pcell;
    *pcell      = cell;

  Exit:
    return cell;
  }

  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static void
  pvg_gray_record_cell( PVG_RAS_ARG )
  {
    if ( PVG_ras.area | PVG_ras.cover )
    {
      PVG_PCell  cell = pvg_gray_find_cell( PVG_RAS_VAR );


      cell->area  += PVG_ras.area;
      cell->cover += PVG_ras.cover;
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /* Set the current cell to a new position.                               */
  /*                                                                       */
  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static void
  pvg_gray_set_cell( PVG_RAS_ARG_ PVG_TCoord  ex,
                          PVG_TCoord  ey )
  {
    /* Move the cell pointer to a new position.  We set the `invalid'      */
    /* flag to indicate that the cell isn't part of those we're interested */
    /* in during the render phase.  This means that:                       */
    /*                                                                     */
    /* . the new vertical position must be within min_ey..max_ey-1.        */
    /* . the new horizontal position must be strictly less than max_ex     */
    /*                                                                     */
    /* Note that if a cell is to the left of the clipping region, it is    */
    /* actually set to the (min_ex-1) horizontal position.                 */

    /* All cells that are on the left of the clipping region go to the */
    /* min_ex - 1 horizontal position.                                 */
    ey -= PVG_ras.min_ey;

    if ( ex > PVG_ras.max_ex )
      ex = PVG_ras.max_ex;

    ex -= PVG_ras.min_ex;
    if ( ex < 0 )
      ex = -1;

    /* are we moving to a different cell ? */
    if ( ex != PVG_ras.ex || ey != PVG_ras.ey )
    {
      /* record the current one if it is valid */
      if ( !PVG_ras.invalid )
        pvg_gray_record_cell( PVG_RAS_VAR );

      PVG_ras.area  = 0;
      PVG_ras.cover = 0;
      PVG_ras.ex    = ex;
      PVG_ras.ey    = ey;
    }

    PVG_ras.invalid = ( (unsigned int)ey >= (unsigned int)PVG_ras.count_ey ||
                                  ex >= PVG_ras.count_ex           );
  }


  /*************************************************************************/
  /*                                                                       */
  /* Start a new contour at a given cell.                                  */
  /*                                                                       */
  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static void
  pvg_gray_start_cell( PVG_RAS_ARG_ PVG_TCoord  ex,
                            PVG_TCoord  ey )
  {
    if ( ex > PVG_ras.max_ex )
      ex = (PVG_TCoord)( PVG_ras.max_ex );

    if ( ex < PVG_ras.min_ex )
      ex = (PVG_TCoord)( PVG_ras.min_ex - 1 );

    PVG_ras.area    = 0;
    PVG_ras.cover   = 0;
    PVG_ras.ex      = ex - PVG_ras.min_ex;
    PVG_ras.ey      = ey - PVG_ras.min_ey;
    PVG_ras.invalid = 0;

    pvg_gray_set_cell( PVG_RAS_VAR_ ex, ey );
  }

// The new render-line implementation is not yet used
#if 1

  /*************************************************************************/
  /*                                                                       */
  /* Render a scanline as one or more cells.                               */
  /*                                                                       */
  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static void
  pvg_gray_render_scanline( PVG_RAS_ARG_ PVG_TCoord  ey,
                                 PVG_TPos    x1,
                                 PVG_TCoord  y1,
                                 PVG_TPos    x2,
                                 PVG_TCoord  y2 )
  {
    PVG_TCoord  ex1, ex2, fx1, fx2, first, dy, delta, mod;
    PVG_TPos    p, dx;
    int     incr;


    ex1 = PVG_TRUNC( x1 );
    ex2 = PVG_TRUNC( x2 );

    /* trivial case.  Happens often */
    if ( y1 == y2 )
    {
      pvg_gray_set_cell( PVG_RAS_VAR_ ex2, ey );
      return;
    }

    fx1   = PVG_FRACT( x1 );
    fx2   = PVG_FRACT( x2 );

    /* everything is located in a single cell.  That is easy! */
    /*                                                        */
    if ( ex1 == ex2 )
      goto End;

    /* ok, we'll have to render a run of adjacent cells on the same */
    /* scanline...                                                  */
    /*                                                              */
    dx = x2 - x1;
    dy = y2 - y1;

    if ( dx > 0 )
    {
      p     = ( PVG_ONE_PIXEL - fx1 ) * dy;
      first = PVG_ONE_PIXEL;
      incr  = 1;
    } else {
      p     = fx1 * dy;
      first = 0;
      incr  = -1;
      dx    = -dx;
    }

    PVG_FT_DIV_MOD( PVG_TCoord, p, dx, delta, mod );

    PVG_ras.area  += (PVG_TArea)( fx1 + first ) * delta;
    PVG_ras.cover += delta;
    y1        += delta;
    ex1       += incr;
    pvg_gray_set_cell( PVG_RAS_VAR_ ex1, ey );

    if ( ex1 != ex2 )
    {
      PVG_TCoord  lift, rem;


      p = PVG_ONE_PIXEL * dy;
      PVG_FT_DIV_MOD( PVG_TCoord, p, dx, lift, rem );

      do
      {
        delta = lift;
        mod  += rem;
        if ( mod >= (PVG_TCoord)dx )
        {
          mod -= (PVG_TCoord)dx;
          delta++;
        }

        PVG_ras.area  += (PVG_TArea)( PVG_ONE_PIXEL * delta );
        PVG_ras.cover += delta;
        y1        += delta;
        ex1       += incr;
        pvg_gray_set_cell( PVG_RAS_VAR_ ex1, ey );
      } while ( ex1 != ex2 );
    }
    fx1 = PVG_ONE_PIXEL - first;

  End:
    dy = y2 - y1;

    PVG_ras.area  += (PVG_TArea)( ( fx1 + fx2 ) * dy );
    PVG_ras.cover += dy;
  }


  /*************************************************************************/
  /*                                                                       */
  /* Render a given line as a series of scanlines.                         */
  /*                                                                       */
  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static void
  pvg_gray_render_line( PVG_RAS_ARG_ PVG_TPos from_x, PVG_TPos from_y, PVG_TPos  to_x, PVG_TPos  to_y )
  {
    PVG_TCoord  ey1, ey2, fy1, fy2, first, delta, mod;
    PVG_TPos    p, dx, dy, x, x2;
    int     incr;

    ey1 = PVG_TRUNC( from_y );
    ey2 = PVG_TRUNC( to_y );     /* if (ey2 >= PVG_ras.max_ey) ey2 = PVG_ras.max_ey-1; */

    /* perform vertical clipping */
    if ( ( ey1 >= PVG_ras.max_ey && ey2 >= PVG_ras.max_ey ) ||
         ( ey1 <  PVG_ras.min_ey && ey2 <  PVG_ras.min_ey ) )
      return;

    fy1 = PVG_FRACT( from_y );
    fy2 = PVG_FRACT( to_y );

    /* everything is on a single scanline */
    if ( ey1 == ey2 )
    {
      pvg_gray_render_scanline( PVG_RAS_VAR_ ey1, from_x, fy1, to_x, fy2 );
      return;
    }

    dx = to_x - from_x;
    dy = to_y - from_y;

    /* vertical line - avoid calling pvg_gray_render_scanline */
    if ( dx == 0 )
    {
      PVG_TCoord  ex     = PVG_TRUNC( from_x );
      PVG_TCoord  two_fx = PVG_FRACT( from_x ) << 1;
      PVG_TPos    area, max_ey1;


      if ( dy > 0)
      {
        first = PVG_ONE_PIXEL;
      }
      else
      {
        first = 0;
      }

      delta      = first - fy1;
      PVG_ras.area  += (PVG_TArea)two_fx * delta;
      PVG_ras.cover += delta;

      delta = first + first - PVG_ONE_PIXEL;
      area  = (PVG_TArea)two_fx * delta;
      max_ey1 = PVG_ras.count_ey + PVG_ras.min_ey;
      if (dy < 0) {
        if (ey1 > max_ey1) {
          ey1 = (max_ey1 > ey2) ? max_ey1 : ey2;
          pvg_gray_set_cell( &PVG_ras, ex, ey1 );
        } else {
          ey1--;
          pvg_gray_set_cell( &PVG_ras, ex, ey1 );
        }
        while ( ey1 > ey2 && ey1 >= PVG_ras.min_ey)
        {
          PVG_ras.area  += area;
          PVG_ras.cover += delta;
          ey1--;

          pvg_gray_set_cell( &PVG_ras, ex, ey1 );
        }
        if (ey1 != ey2) {
          ey1 = ey2;
          pvg_gray_set_cell( &PVG_ras, ex, ey1 );
        }
      } else {
        if (ey1 < PVG_ras.min_ey) {
          ey1 = (PVG_ras.min_ey < ey2) ? PVG_ras.min_ey : ey2;
          pvg_gray_set_cell( &PVG_ras, ex, ey1 );
        } else {
          ey1++;
          pvg_gray_set_cell( &PVG_ras, ex, ey1 );
        }
        while ( ey1 < ey2 && ey1 < max_ey1)
        {
          PVG_ras.area  += area;
          PVG_ras.cover += delta;
          ey1++;

          pvg_gray_set_cell( &PVG_ras, ex, ey1 );
        }
        if (ey1 != ey2) {
          ey1 = ey2;
          pvg_gray_set_cell( &PVG_ras, ex, ey1 );
        }
      }

      delta      = (int)( fy2 - PVG_ONE_PIXEL + first );
      PVG_ras.area  += (PVG_TArea)two_fx * delta;
      PVG_ras.cover += delta;

      return;
    }

    /* ok, we have to render several scanlines */
    if ( dy > 0)
    {
      p     = ( PVG_ONE_PIXEL - fy1 ) * dx;
      first = PVG_ONE_PIXEL;
      incr  = 1;
    }
    else
    {
      p     = fy1 * dx;
      first = 0;
      incr  = -1;
      dy    = -dy;
    }

    /* the fractional part of x-delta is mod/dy. It is essential to */
    /* keep track of its accumulation for accurate rendering.       */
    PVG_FT_DIV_MOD( PVG_TCoord, p, dy, delta, mod );

    x = from_x + delta;
    pvg_gray_render_scanline( PVG_RAS_VAR_ ey1, from_x, fy1, x, (PVG_TCoord)first );

    ey1 += incr;
    pvg_gray_set_cell( PVG_RAS_VAR_ PVG_TRUNC( x ), ey1 );

    if ( ey1 != ey2 )
    {
      PVG_TCoord  lift, rem;


      p    = PVG_ONE_PIXEL * dx;
      PVG_FT_DIV_MOD( PVG_TCoord, p, dy, lift, rem );

      do
      {
        delta = lift;
        mod  += rem;
        if ( mod >= (PVG_TCoord)dy )
        {
          mod -= (PVG_TCoord)dy;
          delta++;
        }

        x2 = x + delta;
        pvg_gray_render_scanline( PVG_RAS_VAR_ ey1,
                                       x, PVG_ONE_PIXEL - first,
                                       x2, first );
        x = x2;

        ey1 += incr;
        pvg_gray_set_cell( PVG_RAS_VAR_ PVG_TRUNC( x ), ey1 );
      } while ( ey1 != ey2 );
    }

    pvg_gray_render_scanline( PVG_RAS_VAR_ ey1,
                                   x, PVG_ONE_PIXEL - first,
                                   to_x, fy2 );
  }


#else

  /*************************************************************************/
  /*                                                                       */
  /* Render a straight line across multiple cells in any direction.        */
  /*                                                                       */
  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static void
  pvg_gray_render_line( PVG_RAS_ARG_ PVG_TPos from_x, PVG_TPos from_y, PVG_TPos  to_x, PVG_TPos  to_y )
  {
    PVG_TPos    dx, dy, fx1, fy1, fx2, fy2;
    PVG_TCoord  ex1, ex2, ey1, ey2;


    ex1 = PVG_TRUNC( from_x );
    ex2 = PVG_TRUNC( to_x );
    ey1 = PVG_TRUNC( from_y );
    ey2 = PVG_TRUNC( to_y );

    /* perform vertical clipping */
    if ( ( ey1 >= PVG_ras.max_ey && ey2 >= PVG_ras.max_ey ) ||
         ( ey1 <  PVG_ras.min_ey && ey2 <  PVG_ras.min_ey ) )
      return;

    dx = to_x - from_x;
    dy = to_y - from_y;

    fx1 = PVG_FRACT( from_x );
    fy1 = PVG_FRACT( from_y );

    if ( ex1 == ex2 && ey1 == ey2 )       /* inside one cell */
      ;
    else if ( dy == 0 ) /* ex1 != ex2 */  /* any horizontal line */
    {
      ex1 = ex2;
      pvg_gray_set_cell( PVG_RAS_VAR_ ex1, ey1 );
    }
    else if ( dx == 0 )
    {
      if ( dy > 0 )                       /* vertical line up */
        do
        {
          fy2 = PVG_ONE_PIXEL;
          PVG_ras.cover += ( fy2 - fy1 );
          PVG_ras.area  += ( fy2 - fy1 ) * fx1 * 2;
          fy1 = 0;
          ey1++;
          pvg_gray_set_cell( PVG_RAS_VAR_ ex1, ey1 );
        } while ( ey1 != ey2 );
      else                                /* vertical line down */
        do
        {
          fy2 = 0;
          PVG_ras.cover += ( fy2 - fy1 );
          PVG_ras.area  += ( fy2 - fy1 ) * fx1 * 2;
          fy1 = PVG_ONE_PIXEL;
          ey1--;
          pvg_gray_set_cell( PVG_RAS_VAR_ ex1, ey1 );
        } while ( ey1 != ey2 );
    }
    else                                  /* any other line */
    {
      PVG_TArea  prod = dx * fy1 - dy * fx1;
      PVG_FT_UDIVPREP( dx );
      PVG_FT_UDIVPREP( dy );


      /* The fundamental value `prod' determines which side and the  */
      /* exact coordinate where the line exits current cell.  It is  */
      /* also easily updated when moving from one cell to the next.  */
      do
      {
        if      ( prod                                   <= 0 &&
                  prod - dx * PVG_ONE_PIXEL                  >  0 ) /* left */
        {
          fx2 = 0;
          fy2 = (PVG_TPos)PVG_FT_UDIV( -prod, -dx );
          prod -= dy * PVG_ONE_PIXEL;
          PVG_ras.cover += ( fy2 - fy1 );
          PVG_ras.area  += ( fy2 - fy1 ) * ( fx1 + fx2 );
          fx1 = PVG_ONE_PIXEL;
          fy1 = fy2;
          ex1--;
        }
        else if ( prod - dx * PVG_ONE_PIXEL                  <= 0 &&
                  prod - dx * PVG_ONE_PIXEL + dy * PVG_ONE_PIXEL >  0 ) /* up */
        {
          prod -= dx * PVG_ONE_PIXEL;
          fx2 = (PVG_TPos)PVG_FT_UDIV( -prod, dy );
          fy2 = PVG_ONE_PIXEL;
          PVG_ras.cover += ( fy2 - fy1 );
          PVG_ras.area  += ( fy2 - fy1 ) * ( fx1 + fx2 );
          fx1 = fx2;
          fy1 = 0;
          ey1++;
        }
        else if ( prod - dx * PVG_ONE_PIXEL + dy * PVG_ONE_PIXEL <= 0 &&
                  prod                  + dy * PVG_ONE_PIXEL >= 0 ) /* right */
        {
          prod += dy * PVG_ONE_PIXEL;
          fx2 = PVG_ONE_PIXEL;
          fy2 = (PVG_TPos)PVG_FT_UDIV( prod, dx );
          PVG_ras.cover += ( fy2 - fy1 );
          PVG_ras.area  += ( fy2 - fy1 ) * ( fx1 + fx2 );
          fx1 = 0;
          fy1 = fy2;
          ex1++;
        }
        else /* ( prod                  + dy * PVG_ONE_PIXEL <  0 &&
                  prod                                   >  0 )    down */
        {
          fx2 = (PVG_TPos)PVG_FT_UDIV( prod, -dy );
          fy2 = 0;
          prod += dx * PVG_ONE_PIXEL;
          PVG_ras.cover += ( fy2 - fy1 );
          PVG_ras.area  += ( fy2 - fy1 ) * ( fx1 + fx2 );
          fx1 = fx2;
          fy1 = PVG_ONE_PIXEL;
          ey1--;
        }

        pvg_gray_set_cell( PVG_RAS_VAR_ ex1, ey1 );
      } while ( ex1 != ex2 || ey1 != ey2 );
    }

    fx2 = PVG_FRACT( to_x );
    fy2 = PVG_FRACT( to_y );

    PVG_ras.cover += ( fy2 - fy1 );
    PVG_ras.area  += ( fy2 - fy1 ) * ( fx1 + fx2 );
  }

#endif

  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static int
  pvg_gray_clip_flags( PVG_RAS_ARG_ PVG_TPos x, PVG_TPos y )
  {
      return ((x > PVG_ras.clip_box.xMax) << 0) | ((y > PVG_ras.clip_box.yMax) << 1) |
             ((x < PVG_ras.clip_box.xMin) << 2) | ((y < PVG_ras.clip_box.yMin) << 3);
  }

  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static int
  pvg_gray_clip_vflags( PVG_RAS_ARG_ PVG_TPos y )
  {
      return ((y > PVG_ras.clip_box.yMax) << 1) | ((y < PVG_ras.clip_box.yMin) << 3);
  }

  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static void
  pvg_gray_vline( PVG_RAS_ARG_ PVG_TPos x1, PVG_TPos y1, PVG_TPos x2, PVG_TPos y2, int f1, int f2 )
  {
      f1 &= 10;
      f2 &= 10;
      if((f1 | f2) == 0) /* Fully visible */
      {
          pvg_gray_render_line( PVG_RAS_VAR_ x1, y1, x2, y2 );
      }
      else if(f1 == f2) /* Invisible by Y */
      {
          return;
      }
      else
      {
          PVG_TPos tx1, ty1, tx2, ty2;
          PVG_TPos clip_y1, clip_y2;

          tx1 = x1;
          ty1 = y1;
          tx2 = x2;
          ty2 = y2;

          clip_y1 = PVG_ras.clip_box.yMin;
          clip_y2 = PVG_ras.clip_box.yMax;

          if(f1 & 8) /* y1 < clip_y1 */
          {
              tx1 = x1 + FT_MulDiv(clip_y1-y1, x2-x1, y2-y1);
              ty1 = clip_y1;
          }

          if(f1 & 2) /* y1 > clip_y2 */
          {
              tx1 = x1 + FT_MulDiv(clip_y2-y1, x2-x1, y2-y1);
              ty1 = clip_y2;
          }

          if(f2 & 8) /* y2 < clip_y1 */
          {
              tx2 = x1 + FT_MulDiv(clip_y1-y1, x2-x1, y2-y1);
              ty2 = clip_y1;
          }

          if(f2 & 2) /* y2 > clip_y2 */
          {
              tx2 = x1 + FT_MulDiv(clip_y2-y1, x2-x1, y2-y1);
              ty2 = clip_y2;
          }

          pvg_gray_render_line( PVG_RAS_VAR_ tx1, ty1, tx2, ty2 );
      }
  }

  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static void
  pvg_gray_line_to( PVG_RAS_ARG_ PVG_TPos x2, PVG_TPos y2 )
  {
      if ( !PVG_ras.clipping )
      {
          pvg_gray_render_line( PVG_RAS_VAR_ PVG_ras.x, PVG_ras.y, x2, y2 );
      }
      else
      {
          PVG_TPos x1, y1, y3, y4;
          PVG_TPos clip_x1, clip_x2;
          int f1, f2, f3, f4;

          f1 = PVG_ras.clip_flags;
          f2 = pvg_gray_clip_flags( PVG_RAS_VAR_ x2, y2 );

          if((f1 & 10) == (f2 & 10) && (f1 & 10) != 0) /* Invisible by Y */
          {
              PVG_ras.clip_flags = f2;
              goto End;
          }

          x1 = PVG_ras.x;
          y1 = PVG_ras.y;

          clip_x1 = PVG_ras.clip_box.xMin;
          clip_x2 = PVG_ras.clip_box.xMax;

          switch(((f1 & 5) << 1) | (f2 & 5))
          {
          case 0: /* Visible by X */
              pvg_gray_vline( PVG_RAS_VAR_ x1, y1, x2, y2, f1, f2);
              break;

          case 1: /* x2 > clip_x2 */
              y3 = y1 + FT_MulDiv(clip_x2-x1, y2-y1, x2-x1);
              f3 = pvg_gray_clip_vflags( PVG_RAS_VAR_ y3 );
              pvg_gray_vline( PVG_RAS_VAR_ x1, y1, clip_x2, y3, f1, f3);
              pvg_gray_vline( PVG_RAS_VAR_ clip_x2, y3, clip_x2, y2, f3, f2);
              break;

          case 2: /* x1 > clip_x2 */
              y3 = y1 + FT_MulDiv(clip_x2-x1, y2-y1, x2-x1);
              f3 = pvg_gray_clip_vflags( PVG_RAS_VAR_ y3 );
              pvg_gray_vline( PVG_RAS_VAR_ clip_x2, y1, clip_x2, y3, f1, f3);
              pvg_gray_vline( PVG_RAS_VAR_ clip_x2, y3, x2, y2, f3, f2);
              break;

          case 3: /* x1 > clip_x2 && x2 > clip_x2 */
              pvg_gray_vline( PVG_RAS_VAR_ clip_x2, y1, clip_x2, y2, f1, f2);
              break;

          case 4: /* x2 < clip_x1 */
              y3 = y1 + FT_MulDiv(clip_x1-x1, y2-y1, x2-x1);
              f3 = pvg_gray_clip_vflags( PVG_RAS_VAR_ y3 );
              pvg_gray_vline( PVG_RAS_VAR_ x1, y1, clip_x1, y3, f1, f3);
              pvg_gray_vline( PVG_RAS_VAR_ clip_x1, y3, clip_x1, y2, f3, f2);
              break;

          case 6: /* x1 > clip_x2 && x2 < clip_x1 */
              y3 = y1 + FT_MulDiv(clip_x2-x1, y2-y1, x2-x1);
              y4 = y1 + FT_MulDiv(clip_x1-x1, y2-y1, x2-x1);
              f3 = pvg_gray_clip_vflags( PVG_RAS_VAR_ y3 );
              f4 = pvg_gray_clip_vflags( PVG_RAS_VAR_ y4 );
              pvg_gray_vline( PVG_RAS_VAR_ clip_x2, y1, clip_x2, y3, f1, f3);
              pvg_gray_vline( PVG_RAS_VAR_ clip_x2, y3, clip_x1, y4, f3, f4);
              pvg_gray_vline( PVG_RAS_VAR_ clip_x1, y4, clip_x1, y2, f4, f2);
              break;

          case 8: /* x1 < clip_x1 */
              y3 = y1 + FT_MulDiv(clip_x1-x1, y2-y1, x2-x1);
              f3 = pvg_gray_clip_vflags( PVG_RAS_VAR_ y3 );
              pvg_gray_vline( PVG_RAS_VAR_ clip_x1, y1, clip_x1, y3, f1, f3);
              pvg_gray_vline( PVG_RAS_VAR_ clip_x1, y3, x2, y2, f3, f2);
              break;

          case 9:  /* x1 < clip_x1 && x2 > clip_x2 */
              y3 = y1 + FT_MulDiv(clip_x1-x1, y2-y1, x2-x1);
              y4 = y1 + FT_MulDiv(clip_x2-x1, y2-y1, x2-x1);
              f3 = pvg_gray_clip_vflags( PVG_RAS_VAR_ y3 );
              f4 = pvg_gray_clip_vflags( PVG_RAS_VAR_ y4 );
              pvg_gray_vline( PVG_RAS_VAR_ clip_x1, y1, clip_x1, y3, f1, f3);
              pvg_gray_vline( PVG_RAS_VAR_ clip_x1, y3, clip_x2, y4, f3, f4);
              pvg_gray_vline( PVG_RAS_VAR_ clip_x2, y4, clip_x2, y2, f4, f2);
              break;

          case 12: /* x1 < clip_x1 && x2 < clip_x1 */
              pvg_gray_vline( PVG_RAS_VAR_ clip_x1, y1, clip_x1, y2, f1, f2);
              break;
          }

          PVG_ras.clip_flags = f2;
      }

  End:
      PVG_ras.x = x2;
      PVG_ras.y = y2;
  }

  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static void
  pvg_gray_split_conic( FT_Vector*  base )
  {
    PVG_TPos  a, b;


    base[4].x = base[2].x;
    b = base[1].x;
    a = base[3].x = ( base[2].x + b ) / 2;
    b = base[1].x = ( base[0].x + b ) / 2;
    base[2].x = ( a + b ) / 2;

    base[4].y = base[2].y;
    b = base[1].y;
    a = base[3].y = ( base[2].y + b ) / 2;
    b = base[1].y = ( base[0].y + b ) / 2;
    base[2].y = ( a + b ) / 2;
  }


  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static void
  pvg_gray_render_conic( PVG_RAS_ARG_ const FT_Vector*  control,
                              const FT_Vector*  to )
  {
    FT_Vector   bez_stack[16 * 2 + 1];  /* enough to accommodate bisections */
    FT_Vector*  arc = bez_stack;
    PVG_TPos        dx, dy;
    int         draw, split;


    arc[0].x = PVG_UPSCALE( to->x );
    arc[0].y = PVG_UPSCALE( to->y );
    arc[1].x = PVG_UPSCALE( control->x );
    arc[1].y = PVG_UPSCALE( control->y );
    arc[2].x = PVG_ras.x;
    arc[2].y = PVG_ras.y;

    /* short-cut the arc that crosses the current band */
    if ( ( PVG_TRUNC( arc[0].y ) >= PVG_ras.max_ey &&
           PVG_TRUNC( arc[1].y ) >= PVG_ras.max_ey &&
           PVG_TRUNC( arc[2].y ) >= PVG_ras.max_ey ) ||
         ( PVG_TRUNC( arc[0].y ) <  PVG_ras.min_ey &&
           PVG_TRUNC( arc[1].y ) <  PVG_ras.min_ey &&
           PVG_TRUNC( arc[2].y ) <  PVG_ras.min_ey ) )
    {
      if ( PVG_ras.clipping )
        PVG_ras.clip_flags = pvg_gray_clip_flags ( PVG_RAS_VAR_ arc[0].x, arc[0].y );
      PVG_ras.x = arc[0].x;
      PVG_ras.y = arc[0].y;
      return;
    }

    dx = FT_ABS( arc[2].x + arc[0].x - 2 * arc[1].x );
    dy = FT_ABS( arc[2].y + arc[0].y - 2 * arc[1].y );
    if ( dx < dy )
      dx = dy;

    /* We can calculate the number of necessary bisections because  */
    /* each bisection predictably reduces deviation exactly 4-fold. */
    /* Even 32-bit deviation would vanish after 16 bisections.      */
    draw = 1;
    while ( dx > PVG_ONE_PIXEL / 4 )
    {
      dx >>= 2;
      draw <<= 1;
    }

    /* We use decrement counter to count the total number of segments */
    /* to draw starting from 2^level. Before each draw we split as    */
    /* many times as there are trailing zeros in the counter.         */
    do
    {
      split = 1;
      while ( ( draw & split ) == 0 )
      {
        pvg_gray_split_conic( arc );
        arc += 2;
        split <<= 1;
      }

      pvg_gray_line_to( PVG_RAS_VAR_ arc[0].x, arc[0].y );
      arc -= 2;

    } while ( --draw );
  }


  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static void
  pvg_gray_split_cubic( FT_Vector*  base )
  {
    PVG_TPos  a, b, c, d;


    base[6].x = base[3].x;
    c = base[1].x;
    d = base[2].x;
    base[1].x = a = ( base[0].x + c ) / 2;
    base[5].x = b = ( base[3].x + d ) / 2;
    c = ( c + d ) / 2;
    base[2].x = a = ( a + c ) / 2;
    base[4].x = b = ( b + c ) / 2;
    base[3].x = ( a + b ) / 2;

    base[6].y = base[3].y;
    c = base[1].y;
    d = base[2].y;
    base[1].y = a = ( base[0].y + c ) / 2;
    base[5].y = b = ( base[3].y + d ) / 2;
    c = ( c + d ) / 2;
    base[2].y = a = ( a + c ) / 2;
    base[4].y = b = ( b + c ) / 2;
    base[3].y = ( a + b ) / 2;
  }


  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static void
  pvg_gray_render_cubic( PVG_RAS_ARG_ const FT_Vector*  control1,
                              const FT_Vector*  control2,
                              const FT_Vector*  to )
  {
    FT_Vector   bez_stack[16 * 3 + 1];  /* enough to accommodate bisections */
    FT_Vector*  arc = bez_stack;
    FT_Vector*  limit = bez_stack + 45;
    PVG_TPos        dx, dy, dx_, dy_;
    PVG_TPos        dx1, dy1, dx2, dy2;
    PVG_TPos        L, s, s_limit;


    arc[0].x = PVG_UPSCALE( to->x );
    arc[0].y = PVG_UPSCALE( to->y );
    arc[1].x = PVG_UPSCALE( control2->x );
    arc[1].y = PVG_UPSCALE( control2->y );
    arc[2].x = PVG_UPSCALE( control1->x );
    arc[2].y = PVG_UPSCALE( control1->y );
    arc[3].x = PVG_ras.x;
    arc[3].y = PVG_ras.y;

    /* short-cut the arc that crosses the current band */
    if ( ( PVG_TRUNC( arc[0].y ) >= PVG_ras.max_ey &&
           PVG_TRUNC( arc[1].y ) >= PVG_ras.max_ey &&
           PVG_TRUNC( arc[2].y ) >= PVG_ras.max_ey &&
           PVG_TRUNC( arc[3].y ) >= PVG_ras.max_ey ) ||
         ( PVG_TRUNC( arc[0].y ) <  PVG_ras.min_ey &&
           PVG_TRUNC( arc[1].y ) <  PVG_ras.min_ey &&
           PVG_TRUNC( arc[2].y ) <  PVG_ras.min_ey &&
           PVG_TRUNC( arc[3].y ) <  PVG_ras.min_ey ) )
    {
      if ( PVG_ras.clipping )
        PVG_ras.clip_flags = pvg_gray_clip_flags ( PVG_RAS_VAR_ arc[0].x, arc[0].y );
      PVG_ras.x = arc[0].x;
      PVG_ras.y = arc[0].y;
      return;
    }

    for (;;)
    {
      /* Decide whether to split or draw. See `Rapid Termination          */
      /* Evaluation for Recursive Subdivision of Bezier Curves' by Thomas */
      /* F. Hain, at                                                      */
      /* http://www.cis.southalabama.edu/~hain/general/Publications/Bezier/Camera-ready%20CISST02%202.pdf */


      /* dx and dy are x and y components of the P0-P3 chord vector. */
      dx = dx_ = arc[3].x - arc[0].x;
      dy = dy_ = arc[3].y - arc[0].y;

      L = FT_HYPOT( dx_, dy_ );

      /* Avoid possible arithmetic overflow below by splitting. */
      if ( L >= (1 << 23) )
        goto Split;

      /* Max deviation may be as much as (s/L) * 3/4 (if Hain's v = 1). */
      s_limit = L * (PVG_TPos)( PVG_ONE_PIXEL / 6 );

      /* s is L * the perpendicular distance from P1 to the line P0-P3. */
      dx1 = arc[1].x - arc[0].x;
      dy1 = arc[1].y - arc[0].y;
      s = FT_ABS( dy * dx1 - dx * dy1 );

      if ( s > s_limit )
        goto Split;

      /* s is L * the perpendicular distance from P2 to the line P0-P3. */
      dx2 = arc[2].x - arc[0].x;
      dy2 = arc[2].y - arc[0].y;
      s = FT_ABS( dy * dx2 - dx * dy2 );

      if ( s > s_limit )
        goto Split;

      /* Split super curvy segments where the off points are so far
         from the chord that the angles P0-P1-P3 or P0-P2-P3 become
         acute as detected by appropriate dot products. */
      if ( dx1 * ( dx1 - dx ) + dy1 * ( dy1 - dy ) > 0 ||
           dx2 * ( dx2 - dx ) + dy2 * ( dy2 - dy ) > 0 )
        goto Split;

      pvg_gray_line_to( PVG_RAS_VAR_ arc[0].x, arc[0].y );

      if ( arc == bez_stack )
        return;

      arc -= 3;
      continue;

    Split:
      if( arc == limit )
        return;
      pvg_gray_split_cubic( arc );
      arc += 3;
    }
  }



  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static int
  pvg_gray_move_to( const FT_Vector*  to,
                PVG_PWorker           worker )
  {
    PVG_TPos  x, y;


    /* record current cell, if any */
    if ( !PVG_ras.invalid )
      pvg_gray_record_cell( worker );

    /* start to a new position */
    x = PVG_UPSCALE( to->x );
    y = PVG_UPSCALE( to->y );

    pvg_gray_start_cell( worker, PVG_TRUNC( x ), PVG_TRUNC( y ) );

    if ( PVG_ras.clipping )
        PVG_ras.clip_flags = pvg_gray_clip_flags( worker, x, y );
    PVG_ras.x = x;
    PVG_ras.y = y;
    return 0;
  }


  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static void
  pvg_gray_hline( PVG_RAS_ARG_ PVG_TCoord  x,
                       PVG_TCoord  y,
                       PVG_TPos    area,
                       int     acount )
  {
    int coverage;


    /* compute the coverage line's coverage, depending on the    */
    /* outline fill rule                                         */
    /*                                                           */
    /* the coverage percentage is area/(PVG_PIXEL_BITS*PVG_PIXEL_BITS*2) */
    /*                                                           */
    coverage = (int)( area >> ( PVG_PIXEL_BITS * 2 + 1 - 8 ) );
                                                    /* use range 0..256 */
    if ( coverage < 0 )
      coverage = -coverage;

    if ( PVG_ras.outline.flags & FT_OUTLINE_EVEN_ODD_FILL )
    {
      coverage &= 511;

      if ( coverage > 256 )
        coverage = 512 - coverage;
      else if ( coverage == 256 )
        coverage = 255;
    }
    else
    {
      /* normal non-zero winding rule */
      if ( coverage >= 256 )
        coverage = 255;
    }

    y += (PVG_TCoord)PVG_ras.min_ey;
    x += (PVG_TCoord)PVG_ras.min_ex;

    /* PVG_FT_Span.x is an int, so limit our coordinates appropriately */
    if ( x >= (1 << 23) )
      x = (1 << 23) - 1;

    /* PVG_FT_Span.y is an int, so limit our coordinates appropriately */
    if ( y >= (1 << 23) )
      y = (1 << 23) - 1;

    if ( coverage )
    {
      PVG_FT_Span*  span;
      int       count;
      int       skip;

      /* see whether we can add this span to the current list */
      count = PVG_ras.num_gray_spans;
      span  = PVG_ras.gray_spans + count - 1;
      if ( count > 0                          &&
           span->y == y                       &&
           span->x + span->len == x           &&
           span->coverage == coverage         )
      {
        span->len = span->len + acount;
        return;
      }

      if ( count >= PVG_FT_MAX_GRAY_SPANS )
      {
        if ( PVG_ras.render_span && count > PVG_ras.skip_spans )
        {
          skip = PVG_ras.skip_spans > 0 ? PVG_ras.skip_spans : 0;
          PVG_ras.render_span( PVG_ras.num_gray_spans - skip,
                           PVG_ras.gray_spans + skip,
                           PVG_ras.render_span_data );
        }

        PVG_ras.skip_spans -= PVG_ras.num_gray_spans;
        /* PVG_ras.render_span( span->y, PVG_ras.gray_spans, count ); */
        PVG_ras.num_gray_spans = 0;

        span  = PVG_ras.gray_spans;
      }
      else
        span++;

      /* add a gray span to the current list */
      span->x        = x;
      span->len      = acount;
      span->y        = y;
      span->coverage = (unsigned char)coverage;

      PVG_ras.num_gray_spans++;
    }
  }



  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static void
  pvg_gray_sweep( PVG_RAS_ARG)
  {
    int  yindex;

    if ( PVG_ras.num_cells == 0 )
      return;

    for ( yindex = 0; yindex < PVG_ras.ycount; yindex++ )
    {
      PVG_PCell   cell  = PVG_ras.ycells[yindex];
      PVG_TCoord  cover = 0;
      PVG_TCoord  x     = 0;


      for ( ; cell != NULL; cell = cell->next )
      {
        PVG_TArea  area;


        if ( cell->x > x && cover != 0 )
          pvg_gray_hline( PVG_RAS_VAR_ x, yindex, cover * ( PVG_ONE_PIXEL * 2 ),
                      cell->x - x );

        cover += cell->cover;
        area   = cover * ( PVG_ONE_PIXEL * 2 ) - cell->area;

        if ( area != 0 && cell->x >= 0 )
          pvg_gray_hline( PVG_RAS_VAR_ cell->x, yindex, area, 1 );

        x = cell->x + 1;
      }

      if ( PVG_ras.count_ex > x && cover != 0 )
        pvg_gray_hline( PVG_RAS_VAR_ x, yindex, cover * ( PVG_ONE_PIXEL * 2 ),
                    PVG_ras.count_ex - x );
    }
  }

FT_Error PVG_FT_Outline_Check(FT_Outline* outline)
{
    if (outline) {
        FT_Int n_points = outline->n_points;
        FT_Int n_contours = outline->n_contours;
        FT_Int end0, end;
        FT_Int n;

        /* empty glyph? */
        if (n_points == 0 && n_contours == 0) return 0;

        /* check point and contour counts */
        if (n_points <= 0 || n_contours <= 0) goto Bad;

        end0 = end = -1;
        for (n = 0; n < n_contours; n++) {
            end = outline->contours[n];

            /* note that we don't accept empty contours */
            if (end <= end0 || end >= n_points) goto Bad;

            end0 = end;
        }

        if (end != n_points - 1) goto Bad;

        /* XXX: check the tags array */
        return 0;
    }

Bad:
    return ErrRaster_Invalid_Outline;
}

void PVG_FT_Outline_Get_CBox(const FT_Outline* outline, FT_BBox* acbox)
{
    FT_Pos xMin, yMin, xMax, yMax;

    if (outline && acbox) {
        if (outline->n_points == 0) {
            xMin = 0;
            yMin = 0;
            xMax = 0;
            yMax = 0;
        } else {
            FT_Vector* vec = outline->points;
            FT_Vector* limit = vec + outline->n_points;

            xMin = xMax = vec->x;
            yMin = yMax = vec->y;
            vec++;

            for (; vec < limit; vec++) {
                FT_Pos x, y;

                x = vec->x;
                if (x < xMin) xMin = x;
                if (x > xMax) xMax = x;

                y = vec->y;
                if (y < yMin) yMin = y;
                if (y > yMax) yMax = y;
            }
        }
        acbox->xMin = xMin;
        acbox->xMax = xMax;
        acbox->yMin = yMin;
        acbox->yMax = yMax;
    }
}

  /*************************************************************************/
  /*                                                                       */
  /*  The following function should only compile in stand_alone mode,      */
  /*  i.e., when building this component without the rest of FreeType.     */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    PVG_FT_Outline_Decompose                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Walks over an outline's structure to decompose it into individual  */
  /*    segments and Bezier arcs.  This function is also able to emit      */
  /*    `move to' and `close to' operations to indicate the start and end  */
  /*    of new contours in the outline.                                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    outline        :: A pointer to the source target.                  */
  /*                                                                       */
  /*    user           :: A typeless pointer which is passed to each       */
  /*                      emitter during the decomposition.  It can be     */
  /*                      used to store the state during the               */
  /*                      decomposition.                                   */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  static
  int  PVG_FT_Outline_Decompose( const FT_Outline*        outline,
                                void*                       user )
  {
#undef SCALED
#define SCALED( x )  (x)

    FT_Vector   v_last;
    FT_Vector   v_control;
    FT_Vector   v_start;

    FT_Vector*  point;
    FT_Vector*  limit;
    unsigned char*       tags; //NOTE(Taylor): Changed to unsigned char* from char*

    int   n;         /* index of contour in outline     */
    int   first;     /* index of first point in contour */
    int   error;
    char  tag;       /* current point's state           */

    if ( !outline )
      return ErrRaster_Invalid_Outline;

    first = 0;

    for ( n = 0; n < outline->n_contours; n++ )
    {
      int  last;  /* index of last point in contour */


      last  = outline->contours[n];
      if ( last < 0 )
        goto Invalid_Outline;
      limit = outline->points + last;

      v_start   = outline->points[first];
      v_start.x = SCALED( v_start.x );
      v_start.y = SCALED( v_start.y );

      v_last   = outline->points[last];
      v_last.x = SCALED( v_last.x );
      v_last.y = SCALED( v_last.y );

      v_control = v_start;

      point = outline->points + first;
      tags  = outline->tags  + first;
      tag   = PVG_FT_CURVE_TAG( tags[0] );

      /* A contour cannot start with a cubic control point! */
      if ( tag == PVG_FT_CURVE_TAG_CUBIC )
        goto Invalid_Outline;

      /* check first point to determine origin */
      if ( tag == PVG_FT_CURVE_TAG_CONIC )
      {
        /* first point is conic control.  Yes, this happens. */
        if ( PVG_FT_CURVE_TAG( outline->tags[last] ) == PVG_FT_CURVE_TAG_ON )
        {
          /* start at last point if it is on the curve */
          v_start = v_last;
          limit--;
        }
        else
        {
          /* if both first and last points are conic,         */
          /* start at their middle and record its position    */
          /* for closure                                      */
          v_start.x = ( v_start.x + v_last.x ) / 2;
          v_start.y = ( v_start.y + v_last.y ) / 2;

          v_last = v_start;
        }
        point--;
        tags--;
      }

      error = pvg_gray_move_to( &v_start, user );
      if ( error )
        goto Exit;

      while ( point < limit )
      {
        point++;
        tags++;

        tag = PVG_FT_CURVE_TAG( tags[0] );
        switch ( tag )
        {
        case PVG_FT_CURVE_TAG_ON:  /* emit a single line_to */
          {
            FT_Vector  vec;


            vec.x = SCALED( point->x );
            vec.y = SCALED( point->y );

            pvg_gray_line_to(user, PVG_UPSCALE(vec.x), PVG_UPSCALE(vec.y));
            continue;
          }

        case PVG_FT_CURVE_TAG_CONIC:  /* consume conic arcs */
          {
            v_control.x = SCALED( point->x );
            v_control.y = SCALED( point->y );

          Do_Conic:
            if ( point < limit )
            {
              FT_Vector  vec;
              FT_Vector  v_middle;


              point++;
              tags++;
              tag = PVG_FT_CURVE_TAG( tags[0] );

              vec.x = SCALED( point->x );
              vec.y = SCALED( point->y );

              if ( tag == PVG_FT_CURVE_TAG_ON )
              {
                pvg_gray_render_conic(user, &v_control, &vec);
                continue;
              }

              if ( tag != PVG_FT_CURVE_TAG_CONIC )
                goto Invalid_Outline;

              v_middle.x = ( v_control.x + vec.x ) / 2;
              v_middle.y = ( v_control.y + vec.y ) / 2;

              pvg_gray_render_conic(user, &v_control, &v_middle);

              v_control = vec;
              goto Do_Conic;
            }

            pvg_gray_render_conic(user, &v_control, &v_start);
            goto Close;
          }

        default:  /* PVG_FT_CURVE_TAG_CUBIC */
          {
            FT_Vector  vec1, vec2;


            if ( point + 1 > limit                             ||
                 PVG_FT_CURVE_TAG( tags[1] ) != PVG_FT_CURVE_TAG_CUBIC )
              goto Invalid_Outline;

            point += 2;
            tags  += 2;

            vec1.x = SCALED( point[-2].x );
            vec1.y = SCALED( point[-2].y );

            vec2.x = SCALED( point[-1].x );
            vec2.y = SCALED( point[-1].y );

            if ( point <= limit )
            {
              FT_Vector  vec;


              vec.x = SCALED( point->x );
              vec.y = SCALED( point->y );

              pvg_gray_render_cubic(user, &vec1, &vec2, &vec);
              continue;
            }

            pvg_gray_render_cubic(user, &vec1, &vec2, &v_start);
            goto Close;
          }
        }
      }

      /* close the contour with a line segment */
      pvg_gray_line_to(user, PVG_UPSCALE(v_start.x), PVG_UPSCALE(v_start.y));

   Close:
      first = last + 1;
    }

    return 0;

  Exit:
    return error;

  Invalid_Outline:
    return ErrRaster_Invalid_Outline;
  }

  //NOTE(Taylor): Added PVG_ prefix to avoid naming conflicts in unity build
  typedef struct  PVG_TBand_
  {
    PVG_TPos  min, max;

  } PVG_TBand;

  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static int
  pvg_gray_convert_glyph_inner( PVG_RAS_ARG )
  {
    volatile int  error = 0;

    if ( pvg_ft_setjmp( PVG_ras.jump_buffer ) == 0 )
    {
      error = PVG_FT_Outline_Decompose( &PVG_ras.outline, &PVG_ras );
      if ( !PVG_ras.invalid )
        pvg_gray_record_cell( PVG_RAS_VAR );
    }
    else
    {
      error = ErrRaster_Memory_Overflow;
    }

    return error;
  }


  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static int
  pvg_gray_convert_glyph( PVG_RAS_ARG )
  {
    PVG_TBand            bands[40];
    PVG_TBand* volatile  band;
    int volatile     n, num_bands;
    PVG_TPos volatile    min, max, max_y;
    FT_BBox*      clip;
    int              skip;

    PVG_ras.num_gray_spans = 0;

    /* Set up state in the raster object */
    pvg_gray_compute_cbox( PVG_RAS_VAR );

    /* clip to target bitmap, exit if nothing to do */
    clip = &PVG_ras.clip_box;

    if ( PVG_ras.max_ex <= clip->xMin || PVG_ras.min_ex >= clip->xMax ||
         PVG_ras.max_ey <= clip->yMin || PVG_ras.min_ey >= clip->yMax )
      return 0;

    PVG_ras.clip_flags = PVG_ras.clipping = 0;

    if ( PVG_ras.min_ex < clip->xMin ) {
        PVG_ras.min_ex = clip->xMin;
        PVG_ras.clipping = 1;
    }

    if ( PVG_ras.min_ey < clip->yMin ) {
        PVG_ras.min_ey = clip->yMin;
        PVG_ras.clipping = 1;
    }

    if ( PVG_ras.max_ex > clip->xMax ) {
        PVG_ras.max_ex = clip->xMax;
        PVG_ras.clipping = 1;
    }

    if ( PVG_ras.max_ey > clip->yMax ) {
        PVG_ras.max_ey = clip->yMax;
        PVG_ras.clipping = 1;
    }

    clip->xMin = (PVG_ras.min_ex - 1) * PVG_ONE_PIXEL;
    clip->yMin = (PVG_ras.min_ey - 1) * PVG_ONE_PIXEL;
    clip->xMax = (PVG_ras.max_ex + 1) * PVG_ONE_PIXEL;
    clip->yMax = (PVG_ras.max_ey + 1) * PVG_ONE_PIXEL;

    PVG_ras.count_ex = PVG_ras.max_ex - PVG_ras.min_ex;
    PVG_ras.count_ey = PVG_ras.max_ey - PVG_ras.min_ey;

    /* set up vertical bands */
    num_bands = (int)( ( PVG_ras.max_ey - PVG_ras.min_ey ) / PVG_ras.band_size );
    if ( num_bands == 0 )
      num_bands = 1;
    if ( num_bands >= 39 )
      num_bands = 39;

    PVG_ras.band_shoot = 0;

    min   = PVG_ras.min_ey;
    max_y = PVG_ras.max_ey;

    for ( n = 0; n < num_bands; n++, min = max )
    {
      max = min + PVG_ras.band_size;
      if ( n == num_bands - 1 || max > max_y )
        max = max_y;

      bands[0].min = min;
      bands[0].max = max;
      band         = bands;

      while ( band >= bands )
      {
        PVG_TPos  bottom, top, middle;
        int   error;

        {
          PVG_PCell  cells_max;
          int    yindex;
          int    cell_start, cell_end, cell_mod;


          PVG_ras.ycells = (PVG_PCell*)PVG_ras.buffer;
          PVG_ras.ycount = band->max - band->min;

          cell_start = sizeof ( PVG_PCell ) * PVG_ras.ycount;
          cell_mod   = cell_start % sizeof ( PVG_TCell );
          if ( cell_mod > 0 )
            cell_start += sizeof ( PVG_TCell ) - cell_mod;

          cell_end  = PVG_ras.buffer_size;
          cell_end -= cell_end % sizeof( PVG_TCell );

          cells_max = (PVG_PCell)( (char*)PVG_ras.buffer + cell_end );
          PVG_ras.cells = (PVG_PCell)( (char*)PVG_ras.buffer + cell_start );
          if ( PVG_ras.cells >= cells_max )
            goto ReduceBands;

          PVG_ras.max_cells = (int)(cells_max - PVG_ras.cells);
          if ( PVG_ras.max_cells < 2 )
            goto ReduceBands;

          for ( yindex = 0; yindex < PVG_ras.ycount; yindex++ )
            PVG_ras.ycells[yindex] = NULL;
        }

        PVG_ras.num_cells = 0;
        PVG_ras.invalid   = 1;
        PVG_ras.min_ey    = band->min;
        PVG_ras.max_ey    = band->max;
        PVG_ras.count_ey  = band->max - band->min;

        error = pvg_gray_convert_glyph_inner( PVG_RAS_VAR );

        if ( !error )
        {
          pvg_gray_sweep( PVG_RAS_VAR);
          band--;
          continue;
        }
        else if ( error != ErrRaster_Memory_Overflow )
          return 1;

      ReduceBands:
        /* render pool overflow; we will reduce the render band by half */
        bottom = band->min;
        top    = band->max;
        middle = bottom + ( ( top - bottom ) >> 1 );

        /* This is too complex for a single scanline; there must */
        /* be some problems.                                     */
        if ( middle == bottom )
        {
          return ErrRaster_OutOfMemory;
        }

        if ( bottom-top >= PVG_ras.band_size )
          PVG_ras.band_shoot++;

        band[1].min = bottom;
        band[1].max = middle;
        band[0].min = middle;
        band[0].max = top;
        band++;
      }
    }

    if ( PVG_ras.render_span && PVG_ras.num_gray_spans > PVG_ras.skip_spans )
    {
        skip = PVG_ras.skip_spans > 0 ? PVG_ras.skip_spans : 0;
        PVG_ras.render_span( PVG_ras.num_gray_spans - skip,
                         PVG_ras.gray_spans + skip,
                         PVG_ras.render_span_data );
    }

    PVG_ras.skip_spans -= PVG_ras.num_gray_spans;

    if ( PVG_ras.band_shoot > 8 && PVG_ras.band_size > 16 )
      PVG_ras.band_size = PVG_ras.band_size / 2;

    return 0;
  }


  //NOTE(Taylor): Added pvg_ prefix to avoid naming conflicts in unity build
  static int
  pvg_gray_raster_render( PVG_RAS_ARG_ void* buffer, long buffer_size,
                      const PVG_FT_Raster_Params*  params )
  {
    const FT_Outline*  outline    = (const FT_Outline*)params->source;
    if ( outline == NULL )
      return ErrRaster_Invalid_Outline;

    /* return immediately if the outline is empty */
    if ( outline->n_points == 0 || outline->n_contours <= 0 )
      return 0;

    if ( !outline->contours || !outline->points )
      return ErrRaster_Invalid_Outline;

    if ( outline->n_points !=
           outline->contours[outline->n_contours - 1] + 1 )
      return ErrRaster_Invalid_Outline;

    /* this version does not support monochrome rendering */
    if ( !( params->flags & PVG_FT_RASTER_FLAG_AA ) )
      return ErrRaster_Invalid_Mode;

    if ( !( params->flags & PVG_FT_RASTER_FLAG_DIRECT ) )
      return ErrRaster_Invalid_Mode;

    /* compute clipping box */
    if ( params->flags & PVG_FT_RASTER_FLAG_CLIP )
    {
      PVG_ras.clip_box = params->clip_box;
    }
    else
    {
      PVG_ras.clip_box.xMin = -(1 << 23);
      PVG_ras.clip_box.yMin = -(1 << 23);
      PVG_ras.clip_box.xMax =  (1 << 23) - 1;
      PVG_ras.clip_box.yMax =  (1 << 23) - 1;
    }

    pvg_gray_init_cells( PVG_RAS_VAR_ buffer, buffer_size );

    PVG_ras.outline   = *outline;
    PVG_ras.num_cells = 0;
    PVG_ras.invalid   = 1;
    PVG_ras.band_size = (int)(buffer_size / (long)(sizeof(PVG_TCell) * 8));

    PVG_ras.render_span      = (PVG_FT_Raster_Span_Func)params->gray_spans;
    PVG_ras.render_span_data = params->user;

    return pvg_gray_convert_glyph( PVG_RAS_VAR );
  }

  void
  PVG_FT_Raster_Render(const PVG_FT_Raster_Params *params)
  {
      char stack[PVG_FT_MINIMUM_POOL_SIZE];
      size_t length = PVG_FT_MINIMUM_POOL_SIZE;

      PVG_TWorker worker;
      worker.skip_spans = 0;
      int rendered_spans = 0;
      int error = pvg_gray_raster_render(&worker, stack, length, params);
      while(error == ErrRaster_OutOfMemory) {
          if(worker.skip_spans < 0)
              rendered_spans += -worker.skip_spans;
          worker.skip_spans = rendered_spans;
          length *= 2;
          void* heap = malloc(length);
          error = pvg_gray_raster_render(&worker, heap, length, params);
          free(heap);
      }
  }

/* END */
