#include <stdio.h>
#include <stdlib.h>
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

int Elevator_unset_lamps(int floor){
  if( 3!=floor){
    elev_set_button_lamp(BUTTON_CALL_UP, floor,0 );
  }
  if (0!=floor ){
    elev_set_button_lamp(BUTTON_CALL_DOWN, floor,0 );
  }
  elev_set_button_lamp(BUTTON_COMMAND, floor, 0 );
  return 0;
}
int Elevator_check_floor(){
  return elev_get_floor_sensor_signal();
}

void Elevator_do(struct Elevator * ele){
  switch( ele->state ) {
  case HALT:
    elev_set_motor_direction(DIRN_STOP);
    break;
  case SEEK:
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
      int floorSense;
      if (-1 != (floorSense = Elevator_check_floor())){
	ele->position = floorSense;
      }
      if ( ele->next != -1){
	if (ele->next - ele->position == 0     && -1 != Elevator_check_floor() ){
	  elev_set_motor_direction(DIRN_STOP);
	}else if(ele->next - ele->position== 0 && -1 == Elevator_check_floor() ){
	  if ( ELE_UP == ele->direction ){
	    elev_set_motor_direction(DIRN_DOWN);
	  }else{
	    elev_set_motor_direction(DIRN_UP);
	  }
	}else if ( ele->next - ele->position < 0 ) {
	  ele -> direction = ELE_DOWN;
	  elev_set_motor_direction(DIRN_DOWN);
	}else if ( ele->next - ele->position > 0 ) {
	  ele -> direction = ELE_UP;
	  elev_set_motor_direction(DIRN_UP);
	}else{
	  elev_set_motor_direction(DIRN_STOP);
	}
      }else{
	elev_set_motor_direction(DIRN_STOP);
      }
    }
    break;
  default:
    printf("oupsie, elevator died\n");
    exit(0);
    break;
  }
  
}
