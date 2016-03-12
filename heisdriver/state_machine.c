#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "state_machine.h"
#include "elevator_module.h"
#include "outside_interface.h"




void request_manager_init( struct RequestManager  *request_manager){
  Buttons_init( 	&request_manager->buttons_up, BUTTON_CALL_UP, 0,3);
  Buttons_init( 	&request_manager->buttons_down, BUTTON_CALL_DOWN, 1,4);
  Buttons_init( 	&request_manager->buttons_elevator, BUTTON_COMMAND, 0,4);
  memset(	request_manager -> up_queue,0, 4*sizeof(int));
  memset(	request_manager -> down_queue,0, 4*sizeof(int));
  request_manager ->runstate = RS_RUN;
  request_manager -> direction = 1;
  request_manager -> next_floor = -1;
}
void request_manager_update( struct RequestManager  *request_manager){
  buttons_update(&request_manager->buttons_up);
  buttons_update(&request_manager->buttons_down);
  buttons_update(&request_manager->buttons_elevator);
  switch( request_manager -> runstate ){
    case RS_RUN:
      request_manager_run_do_list(request_manager);
      break;
    case RS_STOP:
      break;
      default:
      printf("switch failure: runstate: %d\n", request_manager ->runstate);
    }
}

int queue_next( int queue[4] , int start, int limit, int step){
  /*printf("(%d,%d,%d)[ ", start, limit, step);
  for( int i = start; i != limit; i+=step){
    printf("%d ", queue[i]);
  }
  printf("]\n");*/
  for( int i = start; i != limit; i+=step){
    if ( queue[i] ){
      return i;
    }
  }
  
  return -1;
}

void request_manager_unknown_enter_list( struct  RequestManager  *request_manager){}
void request_manager_stop_enter_list( struct  RequestManager  *request_manager){}
void request_manager_run_enter_list(  struct RequestManager  *request_manager){}

void request_manager_unknown_do_list(  struct RequestManager  *request_manager){}
void request_manager_stop_do_list(  struct RequestManager  *request_manager){}
void request_manager_run_do_list(  struct RequestManager  *request_manager){
  int pos = request_manager -> ele->position;
  if ( request_manager ->ele-> position_known && request_manager->next_floor == -1) {
    //printf("asdkljasdkl\n");
    request_manager -> ele->state= HALT;
  }
  for ( int i = 0 ; i < 3 ; i++){
    if(    request_manager->buttons_up.floor[i] && !request_manager->up_queue[i]){
      request_manager->up_queue[i] = 1;
      printf("received order\n");
    }
  }
  
  for ( int i = 1 ; i < 4 ; i++){
    if(    request_manager->buttons_down.floor[i] && !request_manager->down_queue[i]){
      request_manager->down_queue[i] = 1;
      printf("received order\n");
    }
  }
  if ( request_manager-> ele->position_known){
  if ( 1 ==request_manager-> direction ){
     if (request_manager->up_queue[pos]){
      printf("I have arrived in a floor with an order\n");
      request_manager->up_queue[pos] = 0;
    }
    int frontPri = queue_next( request_manager->up_queue, pos, 4, 1);
    int downPri = queue_next( request_manager->down_queue, 3, -1, -1);
    int tailPri = queue_next( request_manager->up_queue, 0, pos, 1);
    if ( -1 != frontPri ){
      request_manager -> next_floor = frontPri;
      request_manager -> ele->state = RUN;
    }else if ( -1 != downPri ){
      request_manager -> next_floor = downPri;    
      request_manager -> ele->state = RUN;
      request_manager -> direction  = -1;
      printf("changing direction to down: current_floor: %d frontPri,downPri,tailpri: %d, %d, %d\n", request_manager->ele->position, frontPri,downPri,tailPri );
      printf("upqueue  : ");
      for( int i = 0 ; i < 4 ;i++) {
        printf( "%d ", request_manager->up_queue[i]);
      }
      printf("\ndownqueue: ");
      for( int i = 0 ; i < 4 ;i++) {
        printf( "%d ", request_manager->down_queue[i]);
      }
      printf("\n");
    }else if ( -1 != tailPri ){
      request_manager -> next_floor = tailPri;    
      request_manager -> ele->state = RUN;
    }else{
      request_manager -> next_floor = -1;  
      request_manager -> ele->state = HALT;    
    }

    
  }else if ( -1 == request_manager->direction ){
    if (request_manager->down_queue[pos]){
      printf("I have arrived in a floor with an order\n");
      request_manager->down_queue[pos] = 0;
    }
    int frontPri = queue_next( request_manager->down_queue, pos, -1, -1);
    int upPri = queue_next( request_manager->up_queue, 0, 4, 1);
    int tailPri = queue_next( request_manager->down_queue, 3, pos, -1);
    if ( -1 != frontPri ){
      request_manager -> next_floor = frontPri;
      request_manager -> ele->state = RUN;
    }else if ( -1 != upPri ) {
      request_manager -> next_floor = upPri;
      request_manager -> ele->state = RUN;
      request_manager -> direction  = 1;
      
      printf("changing direction to up: current_floor: %d frontPri,upPri,tailpri: %d, %d, %d\n", request_manager->ele->position,frontPri,upPri,tailPri );
      printf("upqueue  : ");
      for( int i = 0 ; i < 4 ;i++) {
        printf( "%d ", request_manager->up_queue[i]);
      }
      printf("\ndownqueue: ");
      for( int i = 0 ; i < 4 ;++i) {
        printf( "%d ", request_manager->down_queue[i]);
      }
      printf("\n");
    }else if ( -1 != tailPri ) {
      request_manager -> next_floor = tailPri;
      request_manager -> ele->state = RUN;
    }else { 
      request_manager -> next_floor = -1;  
      request_manager -> ele->state = HALT;    
    }
    }
  

  }
}

int requestManager_next( struct RequestManager *rm ){
        return rm->next_floor;
}
