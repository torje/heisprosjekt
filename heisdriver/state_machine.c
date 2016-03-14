#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "state_machine.h"
#include "elevator_module.h"
#include "outside_interface.h"
#include "timer.h"




void request_manager_init( struct RequestManager  *request_manager) {
  Buttons_init( 	&request_manager->buttons_up, BUTTON_CALL_UP, 0,3);
  Buttons_init( 	&request_manager->buttons_down, BUTTON_CALL_DOWN, 1,4);
  Buttons_init( 	&request_manager->buttons_elevator, BUTTON_COMMAND, 0,4);
  memset(	request_manager -> up_queue,0, 4*sizeof(int));
  memset(	request_manager -> down_queue,0, 4*sizeof(int));
  request_manager ->runstate = RS_UNKNOWN;
  request_manager -> direction = 1;
  request_manager -> next_floor = -1;
  request_manager ->stop_time = 0.5;
}

int request_manager_check_stop( struct RequestManager *rm ) {
  if (elev_get_stop_signal()) {
    rm->runstate=RS_STOP;
    return 1;
  }else{
    return 0;
  }
}

void request_manager_update( struct RequestManager  *request_manager) {
  buttons_update(&request_manager->buttons_up);
  buttons_update(&request_manager->buttons_down);
  buttons_update(&request_manager->buttons_elevator);
  
  int runstate = request_manager->runstate;
  
  switch( request_manager -> runstate ) {
  case RS_RUN:
    request_manager_run_do_list(request_manager);
    break;
  case RS_STOP:
    request_manager_stop_do_list(request_manager);
    break;
  case RS_FLOOR_STOP:
    request_manager_floor_stop_do_list(request_manager);
    break;
  case RS_UNKNOWN:
    request_manager_unknown_do_list( request_manager);
    break;
  default:
    printf("switch failure: runstate: %d\n", request_manager ->runstate);
    elev_init();
    exit(1);
  }

  if ( runstate != request_manager->runstate){
    switch( request_manager->runstate ){
    case RS_FLOOR_STOP:
    printf("state transition floor stop\n");
      request_manager_floor_stop_enter_list(request_manager);
      break;
    case RS_STOP:
    printf("state transition stop\n");
      request_manager_stop_enter_list(request_manager);
      break;
    case RS_RUN:
      printf("state transition run\n");
      request_manager_run_enter_list(request_manager);
      break;
    case RS_UNKNOWN:
      request_manager_unknown_enter_list(request_manager);
      break;
    default:
      printf("failure\n");
      exit(1);
    }
  } 
}

int queue_next( int queue[4] , int start, int limit, int step) {
#ifdef DEBUG 
  printf("(%d,%d,%d)[ ", start, limit, step);
  for( int i = start; i != limit; i+=step){
    printf("%d ", queue[i]);
  }
  printf("]\n");
#endif
  for( int i = start; i != limit; i+=step) {
    if ( queue[i] ) {
      return i;
    }
  }

  return -1;
}

void request_manager_unknown_enter_list( struct  RequestManager  *request_manager) {}
void request_manager_stop_enter_list( struct  RequestManager  *request_manager) {
  request_manager ->ele->state = HALT;
  request_manager ->runstate = RS_STOP;
  elev_set_stop_lamp(1);
  if ( -1 != Elevator_check_floor() ){
    elev_set_door_open_lamp(1);
  }else{
  }

  memset(	request_manager -> up_queue,0, 4*sizeof(int));
  memset(	request_manager -> down_queue,0, 4*sizeof(int));
  Elevator_unset_lamps(0);
  Elevator_unset_lamps(1);
  Elevator_unset_lamps(2);
  Elevator_unset_lamps(3);
}
void request_manager_unknown_do_list(  struct RequestManager  *request_manager){
  if ( request_manager-> ele->position_known) {
    request_manager ->runstate=RS_RUN;
    request_manager->ele->position = Elevator_check_floor();
    printf("elevator pos = %d\n",request_manager->ele->position );
    if ( HALT == request_manager -> ele -> state ){
      request_manager -> ele -> state = RUN;
    }
  }
  
}


void request_manager_floor_stop_enter_list( struct  RequestManager  *rm){
  clock_gettime( CLOCK_MONOTONIC, &rm->timer);
  elev_set_floor_indicator( rm->ele->position);
  int pos = Elevator_check_floor();
  rm->ele->state=HALT;
  elev_set_door_open_lamp(1);
  rm->ele->position = pos;
  Elevator_unset_lamps(pos);
  if ( rm->direction == 1 ){
    int frontPri = queue_next( rm ->up_queue, pos, 4, 1);
    int downPri = queue_next( rm->down_queue, 3, -1, -1);
    if (downPri != -1 && downPri == frontPri ) {
      rm->down_queue[pos] = 0;
    }
    rm->up_queue[pos] = 0;
  }else{
    int frontPri = queue_next( rm->down_queue, pos, -1, -1);
    int upPri = queue_next( rm->up_queue, 0, 4, 1);
    if ( upPri != -1 && frontPri == upPri){
      rm->up_queue[pos] = 0;
    }
    rm->down_queue[pos] = 0;
  }


#ifdef DEBUG
  printf("should open door\n");
#endif
}
void request_manager_run_enter_list(  struct RequestManager  *request_manager) {
  request_manager_next(request_manager);
  
  request_manager -> ele->state = RUN;
  request_manager -> ele->next = request_manager->next_floor;
}

void request_manager_stop_do_list(  struct RequestManager  *request_manager) {
  if ( !request_manager_check_stop(request_manager)){
    request_manager -> runstate=RS_RUN;
    elev_set_stop_lamp(0);
    elev_set_door_open_lamp(0);
  }
}
void request_manager_floor_stop_do_list( struct  RequestManager  *rm){
  if (request_manager_check_stop(rm)){
    return;
  }
  struct timespec tmp;
  request_manager_take_orders(rm);
  if ( rm->up_queue[rm->ele->position] ||rm->down_queue[rm->ele->position] ){
    Elevator_unset_lamps(rm->ele->position);
    clock_gettime(CLOCK_MONOTONIC, &rm->timer);
  }
  clock_gettime(CLOCK_MONOTONIC, &tmp);
  
  rm->down_queue[Elevator_check_floor()] = 0;
  rm->up_queue[Elevator_check_floor()] = 0;
  if ( timespec_cmp( &rm ->timer, &tmp, rm->stop_time) ){
    printf("closing door\n");
    elev_set_door_open_lamp(0);
    rm -> runstate = RS_RUN;
  }
  return;
}

void request_manager_take_orders( struct RequestManager *request_manager){ 
  for ( int i = request_manager->buttons_up.start ; i < request_manager->buttons_up.limit ; i++) {
    if( request_manager->buttons_up.floor[i] ) {
      request_manager->up_queue[i] = 1;
      elev_set_button_lamp(BUTTON_CALL_UP, i,1 );
    }
  }

  for ( int i = request_manager->buttons_down.start ; i < request_manager->buttons_down.limit ; i++) {
    if( request_manager->buttons_down.floor[i] ) {
      request_manager->down_queue[i] = 1;
      elev_set_button_lamp(BUTTON_CALL_DOWN, i,1 );
    }
  }
  
  for ( int i = request_manager->buttons_elevator.start ; i < request_manager->buttons_elevator.limit ; i++) {
    if(    request_manager->buttons_elevator.floor[i]) {
      request_manager->up_queue[i] = 1;
      elev_set_button_lamp(BUTTON_COMMAND, i,1 );
    }
    if( request_manager->buttons_elevator.floor[i] ) {
      request_manager->down_queue[i] = 1;
      elev_set_button_lamp(BUTTON_COMMAND, i,1 );
    }
  } 
}
void request_manager_run_do_list(  struct RequestManager  *request_manager) {
  

  if (request_manager_check_stop(request_manager)){
      return;
  }
  if ( 1 == request_manager-> direction ) {
    if (-1 != Elevator_check_floor() &&  request_manager->up_queue[Elevator_check_floor()] ) {
      request_manager->ele->position = Elevator_check_floor();
      request_manager->runstate = RS_FLOOR_STOP;
    }
  }else if ( -1 == request_manager->direction ) {
    if (-1 != Elevator_check_floor() && request_manager->down_queue[Elevator_check_floor()]) {
      request_manager->ele->position = Elevator_check_floor();
      request_manager->runstate = RS_FLOOR_STOP;
    }
  }
  elev_set_floor_indicator( request_manager ->ele->position);
  request_manager -> ele -> next = request_manager_next(request_manager);
  request_manager_take_orders(request_manager);

  
}

int request_manager_next( struct RequestManager *request_manager ) {
  int pos = request_manager -> ele->position;  
  if ( 1 ==request_manager-> direction ) {
    int frontPri = queue_next( request_manager->up_queue, pos+1, 4, 1);
    int downPri  = queue_next( request_manager->down_queue, 3, -1, -1);
    int tailPri  = queue_next( request_manager->up_queue, 0, pos+1, 1);
    if ( -1 != frontPri ) {
      request_manager -> next_floor = frontPri;
    } else if ( -1 != downPri ) {
      if ( request_manager->ele->position < downPri ){
	request_manager->up_queue[downPri]=1;
	request_manager -> next_floor = downPri;
      }else{
	request_manager -> next_floor = downPri;
	request_manager -> direction  = -1;
      }
    } else if ( -1 != tailPri ) {
      request_manager -> next_floor = tailPri;
      request_manager -> direction = -1;
      request_manager -> down_queue[tailPri] = 1;
    } else {
      request_manager -> next_floor = -1;
    }
  } else if ( -1 == request_manager->direction ) {
    int frontPri = queue_next( request_manager->down_queue, pos-1, -1, -1);
    int upPri = queue_next( request_manager->up_queue, 0, 4, 1);
    int tailPri = queue_next( request_manager->down_queue, 3, pos-1, -1);
    if ( -1 != frontPri ) {
      request_manager -> next_floor = frontPri;
    } else if ( -1 != upPri ) {
      if ( request_manager->ele->position > upPri ){
	request_manager->down_queue[upPri]=1;
	request_manager -> next_floor = upPri;
      }else{
	request_manager -> next_floor = upPri;
	request_manager -> direction  = 1;
      }
    } else if ( -1 != tailPri ) {
      request_manager -> next_floor = tailPri;
      request_manager -> direction = 1;
      request_manager -> up_queue[tailPri] = 1;
    } else {
      request_manager -> next_floor = -1;
    }
  }


  return request_manager->next_floor;
}
