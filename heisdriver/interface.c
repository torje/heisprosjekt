#include <string.h>
#include <stdio.h>
#include "interface.h"
void Buttons_init ( struct Buttons * buttons, elev_button_type_t type, int start, int limit){
	buttons->type =type;
	buttons->limit =limit;
	buttons->start =start;
	memset( buttons->floor,0, 4*sizeof(int));	
}

int buttons_update( struct Buttons * buttons){
	struct Buttons buttons_tmp= *buttons;
	sweepButtons( & buttons_tmp);
	int  equal = 1;
	for  ( int i = 0 ; i < 4 ; i++){
		if ( buttons_tmp.floor[i] != buttons->floor[i]){
			equal = 0;
		}
	}
	if ( equal == 0 ){
		for  ( int i = 0 ; i < 4 ; i++){
			//printf("%i ", buttons_tmp.floor[i]);
		}
		//printf("\n");
		*buttons =buttons_tmp;
		return 1;
	}
	return -1;
}
void sweepButtons( struct Buttons *buttons){
	for (  int floor = buttons->start ; floor < buttons->limit; floor++){
	        //printf("type: %d\n", buttons->type);
		int state = elev_get_button_signal( buttons->type, floor);
		buttons->floor[floor] = state;
		//printf("state: %d", state);
	}
}	
