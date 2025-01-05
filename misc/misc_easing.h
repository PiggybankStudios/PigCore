/*
File:   misc_easing.h
Author: Taylor Robbins
Date:   01\05\2025
Description:
	** A set of standard "easing" functions that take a value in [0, 1] and produce
	** a new value in [0, 1], often not the same value. These functions are mostly
	** useful for making non-linear movement speed animations, but they can be used
	** for anything where you want to change the velocity of a value delta without
	** changing the end-points or the duration
*/

#ifndef _MISC_EASING_H
#define _MISC_EASING_H

#include "std/std_trig.h"
#include "std/std_basic_math.h"

// In Functions: (slow start, fast end)
// f(t)
// ^           ^
// |           |
// |           /
// |          |
// |        _/
// |   __--/
// |_--
// +------------> t
// Out Functions: (fast start, slow end)
// f(t)
// ^       ___-->      
// |    _--         
// |   /          
// |  |          
// | |           
// ||            
// ||            
// +------------> t

typedef enum EasingStyle EasingStyle;
enum EasingStyle
{
	EasingStyle_None = 0,
	
	EasingStyle_Linear = 1,
	EasingStyle_QuadraticIn,
	EasingStyle_QuadraticOut,
	EasingStyle_QuadraticInOut,
	EasingStyle_CubicIn,
	EasingStyle_CubicOut,
	EasingStyle_CubicInOut,
	EasingStyle_QuarticIn,
	EasingStyle_QuarticOut,
	EasingStyle_QuarticInOut,
	EasingStyle_QuinticIn,
	EasingStyle_QuinticOut,
	EasingStyle_QuinticInOut,
	EasingStyle_SineIn,
	EasingStyle_SineOut,
	EasingStyle_SineInOut,
	EasingStyle_CircularIn,
	EasingStyle_CircularOut,
	EasingStyle_CircularInOut,
	EasingStyle_ExponentialIn,
	EasingStyle_ExponentialOut,
	EasingStyle_ExponentialInOut,
	EasingStyle_ElasticIn,
	EasingStyle_ElasticOut,
	EasingStyle_ElasticInOut,
	EasingStyle_BackIn,
	EasingStyle_BackOut,
	EasingStyle_BackInOut,
	EasingStyle_BounceIn,
	EasingStyle_BounceOut,
	EasingStyle_BounceInOut,
	EasingStyle_EarlyInOut,
	EasingStyle_LogTwoOutCustom,
	EasingStyle_LogTwoInCustom,
	
	EasingStyle_NumStyles,
};

const char* GetEasingStyleStr(EasingStyle style)
{
	switch (style)
	{
		case EasingStyle_None:             return "None";
		case EasingStyle_Linear:           return "Linear";
		case EasingStyle_QuadraticIn:      return "QuadraticIn";
		case EasingStyle_QuadraticOut:     return "QuadraticOut";
		case EasingStyle_QuadraticInOut:   return "QuadraticInOut";
		case EasingStyle_CubicIn:          return "CubicIn";
		case EasingStyle_CubicOut:         return "CubicOut";
		case EasingStyle_CubicInOut:       return "CubicInOut";
		case EasingStyle_QuarticIn:        return "QuarticIn";
		case EasingStyle_QuarticOut:       return "QuarticOut";
		case EasingStyle_QuarticInOut:     return "QuarticInOut";
		case EasingStyle_QuinticIn:        return "QuinticIn";
		case EasingStyle_QuinticOut:       return "QuinticOut";
		case EasingStyle_QuinticInOut:     return "QuinticInOut";
		case EasingStyle_SineIn:           return "SineIn";
		case EasingStyle_SineOut:          return "SineOut";
		case EasingStyle_SineInOut:        return "SineInOut";
		case EasingStyle_CircularIn:       return "CircularIn";
		case EasingStyle_CircularOut:      return "CircularOut";
		case EasingStyle_CircularInOut:    return "CircularInOut";
		case EasingStyle_ExponentialIn:    return "ExponentialIn";
		case EasingStyle_ExponentialOut:   return "ExponentialOut";
		case EasingStyle_ExponentialInOut: return "ExponentialInOut";
		case EasingStyle_ElasticIn:        return "ElasticIn";
		case EasingStyle_ElasticOut:       return "ElasticOut";
		case EasingStyle_ElasticInOut:     return "ElasticInOut";
		case EasingStyle_BackIn:           return "BackIn";
		case EasingStyle_BackOut:          return "BackOut";
		case EasingStyle_BackInOut:        return "BackInOut";
		case EasingStyle_BounceIn:         return "BounceIn";
		case EasingStyle_BounceOut:        return "BounceOut";
		case EasingStyle_BounceInOut:      return "BounceInOut";
		case EasingStyle_EarlyInOut:       return "EarlyInOut";
		case EasingStyle_LogTwoOutCustom:  return "LogTwoOutCustom";
		case EasingStyle_LogTwoInCustom:   return "LogTwoInCustom";
		default:                           return "Unknown";
	}
}

// +==============================+
// |            Linear            |
// +==============================+
// Modeled after the line y = x
r32 EaseLinear(r32 p)
{
	return p;
}
r32 InverseEaseLinear(r32 y)
{
	return y;
}

// +==============================+
// |          Quadratic           |
// +==============================+
// Modeled after the parabola y = x^2
r32 EaseQuadraticIn(r32 p)
{
	return p * p;
}
r32 InverseEaseQuadraticIn(r32 y)
{
	return SqrtR32(y);
}

// Modeled after the parabola y = -x^2 + 2x
r32 EaseQuadraticOut(r32 p)
{
	return -(p * (p - 2));
}
r32 InverseEaseQuadraticOut(r32 y)
{
	return 1 - SqrtR32(-y + 1);
}

// Modeled after the piecewise quadratic
// y = (1/2)((2x)^2)             ; [0, 0.5)
// y = -(1/2)((2x-1)*(2x-3) - 1) ; [0.5, 1]
r32 EaseQuadraticInOut(r32 p)
{
	if (p < 0.5f)
	{
		return 2 * p * p;
	}
	else
	{
		return (-2 * p * p) + (4 * p) - 1;
	}
}
r32 InverseEaseQuadraticInOut(r32 y)
{
	if (y < 0.5f)
	{
		return SqrtR32(y / 2);
	}
	else
	{
		return (8 - SqrtR32(-32 * y + 32)) / 8;
	}
}

// +==============================+
// |            Cubic             |
// +==============================+
// Modeled after the cubic y = x^3
r32 EaseCubicIn(r32 p)
{
	return p * p * p;
}

// Modeled after the cubic y = (x - 1)^3 + 1
r32 EaseCubicOut(r32 p)
{
	r32 f = (p - 1);
	return f * f * f + 1;
}

// Modeled after the piecewise cubic
// y = (1/2)((2x)^3)       ; [0, 0.5)
// y = (1/2)((2x-2)^3 + 2) ; [0.5, 1]
r32 EaseCubicInOut(r32 p)
{
	if (p < 0.5f)
	{
		return 4 * p * p * p;
	}
	else
	{
		r32 f = ((2 * p) - 2);
		return 0.5f * f * f * f + 1;
	}
}

// +==============================+
// |           Quartic            |
// +==============================+
// Modeled after the quartic x^4
r32 EaseQuarticIn(r32 p)
{
	return p * p * p * p;
}

// Modeled after the quartic y = 1 - (x - 1)^4
r32 EaseQuarticOut(r32 p)
{
	r32 f = (p - 1);
	return f * f * f * (1 - p) + 1;
}

// Modeled after the piecewise quartic
// y = (1/2)((2x)^4)        ; [0, 0.5)
// y = -(1/2)((2x-2)^4 - 2) ; [0.5, 1]
r32 EaseQuarticInOut(r32 p) 
{
	if (p < 0.5f)
	{
		return 8 * p * p * p * p;
	}
	else
	{
		r32 f = (p - 1);
		return -8 * f * f * f * f + 1;
	}
}

// +==============================+
// |           Quintic            |
// +==============================+
// Modeled after the quintic y = x^5
r32 EaseQuinticIn(r32 p) 
{
	return p * p * p * p * p;
}

// Modeled after the quintic y = (x - 1)^5 + 1
r32 EaseQuinticOut(r32 p) 
{
	r32 f = (p - 1);
	return f * f * f * f * f + 1;
}

// Modeled after the piecewise quintic
// y = (1/2)((2x)^5)       ; [0, 0.5)
// y = (1/2)((2x-2)^5 + 2) ; [0.5, 1]
r32 EaseQuinticInOut(r32 p) 
{
	if (p < 0.5f)
	{
		return 16 * p * p * p * p * p;
	}
	else
	{
		r32 f = ((2 * p) - 2);
		return  0.5f * f * f * f * f * f + 1;
	}
}

// +==============================+
// |             Sine             |
// +==============================+
// Modeled after quarter-cycle of sine wave
r32 EaseSineIn(r32 p)
{
	return SinR32((p - 1) * (Pi32*2)) + 1;
}

// Modeled after quarter-cycle of sine wave (different phase)
r32 EaseSineOut(r32 p)
{
	return SinR32(p * (Pi32*2));
}

// Modeled after half sine wave
r32 EaseSineInOut(r32 p)
{
	return 0.5f * (1 - CosR32(p * Pi32));
}

// +==============================+
// |           Circular           |
// +==============================+
// Modeled after shifted quadrant IV of unit circle
r32 EaseCircularIn(r32 p)
{
	return 1 - SqrtR32(1 - (p * p));
}

// Modeled after shifted quadrant II of unit circle
r32 EaseCircularOut(r32 p)
{
	return SqrtR32((2 - p) * p);
}

// Modeled after the piecewise circular function
// y = (1/2)(1 - SqrtR32(1 - 4x^2))           ; [0, 0.5)
// y = (1/2)(SqrtR32(-(2x - 3)*(2x - 1)) + 1) ; [0.5, 1]
r32 EaseCircularInOut(r32 p)
{
	if (p < 0.5f)
	{
		return 0.5f * (1 - SqrtR32(1 - 4 * (p * p)));
	}
	else
	{
		return 0.5f * (SqrtR32(-((2 * p) - 3) * ((2 * p) - 1)) + 1);
	}
}

// +==============================+
// |         Exponential          |
// +==============================+
// Modeled after the exponential function y = 2^(10(x - 1))
r32 EaseExponentialIn(r32 p)
{
	return (p == 0.0f) ? p : PowR32(2, 10 * (p - 1));
}

// Modeled after the exponential function y = -2^(-10x) + 1
r32 EaseExponentialOut(r32 p)
{
	return (p == 1.0f) ? p : 1 - PowR32(2, -10 * p);
}

// Modeled after the piecewise exponential
// y = (1/2)2^(10(2x - 1))         ; [0,0.5)
// y = -(1/2)*2^(-10(2x - 1))) + 1 ; [0.5,1]
r32 EaseExponentialInOut(r32 p)
{
	if (p == 0.0f || p == 1.0f) { return p; }
	
	if (p < 0.5f)
	{
		return 0.5f * PowR32(2, (20 * p) - 10);
	}
	else
	{
		return -0.5f * PowR32(2, (-20 * p) + 10) + 1;
	}
}

// +==============================+
// |           Elastic            |
// +==============================+
// Modeled after the damped sine wave y = SinR32(13pi/2*x)*PowR32(2, 10 * (x - 1))
r32 EaseElasticIn(r32 p)
{
	return SinR32(13 * (Pi32*2) * p) * PowR32(2, 10 * (p - 1));
}

// Modeled after the damped sine wave y = SinR32(-13pi/2*(x + 1))*PowR32(2, -10x) + 1
r32 EaseElasticOut(r32 p)
{
	return SinR32(-13 * (Pi32*2) * (p + 1)) * PowR32(2, -10 * p) + 1;
}

// Modeled after the piecewise exponentially-damped sine wave:
// y = (1/2)*SinR32(13pi/2*(2*x))*PowR32(2, 10 * ((2*x) - 1))      ; [0,0.5)
// y = (1/2)*(SinR32(-13pi/2*((2x-1)+1))*PowR32(2,-10(2*x-1)) + 2) ; [0.5, 1]
r32 EaseElasticInOut(r32 p)
{
	if (p < 0.5f)
	{
		return 0.5f * SinR32(13 * (Pi32*2) * (2 * p)) * PowR32(2, 10 * ((2 * p) - 1));
	}
	else
	{
		return 0.5f * (SinR32(-13 * (Pi32*2) * ((2 * p - 1) + 1)) * PowR32(2, -10 * (2 * p - 1)) + 2);
	}
}

// +==============================+
// |             Back             |
// +==============================+
// Modeled after the overshooting cubic y = x^3-x*SinR32(x*pi)
r32 EaseBackIn(r32 p)
{
	return p * p * p - p * SinR32(p * Pi32);
}

// Modeled after overshooting cubic y = 1-((1-x)^3-(1-x)*SinR32((1-x)*pi))
r32 EaseBackOut(r32 p)
{
	r32 f = (1 - p);
	return 1 - (f * f * f - f * SinR32(f * Pi32));
}

// Modeled after the piecewise overshooting cubic function:
// y = (1/2)*((2x)^3-(2x)*SinR32(2*x*pi))           ; [0, 0.5)
// y = (1/2)*(1-((1-x)^3-(1-x)*SinR32((1-x)*pi))+1) ; [0.5, 1]
r32 EaseBackInOut(r32 p)
{
	if (p < 0.5f)
	{
		r32 f = 2 * p;
		return 0.5f * (f * f * f - f * SinR32(f * Pi32));
	}
	else
	{
		r32 f = (1 - (2*p - 1));
		return 0.5f * (1 - (f * f * f - f * SinR32(f * Pi32))) + 0.5f;
	}
}

// +==============================+
// |            Bounce            |
// +==============================+
r32 EaseBounceOut(r32 p)
{
	if (p < 4/11.0f)
	{
		return (121 * p * p)/16.0f;
	}
	else if (p < 8/11.0f)
	{
		return (363/40.0f * p * p) - (99/10.0f * p) + 17/5.0f;
	}
	else if (p < 9/10.0f)
	{
		return (4356/361.0f * p * p) - (35442/1805.0f * p) + 16061/1805.0f;
	}
	else
	{
		return (54/5.0f * p * p) - (513/25.0f * p) + 268/25.0f;
	}
}

r32 EaseBounceIn(r32 p)
{
	return 1 - EaseBounceOut(1 - p);
}

r32 EaseBounceInOut(r32 p)
{
	if (p < 0.5f)
	{
		return 0.5f * EaseBounceIn(p*2);
	}
	else
	{
		return 0.5f * EaseBounceOut(p * 2 - 1) + 0.5f;
	}
}

// +==============================+
// |            Early             |
// +==============================+
r32 EaseEarlyInOut(r32 p)
{
	r32 p2 = (1.2f * p);
	if (p < 0.418f)
	{
		return 2 * p2 * p2;
	}
	else if (p < 0.833f)
	{
		return (-2 * p2 * p2) + (4 * p2) - 1;
	}
	else
	{
		return 1;
	}
}

// +==============================+
// |         LogTwoCustom         |
// +==============================+
r32 EaseLogTwoOutCustom(r32 p)
{
	return (1 / 3.16987f) * Log2R32((8.0f * p) + 1.0f);
}
r32 EaseLogTwoInCustom(r32 p)
{
	return (PowR32(2, (3.16987f * p)) - 1.0f) / 8.0f;
}

// +==============================+
// |           Generic            |
// +==============================+
r32 Ease(EasingStyle style, r32 p)
{
	switch (style)
	{
		case EasingStyle_Linear:           return EaseLinear(p);
		case EasingStyle_QuadraticIn:      return EaseQuadraticIn(p);
		case EasingStyle_QuadraticOut:     return EaseQuadraticOut(p);
		case EasingStyle_QuadraticInOut:   return EaseQuadraticInOut(p);
		case EasingStyle_CubicIn:          return EaseCubicIn(p);
		case EasingStyle_CubicOut:         return EaseCubicOut(p);
		case EasingStyle_CubicInOut:       return EaseCubicInOut(p);
		case EasingStyle_QuarticIn:        return EaseQuarticIn(p);
		case EasingStyle_QuarticOut:       return EaseQuarticOut(p);
		case EasingStyle_QuarticInOut:     return EaseQuarticInOut(p);
		case EasingStyle_QuinticIn:        return EaseQuinticIn(p);
		case EasingStyle_QuinticOut:       return EaseQuinticOut(p);
		case EasingStyle_QuinticInOut:     return EaseQuinticInOut(p);
		case EasingStyle_SineIn:           return EaseSineIn(p);
		case EasingStyle_SineOut:          return EaseSineOut(p);
		case EasingStyle_SineInOut:        return EaseSineInOut(p);
		case EasingStyle_CircularIn:       return EaseCircularIn(p);
		case EasingStyle_CircularOut:      return EaseCircularOut(p);
		case EasingStyle_CircularInOut:    return EaseCircularInOut(p);
		case EasingStyle_ExponentialIn:    return EaseExponentialIn(p);
		case EasingStyle_ExponentialOut:   return EaseExponentialOut(p);
		case EasingStyle_ExponentialInOut: return EaseExponentialInOut(p);
		case EasingStyle_ElasticIn:        return EaseElasticIn(p);
		case EasingStyle_ElasticOut:       return EaseElasticOut(p);
		case EasingStyle_ElasticInOut:     return EaseElasticInOut(p);
		case EasingStyle_BackIn:           return EaseBackIn(p);
		case EasingStyle_BackOut:          return EaseBackOut(p);
		case EasingStyle_BackInOut:        return EaseBackInOut(p);
		case EasingStyle_BounceIn:         return EaseBounceIn(p);
		case EasingStyle_BounceOut:        return EaseBounceOut(p);
		case EasingStyle_BounceInOut:      return EaseBounceInOut(p);
		case EasingStyle_EarlyInOut:       return EaseEarlyInOut(p);
		case EasingStyle_LogTwoOutCustom:  return EaseLogTwoOutCustom(p);
		case EasingStyle_LogTwoInCustom:   return EaseLogTwoInCustom(p);
		default: Assert(false); return p;
	};
}
//TODO: Implement more inverse easing operations
r32 InverseEase(EasingStyle style, r32 y)
{
	switch (style)
	{
		case EasingStyle_Linear:           return InverseEaseLinear(y);
		case EasingStyle_QuadraticIn:      return InverseEaseQuadraticIn(y);
		case EasingStyle_QuadraticOut:     return InverseEaseQuadraticOut(y);
		case EasingStyle_QuadraticInOut:   return InverseEaseQuadraticInOut(y);
		// case EasingStyle_CubicIn:          return InverseEaseCubicIn(y);
		// case EasingStyle_CubicOut:         return InverseEaseCubicOut(y);
		// case EasingStyle_CubicInOut:       return InverseEaseCubicInOut(y);
		// case EasingStyle_QuarticIn:        return InverseEaseQuarticIn(y);
		// case EasingStyle_QuarticOut:       return InverseEaseQuarticOut(y);
		// case EasingStyle_QuarticInOut:     return InverseEaseQuarticInOut(y);
		// case EasingStyle_QuinticIn:        return InverseEaseQuinticIn(y);
		// case EasingStyle_QuinticOut:       return InverseEaseQuinticOut(y);
		// case EasingStyle_QuinticInOut:     return InverseEaseQuinticInOut(y);
		// case EasingStyle_SineIn:           return InverseEaseSineIn(y);
		// case EasingStyle_SineOut:          return InverseEaseSineOut(y);
		// case EasingStyle_SineInOut:        return InverseEaseSineInOut(y);
		// case EasingStyle_CircularIn:       return InverseEaseCircularIn(y);
		// case EasingStyle_CircularOut:      return InverseEaseCircularOut(y);
		// case EasingStyle_CircularInOut:    return InverseEaseCircularInOut(y);
		// case EasingStyle_ExponentialIn:    return InverseEaseExponentialIn(y);
		// case EasingStyle_ExponentialOut:   return InverseEaseExponentialOut(y);
		// case EasingStyle_ExponentialInOut: return InverseEaseExponentialInOut(y);
		// case EasingStyle_ElasticIn:        return InverseEaseElasticIn(y);
		// case EasingStyle_ElasticOut:       return InverseEaseElasticOut(y);
		// case EasingStyle_ElasticInOut:     return InverseEaseElasticInOut(y);
		// case EasingStyle_BackIn:           return InverseEaseBackIn(y);
		// case EasingStyle_BackOut:          return InverseEaseBackOut(y);
		// case EasingStyle_BackInOut:        return InverseEaseBackInOut(y);
		// case EasingStyle_BounceIn:         return InverseEaseBounceIn(y);
		// case EasingStyle_BounceOut:        return InverseEaseBounceOut(y);
		// case EasingStyle_BounceInOut:      return InverseEaseBounceInOut(y);
		// case EasingStyle_EarlyInOut:       return InverseEaseEarlyInOut(y);
		case EasingStyle_LogTwoOutCustom:     return EaseLogTwoInCustom(y); //TODO: Should we make these separate functions called InverseEase? rather than just using the pair function to inverse?
		case EasingStyle_LogTwoInCustom:      return EaseLogTwoOutCustom(y);
		default: Assert(false); return y;
	};
}

#endif //  _MISC_EASING_H
