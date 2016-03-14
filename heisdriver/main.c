#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include "elev.h"
#include "interface.h"
#include "elevator_module.h"
#include "state_machine.h"
#include "unistd.h"



int sweepSensors(){
	int floorSensed = elev_get_floor_sensor_signal();
	return floorSensed;
}

void signal_interupt_handler( int a ){
	elev_set_motor_direction(DIRN_STOP);
	exit(0);
}
int main() {
	signal( SIGINT ,signal_interupt_handler);
	//signal( SIGTERM,signal_interupt_handler);
	
	struct RequestManager rm;	
	request_manager_init( &rm);
	struct  Elevator ele;
	Elevator_init( &ele);
	rm.ele = &ele;
	Elevator_find_floor(&ele);
	// Initialize hardware
	if (!elev_init()) {
		printf("Unable to initialize elevator hardware!\n");
		return 1;
	}
	

	
	while ( 1 ) {
		
		// Sweeps start
		//sweepButtons();	
		request_manager_update( &rm);
		Elevator_do(&ele);
		// sweeps end
		ele.next = requestManager_next(&rm);
		/*if ( ele.position_known==1){
		        ele.state = RUN;
		}*/
        	
        	
        	
        	usleep(1000);
        	/*if( floor != -1){
		        Elevator_do(&ele);
			ele.position_known = 1;
			ele.position = floor;
			//printf("Found floor");
		}*/
		/*printf("known: %i -- position: %lf \n", ele.position_known, ele.position);*/
		/*
		for ( int i = 0 ; i  < 4 ; i++){
			//printf("%i %i %i , ",lampState[i][0],lampState[i][1],lampState[i][2]);
		}
		printf("\n");
		*/

                /*
                
		if ( ele.position_known == 0 ){
			elev_set_motor_direction(DIRN_UP);
		}else{
			elev_set_motor_direction(DIRN_STOP);
		}
*/



		// Change direction when we reach top/bottom floor
		if (elev_get_floor_sensor_signal() == N_FLOORS - 1) {
			//elev_set_motor_direction(DIRN_DOWN);
		} else if (elev_get_floor_sensor_signal() == 0) {
			//elev_set_motor_direction(DIRN_UP);
		}

	}

	return 0;
}
