/*
File:   app_main.h
Author: Taylor Robbins
Date:   02\25\2025
*/

#ifndef _APP_MAIN_H
#define _APP_MAIN_H

typedef struct AppData AppData;
struct AppData
{
	bool initialized;
	RandomSeries random;
};

#endif //  _APP_MAIN_H
