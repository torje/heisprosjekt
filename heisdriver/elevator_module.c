#include <stdio.h>
#include "elevator_module.h"
#include "elev.h"
#include "state_machine.h"


void Elevator_init( struct Elevator * ele){
	ele -> position = 404;
	ele -> position_known = 0;
	ele -> state = HALT;
	ele -> next = -1;
	ele -> position_estimate = 404;
}



void Elevator_find_floor( struct Elevator * ele){
	ele->state = SEEK;
}

int Elevator_check_floor(){
        return elev_get_floor_sensor_signal();
}

void Elevator_do(struct Elevator * ele){
	switch( ele->state ) {
		case HALT:
		        //printf("halting\n");
		        elev_set_motor_direction(DIRN_STOP);
			break;
		case SEEK:
		        //printf("seeking\n");
		        if ( ele->position_known == 0 ){
		                int floorSense;
		                if (-1 != (floorSense = Elevator_check_floor())){
		                        ele->position = floorSense;
		                        ele->position_known = 1;
        		                printf("found position\n");
		                }
		                elev_set_motor_direction(DIRN_UP);			
		        }else{
		                elev_set_motor_direction(DIRN_STOP);
                        }
                        break;
                case RUN:
                {
                        //printf("running\n");
                        int floorSense;
		        if (-1 != (floorSense = Elevator_check_floor())){
		                ele->position = floorSense;
		        }
		        //printf("floorSense: %d\n", floorSense);
		        if ( ele->next != -1){
		                //printf("pos: %d order: %d\n", ele->position, ele->next);
                                if ( ele->next - ele->position < 0 ) {
                                       elev_set_motor_direction(DIRN_DOWN);
        		        }else if ( ele->next - ele->position > 0 ) {
                                        elev_set_motor_direction(DIRN_UP);
        		        }else{
        		                elev_set_motor_direction(DIRN_STOP);
        		        }
        		}
                }
                        break;	
	}
}
