#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "state_machine.h"
#include "elevator_module.h"
#include "outside_interface.h"




void request_manager_init( struct RequestManager  *request_manager) {
  Buttons_init( 	&request_manager->buttons_up, BUTTON_CALL_UP, 0,3);
  Buttons_init( 	&request_manager->buttons_down, BUTTON_CALL_DOWN, 1,4);
  Buttons_init( 	&request_manager->buttons_elevator, BUTTON_COMMAND, 0,4);
  memset(	request_manager -> up_queue,0, 4*sizeof(int));
  memset(	request_manager -> down_queue,0, 4*sizeof(int));
  request_manager ->runstate = RS_RUN;
  request_manager -> direction = 1;
  request_manager -> next_floor = -1;
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
void request_manager_stop_enter_list( struct  RequestManager  *request_manager) {}
void request_manager_floor_stop_enter_list( struct  RequestManager  *rm){
  clock_gettime( CLOCK_MONOTONIC, &rm->timer);
  rm->ele->state=HALT;
  int pos = rm->ele->position;
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
void request_manager_run_enter_list(  struct RequestManager  *request_manager) {}

void request_manager_unknown_do_list(  struct RequestManager  *request_manager) {}
void request_manager_stop_do_list(  struct RequestManager  *request_manager) {}
void request_manager_floor_stop_do_list( struct  RequestManager  *rm){
  struct timespec tmp;
  request_manager_take_orders(rm);
  clock_gettime(CLOCK_MONOTONIC, &tmp);
  int secs = tmp.tv_sec -rm->timer.tv_sec;
  
  if ( secs > 3 ){
    printf("closing door\n");
    rm -> runstate = RS_RUN;
  }
}

void request_manager_take_orders( struct RequestManager *request_manager){ 
  for ( int i = 0 ; i < 3 ; i++) {
    if(    request_manager->buttons_up.floor[i] && !request_manager->up_queue[i]) {
      request_manager->up_queue[i] = 1;
      printf("received order\n");
    }
  }

  for ( int i = 1 ; i < 4 ; i++) {
    if(    request_manager->buttons_down.floor[i] && !request_manager->down_queue[i]) {
      request_manager->down_queue[i] = 1;
      printf("received order\n");
    }
  }
  
  for ( int i = 0 ; i < 4 ; i++) {
    if ( request_manager-> direction == 1){
      if(    request_manager->buttons_elevator.floor[i] && !request_manager->up_queue[i]) {
	request_manager->up_queue[i] = 1;
	printf("received order\n");
      }
    }else{ 
      if(    request_manager->buttons_elevator.floor[i] && !request_manager->down_queue[i]) {
	request_manager->down_queue[i] = 1;
	printf("received order\n");
      }
    }
  
}
void request_manager_run_do_list(  struct RequestManager  *request_manager) {
  int pos = request_manager -> ele->position;
  request_manager_take_orders(request_manager);
  if ( request_manager ->ele-> position_known && request_manager->next_floor == -1) {
    request_manager -> ele->state= HALT;
  }
  if ( request_manager-> ele->position_known) {
    if ( 1 ==request_manager-> direction ) {
      if (request_manager->up_queue[pos]) {
	printf("I have arrived in a floor with an order\n");
	request_manager->up_queue[pos] = 0;
	request_manager->runstate = RS_FLOOR_STOP;
      }
      int frontPri = queue_next( request_manager->up_queue, pos, 4, 1);
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
	}
      } else if ( -1 != tailPri ) {
	request_manager -> next_floor = tailPri;
	request_manager -> ele->state = RUN;
      } else {
	request_manager -> next_floor = -1;
	request_manager -> ele->state = HALT;
      }
    } else if ( -1 == request_manager->direction ) {
      if (request_manager->down_queue[pos]) {
	printf("I have arrived in a floor with an order\n");
	request_manager->down_queue[pos] = 0;
	request_manager->runstate = RS_FLOOR_STOP;
      }
      int frontPri = queue_next( request_manager->down_queue, pos, -1, -1);
      int upPri = queue_next( request_manager->up_queue, 0, 4, 1);
      int tailPri = queue_next( request_manager->down_queue, 3, pos, -1);
      if ( -1 != frontPri ) {
	request_manager -> next_floor = frontPri;
	request_manager -> ele->state = RUN;
      } else if ( -1 != upPri ) {
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
	request_manager -> next_floor = tailPri;
	request_manager -> ele->state = RUN;
      } else {
	request_manager -> next_floor = -1;
	request_manager -> ele->state = HALT;
      }
    }


  }
}

int requestManager_next( struct RequestManager *rm ) {
  return rm->next_floor;
}
