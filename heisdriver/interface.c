#include <string.h>
#include "interface.h"
void Buttons_init ( struct Buttons * buttons, elev_button_type_t type, int start, int limit){
	buttons->type =type;
	buttons->limit =limit;
	buttons->start =start;
	memset( buttons->floor,0, 4*sizeof(int));	
}
void sweepButtons( struct Buttons *buttons){
	for (  int floor = buttons->start ; floor < buttons->limit; floor++){
		int state = elev_get_button_signal( buttons->type, floor);
		buttons->floor[floor] = state;
	}
}	
