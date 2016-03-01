#include "elev.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include "interface.h"
#include "elevator_module.h"
#include "state_machine.h"



int sweepSensors(){
	int floorSensed = elev_get_floor_sensor_signal();
	return floorSensed;
}

void signal_interupt_handler( int a ){
	elev_set_motor_direction(DIRN_STOP);
	exit(0);
}
int main() {
	struct RequestManager rm;	
	request_manager_init( &rm);
	signal( SIGINT,signal_interupt_handler);
	signal( SIGTERM,signal_interupt_handler);
	struct  Elevator ele;
	Elevator_init( &ele);
	// Initialize hardware
	if (!elev_init()) {
		printf("Unable to initialize elevator hardware!\n");
		return 1;
	}

	printf("Press STOP button to stop elevator and exit program.\n");

	//elev_set_motor_direction(DIRN_UP);

	while (1) {

		// Sweeps start
		//sweepButtons();	
		request_manager_update( &rm);
		int floor = sweepSensors();
		// sweeps end
		if( floor != -1){
			ele.position_known = 1;
			ele.position = floor;
		}
		/*printf("known: %i -- position: %lf \n", ele.position_known, ele.position);*/
		/*
		for ( int i = 0 ; i  < 4 ; i++){
			//printf("%i %i %i , ",lampState[i][0],lampState[i][1],lampState[i][2]);
		}
		printf("\n");
		*/


		if ( ele.position_known == 0 ){
			elev_set_motor_direction(DIRN_UP);
		}else{
			elev_set_motor_direction(DIRN_STOP);
		}




		// Change direction when we reach top/bottom floor
		if (elev_get_floor_sensor_signal() == N_FLOORS - 1) {
			//elev_set_motor_direction(DIRN_DOWN);
		} else if (elev_get_floor_sensor_signal() == 0) {
			//elev_set_motor_direction(DIRN_UP);
		}

		// Stop elevator and exit program if the stop button is pressed
		if (elev_get_stop_signal()) {
			elev_set_motor_direction(DIRN_STOP);
			break;
		}
	}

	return 0;
}
