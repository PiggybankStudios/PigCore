/*
File:   cross_parsing_and_directions.h
Author: Taylor Robbins
Date:   02\01\2025
*/

#ifndef _CROSS_PARSING_AND_DIRECTIONS_H
#define _CROSS_PARSING_AND_DIRECTIONS_H

//NOTE: Intentionally no includes here

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	bool TryParseDir2(Str8 str, Dir2* valueOut, Result* errorOut);
	bool TryParseDir3(Str8 str, Dir3* valueOut, Result* errorOut);
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXP bool TryParseDir2(Str8 str, Dir2* valueOut, Result* errorOut)
{
	NotNullStr(str);
	str = TrimWhitespace(str);
	Dir2 result = Dir2_None;
	if      (StrAnyCaseEquals(str, StrLit("none")))   { result = Dir2_None;  }
	else if (StrAnyCaseEquals(str, StrLit("r")))      { result = Dir2_Right; }
	else if (StrAnyCaseEquals(str, StrLit("right")))  { result = Dir2_Right; }
	else if (StrAnyCaseEquals(str, StrLit("+x")))     { result = Dir2_Right; }
	else if (StrAnyCaseEquals(str, StrLit("l")))      { result = Dir2_Left;  }
	else if (StrAnyCaseEquals(str, StrLit("left")))   { result = Dir2_Left;  }
	else if (StrAnyCaseEquals(str, StrLit("-x")))     { result = Dir2_Left;  }
	else if (StrAnyCaseEquals(str, StrLit("u")))      { result = Dir2_Up;    }
	else if (StrAnyCaseEquals(str, StrLit("up")))     { result = Dir2_Up;    }
	else if (StrAnyCaseEquals(str, StrLit("t")))      { result = Dir2_Up;    }
	else if (StrAnyCaseEquals(str, StrLit("top")))    { result = Dir2_Up;    }
	else if (StrAnyCaseEquals(str, StrLit("+y")))     { result = Dir2_Up;    }
	else if (StrAnyCaseEquals(str, StrLit("d")))      { result = Dir2_Down;  }
	else if (StrAnyCaseEquals(str, StrLit("down")))   { result = Dir2_Down;  }
	else if (StrAnyCaseEquals(str, StrLit("b")))      { result = Dir2_Down;  } //NOTE: "b" is "bottom" is Dir2 land but is "backward" in Dir3 land!
	else if (StrAnyCaseEquals(str, StrLit("bottom"))) { result = Dir2_Down;  }
	else if (StrAnyCaseEquals(str, StrLit("-y")))     { result = Dir2_Down;  }
	else
	{
		SetOptionalOutPntr(errorOut, Result_UnknownString);
		return false;
	}
	SetOptionalOutPntr(valueOut, result);
	return true;
}

PEXP bool TryParseDir3(Str8 str, Dir3* valueOut, Result* errorOut)
{
	NotNullStr(str);
	str = TrimWhitespace(str);
	Dir3 result = Dir3_None;
	if      (StrAnyCaseEquals(str, StrLit("none")))     { result = Dir3_None;     }
	else if (StrAnyCaseEquals(str, StrLit("r")))        { result = Dir3_Right;    }
	else if (StrAnyCaseEquals(str, StrLit("right")))    { result = Dir3_Right;    }
	else if (StrAnyCaseEquals(str, StrLit("+x")))       { result = Dir3_Right;    }
	else if (StrAnyCaseEquals(str, StrLit("l")))        { result = Dir3_Left;     }
	else if (StrAnyCaseEquals(str, StrLit("left")))     { result = Dir3_Left;     }
	else if (StrAnyCaseEquals(str, StrLit("-x")))       { result = Dir3_Left;     }
	else if (StrAnyCaseEquals(str, StrLit("u")))        { result = Dir3_Up;       }
	else if (StrAnyCaseEquals(str, StrLit("up")))       { result = Dir3_Up;       }
	else if (StrAnyCaseEquals(str, StrLit("t")))        { result = Dir3_Up;       }
	else if (StrAnyCaseEquals(str, StrLit("top")))      { result = Dir3_Up;       }
	else if (StrAnyCaseEquals(str, StrLit("+y")))       { result = Dir3_Up;       }
	else if (StrAnyCaseEquals(str, StrLit("d")))        { result = Dir3_Down;     }
	else if (StrAnyCaseEquals(str, StrLit("down")))     { result = Dir3_Down;     }
	else if (StrAnyCaseEquals(str, StrLit("bottom")))   { result = Dir3_Down;     } //NOTE: "bottom" and "backward" share the first same letter, this might cause some confusion if someone uses these single letter directions
	else if (StrAnyCaseEquals(str, StrLit("-y")))       { result = Dir3_Down;     }
	else if (StrAnyCaseEquals(str, StrLit("f")))        { result = Dir3_Forward;  }
	else if (StrAnyCaseEquals(str, StrLit("forward")))  { result = Dir3_Forward;  }
	else if (StrAnyCaseEquals(str, StrLit("front")))    { result = Dir3_Forward;  }
	else if (StrAnyCaseEquals(str, StrLit("+z")))       { result = Dir3_Forward;  }
	else if (StrAnyCaseEquals(str, StrLit("b")))        { result = Dir3_Backward; }
	else if (StrAnyCaseEquals(str, StrLit("backward"))) { result = Dir3_Backward; }
	else if (StrAnyCaseEquals(str, StrLit("back")))     { result = Dir3_Backward; }
	else if (StrAnyCaseEquals(str, StrLit("-z")))       { result = Dir3_Backward; }
	else
	{
		SetOptionalOutPntr(errorOut, Result_UnknownString);
		return false;
	}
	SetOptionalOutPntr(valueOut, result);
	return true;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _CROSS_PARSING_AND_DIRECTIONS_H
