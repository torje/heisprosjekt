#pragma once
// Interface start
#include "elev.h"
struct Buttons{
	elev_button_type_t type;
	int floor[4];
	int start;
	int limit;
};

void Buttons_init ( struct Buttons * buttons, elev_button_type_t type, int start, int limit);
void sweepButtons( struct Buttons *buttons);
