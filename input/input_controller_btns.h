/*
File:   input_controller_btns.h
Author: Taylor Robbins
Date:   02\03\2025
*/

#ifndef _INPUT_CONTROLLER_BTNS_H
#define _INPUT_CONTROLLER_BTNS_H

#include "base/base_defines_check.h"
#include "base/base_macros.h"

typedef enum ContBtn ContBtn;
enum ContBtn
{
	ContBtn_None = 0,
	
	ContBtn_A, //Aka Bottom Face Btn
	ContBtn_B, //Aka Right Face Btn
	ContBtn_X, //Aka Left Face Btn
	ContBtn_Y, //Aka Top Face Btn
	
	ContBtn_Right,
	ContBtn_Left,
	ContBtn_Up,
	ContBtn_Down,
	
	ContBtn_LeftBumper, //aka L1 on Playstation
	ContBtn_RightBumper, //aka R1 on Playstation
	ContBtn_LeftTrigger, //aka L2 on Playstation
	ContBtn_RightTrigger, //aka R2 on Playstation
	ContBtn_LeftStick, //aka L3 on Playstation
	ContBtn_RightStick, //aka R3 on Playstation
	
	ContBtn_Start,
	ContBtn_Back,
	ContBtn_Console, //aka playstation or xbox btn, usually in the bottom middle
	ContBtn_Pad, //Ps4/Ps5 touch pad
	ContBtn_Microphone,
	
	ContBtn_lsRight,
	ContBtn_lsLeft,
	ContBtn_lsUp,
	ContBtn_lsDown,
	
	ContBtn_rsRight,
	ContBtn_rsLeft,
	ContBtn_rsUp,
	ContBtn_rsDown,
	
	ContBtn_Count,
};

#if !PIG_CORE_IMPLEMENTATION
const char* GetContBtnStr(ContBtn enumValue);
#else
PEXP const char* GetContBtnStr(ContBtn enumValue)
{
	switch (enumValue)
	{
		case ContBtn_None: return "None";
		case ContBtn_A: return "A";
		case ContBtn_B: return "B";
		case ContBtn_X: return "X";
		case ContBtn_Y: return "Y";
		case ContBtn_Right: return "Right";
		case ContBtn_Left: return "Left";
		case ContBtn_Up: return "Up";
		case ContBtn_Down: return "Down";
		case ContBtn_LeftBumper: return "LeftBumper";
		case ContBtn_RightBumper: return "RightBumper";
		case ContBtn_LeftTrigger: return "LeftTrigger";
		case ContBtn_RightTrigger: return "RightTrigger";
		case ContBtn_LeftStick: return "LeftStick";
		case ContBtn_RightStick: return "RightStick";
		case ContBtn_Start: return "Start";
		case ContBtn_Back: return "Back";
		case ContBtn_Console: return "Console";
		case ContBtn_Pad: return "Pad";
		case ContBtn_Microphone: return "Microphone";
		case ContBtn_lsRight: return "lsRight";
		case ContBtn_lsLeft: return "lsLeft";
		case ContBtn_lsUp: return "lsUp";
		case ContBtn_lsDown: return "lsDown";
		case ContBtn_rsRight: return "rsRight";
		case ContBtn_rsLeft: return "rsLeft";
		case ContBtn_rsUp: return "rsUp";
		case ContBtn_rsDown: return "rsDown";
		default: return UNKNOWN_STR;
	}
}
#endif

#endif //  _INPUT_CONTROLLER_BTNS_H
