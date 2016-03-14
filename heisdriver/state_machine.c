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
  request_manager ->runstate = RS_RUN;
  request_manager -> direction = 1;
  request_manager -> next_floor = -1;
  request_manager ->stop_time = 0.5;
  request_manager ->ease_in_time = 0.25;
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
  
#ifdef DEBUGupd
  printf("state: %d", request_manager->runstate);
#endif
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
  
  memset(	request_manager -> up_queue,0, 4*sizeof(int));
  memset(	request_manager -> down_queue,0, 4*sizeof(int));
  Elevator_unset_lamps(0);
  Elevator_unset_lamps(1);
  Elevator_unset_lamps(2);
  Elevator_unset_lamps(3);
}


void request_manager_floor_stop_enter_list( struct  RequestManager  *rm){
  clock_gettime( CLOCK_MONOTONIC, &rm->timer);
  rm->ele->state=HALT;
  int pos = Elevator_check_floor();
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

}

void request_manager_unknown_do_list(  struct RequestManager  *request_manager) {}
void request_manager_stop_do_list(  struct RequestManager  *request_manager) {
  if ( !request_manager_check_stop(request_manager)){
    request_manager -> runstate=RS_RUN;
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
    rm -> runstate = RS_RUN;
  }
  return;
}

void request_manager_take_orders( struct RequestManager *request_manager){ 
  for ( int i = 0 ; i < 3 ; i++) {
    if(    request_manager->buttons_up.floor[i] /*&& !request_manager->up_queue[i]*/) {
      request_manager->up_queue[i] = 1;
      //printf("received order\n");
      elev_set_button_lamp(BUTTON_CALL_UP, i,1 );
    }
  }

  for ( int i = 1 ; i < 4 ; i++) {
    if(    request_manager->buttons_down.floor[i] /*&& !request_manager->down_queue[i]*/) {
      request_manager->down_queue[i] = 1;
      //printf("received order\n");
      elev_set_button_lamp(BUTTON_CALL_DOWN, i,1 );
    }
  }
  
  for ( int i = 0 ; i < 4 ; i++) {
    if(    request_manager->buttons_elevator.floor[i] /*&& !request_manager->up_queue[i]*/) {
      request_manager->up_queue[i] = 1;
      elev_set_button_lamp(BUTTON_COMMAND, i,1 );
      //printf("received order\n");
    }
    if(    request_manager->buttons_elevator.floor[i] /*&& !request_manager->down_queue[i]*/) {
      request_manager->down_queue[i] = 1;
      elev_set_button_lamp(BUTTON_COMMAND, i,1 );
      //printf("received order\n");
    }
  } 
}
void request_manager_run_do_list(  struct RequestManager  *request_manager) {
  if (request_manager_check_stop(request_manager)){
    return;
  }
  
  int pos = request_manager -> ele->position;
  if ( request_manager ->ele-> position_known && request_manager->next_floor == -1) {
    //request_manager -> ele->state= HALT;
  }
  if ( request_manager-> ele->position_known) {
    request_manager_take_orders(request_manager);
    if ( 1 ==request_manager-> direction ) {
      if (-1 != Elevator_check_floor() && request_manager->up_queue[Elevator_check_floor()] ) {
	request_manager->runstate = RS_FLOOR_STOP;
      }
      int frontPri = queue_next( request_manager->up_queue, pos+1, 4, 1);
      int downPri = queue_next( request_manager->down_queue, 3, -1, -1);
      int tailPri = queue_next( request_manager->up_queue, 0, pos, 1);
      if ( -1 != frontPri ) {
	request_manager -> next_floor = frontPri;
	request_manager -> ele->state = RUN;
      } else if ( -1 != downPri ) {
 	if ( request_manager->ele->position < downPri ){
	  request_manager->up_queue[downPri]=1;
	  request_manager -> next_floor = downPri;
	  request_manager -> ele->state = RUN;
	}else{
	  request_manager -> next_floor = downPri;
	  request_manager -> ele->state = RUN;
	  request_manager -> direction  = -1;
#ifdef DEBUG 
	  printf("changing direction to down: current_floor: %d frontPri,downPri,tailpri: %d, %d, %d\n", request_manager->ele->position, frontPri,downPri,tailPri );
	  printf("upqueue  : ");
	  for( int i = 0 ; i < 4 ; i++) {
	    printf( "%d ", request_manager->up_queue[i]);
	  }
	  printf("\ndownqueue: ");
	  for( int i = 0 ; i < 4 ; i++) {
	    printf( "%d ", request_manager->down_queue[i]);
	  }
	  printf("\n");
#endif
	}
      } else if ( -1 != tailPri ) {
	//printf("up tail\n");
	request_manager -> next_floor = tailPri;
	request_manager -> ele->state = RUN;
	request_manager -> direction = -1;
	request_manager -> down_queue[tailPri] = 1;
      } else {
	//printf("up no\n");
	request_manager -> next_floor = -1;
	request_manager -> ele->state = HALT;
      }
    } else if ( -1 == request_manager->direction ) {
      if (-1 != Elevator_check_floor() && request_manager->up_queue[Elevator_check_floor()]) {
	printf("I have arrived in a floor with an order\n");
	request_manager->runstate = RS_FLOOR_STOP;
      }
      int frontPri = queue_next( request_manager->down_queue, pos-1, -1, -1);
      int upPri = queue_next( request_manager->up_queue, 0, 4, 1);
      int tailPri = queue_next( request_manager->down_queue, 3, pos, -1);
      if ( -1 != frontPri ) {
	//printf("downFront");
	request_manager -> next_floor = frontPri;
	request_manager -> ele->state = RUN;
      } else if ( -1 != upPri ) {
	//printf("down up\n");
	if ( request_manager->ele->position > upPri ){
	  request_manager->down_queue[upPri]=1;
	  request_manager -> next_floor = upPri;
	  request_manager -> ele->state = RUN;
	}else{
	  request_manager -> next_floor = upPri;
	  request_manager -> ele->state = RUN;
	  request_manager -> direction  = 1;
	  #ifdef DEBUG
	  printf("changing direction to up: current_floor: %d frontPri,upPri,tailpri: %d, %d, %d\n", request_manager->ele->position,frontPri,upPri,tailPri );
	  printf("upqueue  : ");
	  for( int i = 0 ; i < 4 ; i++) {
	    printf( "%d ", request_manager->up_queue[i]);
	  }
	  printf("\ndownqueue: ");
	  for( int i = 0 ; i < 4 ; ++i) {
	    printf( "%d ", request_manager->down_queue[i]);
	  }
	  printf("\n");
	  #endif
	}
      } else if ( -1 != tailPri ) {
	//printf("down tail\n");
	request_manager -> next_floor = tailPri;
	request_manager -> ele->state = RUN;
	request_manager -> direction = 1;
	request_manager -> up_queue[tailPri] = 1;
      } else {
	//printf("down no\n");
	request_manager -> next_floor = -1;
	request_manager -> ele->state = HALT;
      }
    }


  }
}

int requestManager_next( struct RequestManager *rm ) {
  return rm->next_floor;
}
