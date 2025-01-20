/*
File:   misc_standard_colors.h
Author: Taylor Robbins
Date:   01\05\2025
Description:
	** Contains a bunch of defines for sets of predefined colors.
	** Things like a set of primary colors, colors derived from the Monokai theme,
	** and a few custom sets for other things.
*/

#ifndef _MISC_STANDARD_COLORS_H
#define _MISC_STANDARD_COLORS_H

#include "base/base_typedefs.h"
#include "struct/struct_color.h"

// +--------------------------------------------------------------+
// |                         Pure Colors                          |
// +--------------------------------------------------------------+
	#define NUM_PREDEF_PURE_COLORS       6
	#define NUM_PREDEF_GREYSCALE_COLORS  16
	
	#define NoColor_Value                 0x00000000UL
	#define TransparentBlack_Value        0x00000000UL
	#define Transparent_Value             0x00FFFFFFUL
	#define TransparentWhite_Value        0x00FFFFFFUL
	
	#define Black_Value                   0xFF000000UL
	#define Grey1_Value                   0xFF111111UL
	#define Grey2_Value                   0xFF222222UL
	#define Grey3_Value                   0xFF333333UL
	#define Grey4_Value                   0xFF444444UL
	#define Grey5_Value                   0xFF555555UL
	#define Grey6_Value                   0xFF666666UL
	#define Grey7_Value                   0xFF777777UL
	#define Grey8_Value                   0xFF888888UL
	#define Grey9_Value                   0xFF999999UL
	#define Grey10_Value                  0xFFAAAAAAUL
	#define Grey11_Value                  0xFFBBBBBBUL
	#define Grey12_Value                  0xFFCCCCCCUL
	#define Grey13_Value                  0xFFDDDDDDUL
	#define Grey14_Value                  0xFFEEEEEEUL
	#define White_Value                   0xFFFFFFFFUL
	
	#define PureRed_Value                 0xFFFF0000UL
	#define PureOrange_Value              0xFFFFA500UL
	#define PureYellow_Value              0xFFFFFF00UL
	#define PureGreen_Value               0xFF008000UL
	#define PureBlue_Value                0xFF0000FFUL
	#define PurePurple_Value              0xFF800080UL
	
	#define NoColor                 NewColorU32(NoColor_Value)
	#define TransparentBlack        NewColorU32(TransparentBlack_Value)
	#define Transparent             NewColorU32(Transparent_Value)
	#define TransparentWhite        NewColorU32(TransparentWhite_Value)
	#define Black                   NewColorU32(Black_Value)
	#define Grey1                   NewColorU32(Grey1_Value)
	#define Grey2                   NewColorU32(Grey2_Value)
	#define Grey3                   NewColorU32(Grey3_Value)
	#define Grey4                   NewColorU32(Grey4_Value)
	#define Grey5                   NewColorU32(Grey5_Value)
	#define Grey6                   NewColorU32(Grey6_Value)
	#define Grey7                   NewColorU32(Grey7_Value)
	#define Grey8                   NewColorU32(Grey8_Value)
	#define Grey9                   NewColorU32(Grey9_Value)
	#define Grey10                  NewColorU32(Grey10_Value)
	#define Grey11                  NewColorU32(Grey11_Value)
	#define Grey12                  NewColorU32(Grey12_Value)
	#define Grey13                  NewColorU32(Grey13_Value)
	#define Grey14                  NewColorU32(Grey14_Value)
	#define White                   NewColorU32(White_Value)
	#define PureRed                 NewColorU32(PureRed_Value)
	#define PureOrange              NewColorU32(PureOrange_Value)
	#define PureYellow              NewColorU32(PureYellow_Value)
	#define PureGreen               NewColorU32(PureGreen_Value)
	#define PureBlue                NewColorU32(PureBlue_Value)
	#define PurePurple              NewColorU32(PurePurple_Value)
	
	#if !PIG_CORE_IMPLEMENTATION
	Color32 GetPureColorByIndex(uxx index);
	Color32 GetGreyscaleColorByIndex(uxx index);
	uxx GetPureColorIndexByValue(u32 colorValue);
	uxx GetPureColorIndex(Color32 color);
	uxx GetGreyscaleColorIndexByValue(u32 colorValue);
	uxx GetGreyscaleColorIndex(Color32 color);
	#else //PIG_CORE_IMPLEMENTATION
	PEXP Color32 GetPureColorByIndex(uxx index)
	{
		switch (index % NUM_PREDEF_PURE_COLORS)
		{
			case 0:  return PureRed;
			case 1:  return PureOrange;
			case 2:  return PureYellow;
			case 3:  return PureGreen;
			case 4:  return PureBlue;
			case 5:  return PurePurple;
			default: return PureRed;
		}
	}
	PEXP Color32 GetGreyscaleColorByIndex(uxx index)
	{
		switch (index % NUM_PREDEF_GREYSCALE_COLORS)
		{
			case 0:  return Black;
			case 1:  return Grey1;
			case 2:  return Grey2;
			case 3:  return Grey3;
			case 4:  return Grey4;
			case 5:  return Grey5;
			case 6:  return Grey6;
			case 7:  return Grey7;
			case 8:  return Grey8;
			case 9:  return Grey9;
			case 10: return Grey10;
			case 11: return Grey11;
			case 12: return Grey12;
			case 13: return Grey13;
			case 14: return Grey14;
			case 15: return White;
			default: return Black;
		}
	}
	PEXP uxx GetPureColorIndexByValue(u32 colorValue)
	{
		switch (colorValue)
		{
			case PureRed_Value:     return 0;
			case PureOrange_Value:  return 1;
			case PureYellow_Value:  return 2;
			case PureGreen_Value:   return 3;
			case PureBlue_Value:    return 4;
			case PurePurple_Value:  return 5;
			default: return NUM_PREDEF_PURE_COLORS;
		}
	}
	PEXP uxx GetPureColorIndex(Color32 color)
	{
		return GetPureColorIndexByValue(color.valueU32);
	}
	PEXP uxx GetGreyscaleColorIndexByValue(u32 colorValue)
	{
		switch (colorValue)
		{
			case Black_Value:  return 0;
			case Grey1_Value:  return 1;
			case Grey2_Value:  return 2;
			case Grey3_Value:  return 3;
			case Grey4_Value:  return 4;
			case Grey5_Value:  return 5;
			case Grey6_Value:  return 6;
			case Grey7_Value:  return 7;
			case Grey8_Value:  return 8;
			case Grey9_Value:  return 9;
			case Grey10_Value: return 10;
			case Grey11_Value: return 11;
			case Grey12_Value: return 12;
			case Grey13_Value: return 13;
			case Grey14_Value: return 14;
			case White_Value:  return 15;
			default: return NUM_PREDEF_GREYSCALE_COLORS;
		}
	}
	PEXP uxx GetGreyscaleColorIndex(Color32 color)
	{
		return GetGreyscaleColorIndexByValue(color.valueU32);
	}
	#endif //PIG_CORE_IMPLEMENTATION
// +--------------------------------------------------------------+

// +--------------------------------------------------------------+
// |                        Palette Colors                        |
// +--------------------------------------------------------------+
	#define NUM_PREDEF_PAL_COLORS       70
	
	#define PalRedDarker_Value            0xFFB91668
	#define PalRedDark_Value              0xFFE1126B
	#define PalRed_Value                  0xFFF92672
	#define PalRedLight_Value             0xFFFB6C7F
	#define PalRedLighter_Value           0xFFFDB5A8
	#define PalPinkDarker_Value           0xFFBF2A83
	#define PalPinkDark_Value             0xFFD9368B
	#define PalPink_Value                 0xFFED4E95
	#define PalPinkLight_Value            0xFFF088B8
	#define PalPinkLighter_Value          0xFFF6ACCD
	#define PalOrangeDarker_Value         0xFFBD7628
	#define PalOrangeDark_Value           0xFFE4861C
	#define PalOrange_Value               0xFFFD971F
	#define PalOrangeLight_Value          0xFFFEB64B
	#define PalOrangeLighter_Value        0xFFFED88F
	#define PalHoneyDarker_Value          0xFFD5A43E
	#define PalHoneyDark_Value            0xFFE9B34A
	#define PalHoney_Value                0xFFFDC459
	#define PalHoneyLight_Value           0xFFFED47A
	#define PalHoneyLighter_Value         0xFFFEE8AC
	#define PalYellowDarker_Value         0xFFB3B947
	#define PalYellowDark_Value           0xFFD5CE62
	#define PalYellow_Value               0xFFE6DB74
	#define PalYellowLight_Value          0xFFEFE48C
	#define PalYellowLighter_Value        0xFFF3ECAC
	#define PalBananaDarker_Value         0xFFB7BD2F
	#define PalBananaDark_Value           0xFFDAD942
	#define PalBanana_Value               0xFFEEE64F
	#define PalBananaLight_Value          0xFFF4EA6F
	#define PalBananaLighter_Value        0xFFF9F1A4
	#define PalGreenDarker_Value          0xFF76B434
	#define PalGreenDark_Value            0xFF8DCE2E
	#define PalGreen_Value                0xFFA6E22E
	#define PalGreenLight_Value           0xFFC3E753
	#define PalGreenLighter_Value         0xFFE1EE92
	#define PalGrassDarker_Value          0xFF608B4D
	#define PalGrassDark_Value            0xFF7DA641
	#define PalGrass_Value                0xFF92BB41
	#define PalGrassLight_Value           0xFFB3D05B
	#define PalGrassLighter_Value         0xFFD7E295
	#define PalBlueDarker_Value           0xFF2471EC
	#define PalBlueDark_Value             0xFF4585EE
	#define PalBlue_Value                 0xFF669BEF
	#define PalBlueLight_Value            0xFF99B7DE
	#define PalBlueLighter_Value          0xFFCDD9DB
	#define PalSkyDarker_Value            0xFF24B1EC
	#define PalSkyDark_Value              0xFF45C9EE
	#define PalSky_Value                  0xFF66D9EF
	#define PalSkyLight_Value             0xFF99E1DE
	#define PalSkyLighter_Value           0xFFCDEDDB
	#define PalPurpleDarker_Value         0xFF7445FF
	#define PalPurpleDark_Value           0xFF9161FF
	#define PalPurple_Value               0xFFAE81FF
	#define PalPurpleLight_Value          0xFFCBAAEA
	#define PalPurpleLighter_Value        0xFFD6BBEE
	#define PalVioletDarker_Value         0xFFC038FF
	#define PalVioletDark_Value           0xFFC95BFF
	#define PalViolet_Value               0xFFDD7BFF
	#define PalVioletLight_Value          0xFFE595FF
	#define PalVioletLighter_Value        0xFFEBAAFF
	#define PalNeutralDarker_Value        0xFF5E5E4A
	#define PalNeutralDark_Value          0xFF75715E
	#define PalNeutral_Value              0xFF908861
	#define PalNeutralLight_Value         0xFFA79C65
	#define PalNeutralLighter_Value       0xFFB9B084
	
	#define PalBackgroundDarker_Value     0xFF131410
	#define PalBackgroundDark_Value       0xFF1F221A
	#define PalBackground_Value           0xFF272822
	#define PalBackgroundLight_Value      0xFF424431
	#define PalBackgroundLighter_Value    0xFF545934
	#define PalGreyDarker_Value           0xFF64635B
	#define PalGreyDark_Value             0xFF707067
	#define PalGrey_Value                 0xFF7D7C72
	#define PalGreyLight_Value            0xFF98978E
	#define PalGreyLighter_Value          0xFFB1B1AA
	#define PalBlackDarker_Value          0xFF121211
	#define PalBlackDark_Value            0xFF1E1E1D
	#define PalBlack_Value                0xFF262624
	#define PalBlackLight_Value           0xFF3C3C39
	#define PalBlackLighter_Value         0xFF53534E
	
	#define PalRedDarker            NewColorU32(PalRedDarker_Value)
	#define PalRedDark              NewColorU32(PalRedDark_Value)
	#define PalRed                  NewColorU32(PalRed_Value)
	#define PalRedLight             NewColorU32(PalRedLight_Value)
	#define PalRedLighter           NewColorU32(PalRedLighter_Value)
	#define PalPinkDarker           NewColorU32(PalPinkDarker_Value)
	#define PalPinkDark             NewColorU32(PalPinkDark_Value)
	#define PalPink                 NewColorU32(PalPink_Value)
	#define PalPinkLight            NewColorU32(PalPinkLight_Value)
	#define PalPinkLighter          NewColorU32(PalPinkLighter_Value)
	#define PalOrangeDarker         NewColorU32(PalOrangeDarker_Value)
	#define PalOrangeDark           NewColorU32(PalOrangeDark_Value)
	#define PalOrange               NewColorU32(PalOrange_Value)
	#define PalOrangeLight          NewColorU32(PalOrangeLight_Value)
	#define PalOrangeLighter        NewColorU32(PalOrangeLighter_Value)
	#define PalHoneyDarker          NewColorU32(PalHoneyDarker_Value)
	#define PalHoneyDark            NewColorU32(PalHoneyDark_Value)
	#define PalHoney                NewColorU32(PalHoney_Value)
	#define PalHoneyLight           NewColorU32(PalHoneyLight_Value)
	#define PalHoneyLighter         NewColorU32(PalHoneyLighter_Value)
	#define PalYellowDarker         NewColorU32(PalYellowDarker_Value)
	#define PalYellowDark           NewColorU32(PalYellowDark_Value)
	#define PalYellow               NewColorU32(PalYellow_Value)
	#define PalYellowLight          NewColorU32(PalYellowLight_Value)
	#define PalYellowLighter        NewColorU32(PalYellowLighter_Value)
	#define PalBananaDarker         NewColorU32(PalBananaDarker_Value)
	#define PalBananaDark           NewColorU32(PalBananaDark_Value)
	#define PalBanana               NewColorU32(PalBanana_Value)
	#define PalBananaLight          NewColorU32(PalBananaLight_Value)
	#define PalBananaLighter        NewColorU32(PalBananaLighter_Value)
	#define PalGreenDarker          NewColorU32(PalGreenDarker_Value)
	#define PalGreenDark            NewColorU32(PalGreenDark_Value)
	#define PalGreen                NewColorU32(PalGreen_Value)
	#define PalGreenLight           NewColorU32(PalGreenLight_Value)
	#define PalGreenLighter         NewColorU32(PalGreenLighter_Value)
	#define PalGrassDarker          NewColorU32(PalGrassDarker_Value)
	#define PalGrassDark            NewColorU32(PalGrassDark_Value)
	#define PalGrass                NewColorU32(PalGrass_Value)
	#define PalGrassLight           NewColorU32(PalGrassLight_Value)
	#define PalGrassLighter         NewColorU32(PalGrassLighter_Value)
	#define PalBlueDarker           NewColorU32(PalBlueDarker_Value)
	#define PalBlueDark             NewColorU32(PalBlueDark_Value)
	#define PalBlue                 NewColorU32(PalBlue_Value)
	#define PalBlueLight            NewColorU32(PalBlueLight_Value)
	#define PalBlueLighter          NewColorU32(PalBlueLighter_Value)
	#define PalSkyDarker            NewColorU32(PalSkyDarker_Value)
	#define PalSkyDark              NewColorU32(PalSkyDark_Value)
	#define PalSky                  NewColorU32(PalSky_Value)
	#define PalSkyLight             NewColorU32(PalSkyLight_Value)
	#define PalSkyLighter           NewColorU32(PalSkyLighter_Value)
	#define PalPurpleDarker         NewColorU32(PalPurpleDarker_Value)
	#define PalPurpleDark           NewColorU32(PalPurpleDark_Value)
	#define PalPurple               NewColorU32(PalPurple_Value)
	#define PalPurpleLight          NewColorU32(PalPurpleLight_Value)
	#define PalPurpleLighter        NewColorU32(PalPurpleLighter_Value)
	#define PalVioletDarker         NewColorU32(PalVioletDarker_Value)
	#define PalVioletDark           NewColorU32(PalVioletDark_Value)
	#define PalViolet               NewColorU32(PalViolet_Value)
	#define PalVioletLight          NewColorU32(PalVioletLight_Value)
	#define PalVioletLighter        NewColorU32(PalVioletLighter_Value)
	#define PalNeutralDarker        NewColorU32(PalNeutralDarker_Value)
	#define PalNeutralDark          NewColorU32(PalNeutralDark_Value)
	#define PalNeutral              NewColorU32(PalNeutral_Value)
	#define PalNeutralLight         NewColorU32(PalNeutralLight_Value)
	#define PalNeutralLighter       NewColorU32(PalNeutralLighter_Value)
	
	#define PalBackgroundDarker     NewColorU32(PalBackgroundDarker_Value)
	#define PalBackgroundDark       NewColorU32(PalBackgroundDark_Value)
	#define PalBackground           NewColorU32(PalBackground_Value)
	#define PalBackgroundLight      NewColorU32(PalBackgroundLight_Value)
	#define PalBackgroundLighter    NewColorU32(PalBackgroundLighter_Value)
	#define PalGreyDarker           NewColorU32(PalGreyDarker_Value)
	#define PalGreyDark             NewColorU32(PalGreyDark_Value)
	#define PalGrey                 NewColorU32(PalGrey_Value)
	#define PalGreyLight            NewColorU32(PalGreyLight_Value)
	#define PalGreyLighter          NewColorU32(PalGreyLighter_Value)
	#define PalBlackDarker          NewColorU32(PalBlackDarker_Value)
	#define PalBlackDark            NewColorU32(PalBlackDark_Value)
	#define PalBlack                NewColorU32(PalBlack_Value)
	#define PalBlackLight           NewColorU32(PalBlackLight_Value)
	#define PalBlackLighter         NewColorU32(PalBlackLighter_Value)

	#if !PIG_CORE_IMPLEMENTATION
	Color32 GetPredefPalColorByIndex(uxx index);
	uxx GetPredefPalColorIndexByValue(u32 colorValue);
	uxx GetPredefPalColorIndex(Color32 color);
	#else //PIG_CORE_IMPLEMENTATION
	PEXP Color32 GetPredefPalColorByIndex(uxx index)
	{
		switch (index % NUM_PREDEF_PAL_COLORS)
		{
			case 0:  return PalRed;
			case 1:  return PalOrange;
			case 2:  return PalYellow;
			case 3:  return PalGreen;
			case 4:  return PalBlue;
			case 5:  return PalPurple;
			case 6:  return PalNeutral;
			
			case 7:  return PalRedLight;
			case 8:  return PalOrangeLight;
			case 9:  return PalYellowLight;
			case 10: return PalGreenLight;
			case 11: return PalBlueLight;
			case 12: return PalPurpleLight;
			case 13: return PalNeutralLight;
			
			case 14: return PalRedDark;
			case 15: return PalOrangeDark;
			case 16: return PalYellowDark;
			case 17: return PalGreenDark;
			case 18: return PalBlueDark;
			case 19: return PalPurpleDark;
			case 20: return PalNeutralDark;
			
			case 21: return PalPink;
			case 22: return PalHoney;
			case 23: return PalBanana;
			case 24: return PalGrass;
			case 25: return PalSky;
			case 26: return PalViolet;
			case 27: return PalGrey;
			
			case 28: return PalPinkLight;
			case 29: return PalHoneyLight;
			case 30: return PalBananaLight;
			case 31: return PalGrassLight;
			case 32: return PalSkyLight;
			case 33: return PalVioletLight;
			case 34: return PalGreyLight;
			
			case 35: return PalPinkDark;
			case 36: return PalHoneyDark;
			case 37: return PalBananaDark;
			case 38: return PalGrassDark;
			case 39: return PalSkyDark;
			case 40: return PalVioletDark;
			case 41: return PalGreyDark;
			
			case 42: return PalRedLighter;
			case 43: return PalOrangeLighter;
			case 44: return PalYellowLighter;
			case 45: return PalGreenLighter;
			case 46: return PalBlueLighter;
			case 47: return PalPurpleLighter;
			case 48: return PalNeutralLighter;
			
			case 49: return PalPinkLighter;
			case 50: return PalHoneyLighter;
			case 51: return PalBananaLighter;
			case 52: return PalGrassLighter;
			case 53: return PalSkyLighter;
			case 54: return PalVioletLighter;
			case 55: return PalGreyLighter;
			
			case 56: return PalRedDarker;
			case 57: return PalOrangeDarker;
			case 58: return PalYellowDarker;
			case 59: return PalGreenDarker;
			case 60: return PalBlueDarker;
			case 61: return PalPurpleDarker;
			case 62: return PalNeutralDarker;
			
			case 63: return PalPinkDarker;
			case 64: return PalHoneyDarker;
			case 65: return PalBananaDarker;
			case 66: return PalGrassDarker;
			case 67: return PalSkyDarker;
			case 68: return PalVioletDarker;
			case 69: return PalGreyDarker;
			
			default: return Black;
		}
	}
	PEXP uxx GetPredefPalColorIndexByValue(u32 colorValue)
	{
		switch (colorValue)
		{
			case PalRed_Value:            return 0;
			case PalOrange_Value:         return 1;
			case PalYellow_Value:         return 2;
			case PalGreen_Value:          return 3;
			case PalBlue_Value:           return 4;
			case PalPurple_Value:         return 5;
			case PalNeutral_Value:        return 6;
			case PalRedLight_Value:       return 7;
			case PalOrangeLight_Value:    return 8;
			case PalYellowLight_Value:    return 9;
			case PalGreenLight_Value:     return 10;
			case PalBlueLight_Value:      return 11;
			case PalPurpleLight_Value:    return 12;
			case PalNeutralLight_Value:   return 13;
			case PalRedDark_Value:        return 14;
			case PalOrangeDark_Value:     return 15;
			case PalYellowDark_Value:     return 16;
			case PalGreenDark_Value:      return 17;
			case PalBlueDark_Value:       return 18;
			case PalPurpleDark_Value:     return 19;
			case PalNeutralDark_Value:    return 20;
			case PalPink_Value:           return 21;
			case PalHoney_Value:          return 22;
			case PalBanana_Value:         return 23;
			case PalGrass_Value:          return 24;
			case PalSky_Value:            return 25;
			case PalViolet_Value:         return 26;
			case PalGrey_Value:           return 27;
			case PalPinkLight_Value:      return 28;
			case PalHoneyLight_Value:     return 29;
			case PalBananaLight_Value:    return 30;
			case PalGrassLight_Value:     return 31;
			case PalSkyLight_Value:       return 32;
			case PalVioletLight_Value:    return 33;
			case PalGreyLight_Value:      return 34;
			case PalPinkDark_Value:       return 35;
			case PalHoneyDark_Value:      return 36;
			case PalBananaDark_Value:     return 37;
			case PalGrassDark_Value:      return 38;
			case PalSkyDark_Value:        return 39;
			case PalVioletDark_Value:     return 40;
			case PalGreyDark_Value:       return 41;
			case PalRedLighter_Value:     return 42;
			case PalOrangeLighter_Value:  return 43;
			case PalYellowLighter_Value:  return 44;
			case PalGreenLighter_Value:   return 45;
			case PalBlueLighter_Value:    return 46;
			case PalPurpleLighter_Value:  return 47;
			case PalNeutralLighter_Value: return 48;
			case PalPinkLighter_Value:    return 49;
			case PalHoneyLighter_Value:   return 50;
			case PalBananaLighter_Value:  return 51;
			case PalGrassLighter_Value:   return 52;
			case PalSkyLighter_Value:     return 53;
			case PalVioletLighter_Value:  return 54;
			case PalGreyLighter_Value:    return 55;
			case PalRedDarker_Value:      return 56;
			case PalOrangeDarker_Value:   return 57;
			case PalYellowDarker_Value:   return 58;
			case PalGreenDarker_Value:    return 59;
			case PalBlueDarker_Value:     return 60;
			case PalPurpleDarker_Value:   return 61;
			case PalNeutralDarker_Value:  return 62;
			case PalPinkDarker_Value:     return 63;
			case PalHoneyDarker_Value:    return 64;
			case PalBananaDarker_Value:   return 65;
			case PalGrassDarker_Value:    return 66;
			case PalSkyDarker_Value:      return 67;
			case PalVioletDarker_Value:   return 68;
			case PalGreyDarker_Value:     return 69;
			default: return NUM_PREDEF_PAL_COLORS;
		}
	}
	PEXP uxx GetPredefPalColorIndex(Color32 color)
	{
		return GetPredefPalColorIndexByValue(color.valueU32);
	}
	#endif //PIG_CORE_IMPLEMENTATION
// +--------------------------------------------------------------+

// +--------------------------------------------------------------+
// |                        Monokai Colors                        |
// +--------------------------------------------------------------+
	#define NUM_MONOKAI_COLORS       20
	
	#define MonokaiBack_Value               0xFF3B3A32
	#define MonokaiYellow_Value             0xFFE6DB74
	#define MonokaiLightYellow_Value        0xFFFFE792
	#define MonokaiFadedYellow_Value        0xFFFFEFB7
	#define MonokaiPurple_Value             0xFFAE81FF
	#define MonokaiLightPurple_Value        0xFFE777FF
	#define MonokaiGreen_Value              0xFFA6E22E
	#define MonokaiDarkGreen_Value          0xFF829520
	#define MonokaiOrange_Value             0xFFFD971F
	#define MonokaiBrown_Value              0xFF9D550F
	#define MonokaiMagenta_Value            0xFFF92672
	#define MonokaiRed_Value                0xFFF83333
	#define MonokaiLightRed_Value           0xFFFF5959
	#define MonokaiBlue_Value               0xFF66D9EF
	#define MonokaiLightBlue_Value          0xFFA9FFFF
	#define MonokaiWhite_Value              0xFFF8F8F2
	#define MonokaiLightGray_Value          0xFFBBBBBB
	#define MonokaiGray1_Value              0xFFAFAFA2
	#define MonokaiGray2_Value              0xFF75715E
	#define MonokaiDarkGray_Value           0xFF212121

	#define MonokaiBack                     NewColorU32(MonokaiBack_Value)
	#define MonokaiYellow                   NewColorU32(MonokaiYellow_Value)
	#define MonokaiLightYellow              NewColorU32(MonokaiLightYellow_Value)
	#define MonokaiFadedYellow              NewColorU32(MonokaiFadedYellow_Value)
	#define MonokaiPurple                   NewColorU32(MonokaiPurple_Value)
	#define MonokaiLightPurple              NewColorU32(MonokaiLightPurple_Value)
	#define MonokaiGreen                    NewColorU32(MonokaiGreen_Value)
	#define MonokaiDarkGreen                NewColorU32(MonokaiDarkGreen_Value)
	#define MonokaiOrange                   NewColorU32(MonokaiOrange_Value)
	#define MonokaiBrown                    NewColorU32(MonokaiBrown_Value)
	#define MonokaiMagenta                  NewColorU32(MonokaiMagenta_Value)
	#define MonokaiRed                      NewColorU32(MonokaiRed_Value)
	#define MonokaiLightRed                 NewColorU32(MonokaiLightRed_Value)
	#define MonokaiBlue                     NewColorU32(MonokaiBlue_Value)
	#define MonokaiLightBlue                NewColorU32(MonokaiLightBlue_Value)
	#define MonokaiWhite                    NewColorU32(MonokaiWhite_Value)
	#define MonokaiLightGray                NewColorU32(MonokaiLightGray_Value)
	#define MonokaiGray1                    NewColorU32(MonokaiGray1_Value)
	#define MonokaiGray2                    NewColorU32(MonokaiGray2_Value)
	#define MonokaiDarkGray                 NewColorU32(MonokaiDarkGray_Value)
	
	#if !PIG_CORE_IMPLEMENTATION
	Color32 GetMonokaiColorByIndex(uxx index);
	uxx GetMonokaiColorIndexByValue(u32 colorValue);
	uxx GetMonokaiColorIndex(Color32 color);
	#else //PIG_CORE_IMPLEMENTATION
	PEXP Color32 GetMonokaiColorByIndex(uxx index)
	{
		switch (index % NUM_MONOKAI_COLORS)
		{
			case 0:  return MonokaiBack;
			case 1:  return MonokaiYellow;
			case 2:  return MonokaiLightYellow;
			case 3:  return MonokaiFadedYellow;
			case 4:  return MonokaiPurple;
			case 5:  return MonokaiLightPurple;
			case 6:  return MonokaiGreen;
			case 7:  return MonokaiDarkGreen;
			case 8:  return MonokaiOrange;
			case 9:  return MonokaiBrown;
			case 10: return MonokaiMagenta;
			case 11: return MonokaiRed;
			case 12: return MonokaiLightRed;
			case 13: return MonokaiBlue;
			case 14: return MonokaiLightBlue;
			case 15: return MonokaiWhite;
			case 16: return MonokaiLightGray;
			case 17: return MonokaiGray1;
			case 18: return MonokaiGray2;
			case 19: return MonokaiDarkGray;
			default: return Black;
		}
	}
	PEXP uxx GetMonokaiColorIndexByValue(u32 colorValue)
	{
		switch (colorValue)
		{
			case MonokaiBack_Value:         return 0;
			case MonokaiYellow_Value:       return 1;
			case MonokaiLightYellow_Value:  return 2;
			case MonokaiFadedYellow_Value:  return 3;
			case MonokaiPurple_Value:       return 4;
			case MonokaiLightPurple_Value:  return 5;
			case MonokaiGreen_Value:        return 6;
			case MonokaiDarkGreen_Value:    return 7;
			case MonokaiOrange_Value:       return 8;
			case MonokaiBrown_Value:        return 9;
			case MonokaiMagenta_Value:      return 10;
			case MonokaiRed_Value:          return 11;
			case MonokaiLightRed_Value:     return 12;
			case MonokaiBlue_Value:         return 13;
			case MonokaiLightBlue_Value:    return 14;
			case MonokaiWhite_Value:        return 15;
			case MonokaiLightGray_Value:    return 16;
			case MonokaiGray1_Value:        return 17;
			case MonokaiGray2_Value:        return 18;
			case MonokaiDarkGray_Value:     return 19;
			default: return NUM_MONOKAI_COLORS;
		}
	}
	PEXP uxx GetMonokaiColorIndex(Color32 color)
	{
		return GetMonokaiColorIndexByValue(color.valueU32);
	}
	#endif //PIG_CORE_IMPLEMENTATION
// +--------------------------------------------------------------+

#endif //  _MISC_STANDARD_COLORS_H

#if defined(_BASE_DBG_LEVEL_H) && defined(_MISC_STANDARD_COLORS_H)
#include "cross/cross_dbg_level_and_standard_colors.h"
#endif
