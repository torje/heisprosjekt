#include "outside_interface.h"
#include "elev.h"

void interface_light( struct outside_interface *interface, enum out_light_direction which, enum light_state state, int floor){
	if ( which == DOWN ){
		if ( state == ON){
			set_down_light( interface, floor);
		}else{
			reset_down_light( interface, floor);
		}
	}else{//which is UP
		if ( state == ON){
			set_up_light( interface, floor);
		}else{
			reset_up_light( interface, floor);
		}
	}
}

void set_down_light(struct outside_interface *interface, int floor) {
	elev_set_button_lamp( BUTTON_CALL_DOWN, floor, 1);
	interface->down_lights[floor-1] = 1;
}

void reset_down_light(struct outside_interface *interface, int floor) {
	elev_set_button_lamp( BUTTON_CALL_DOWN, floor, 0);
	interface->down_lights[floor-1] = 0;
}

void set_up_light(struct outside_interface *interface, int floor) {
	elev_set_button_lamp( BUTTON_CALL_UP, floor, 1);
	interface->up_lights[floor] = 1;
}

void reset_up_light(struct outside_interface *interface, int floor) {
	elev_set_button_lamp( BUTTON_CALL_UP, floor, 0);
	interface->up_lights[floor] = 0;
}
