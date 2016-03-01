#include <stdio.h>
#include <string.h>
#include "state_machine.h"




void request_manager_init( struct RequestManager  *request_manager){
	Buttons_init( 	&request_manager->buttons_up, BUTTON_CALL_UP, 0,3);
	Buttons_init( 	&request_manager->buttons_down, BUTTON_CALL_DOWN, 1,4);
	Buttons_init( 	&request_manager->buttons_elevator, BUTTON_COMMAND, 0,4);
}
void request_manager_update( struct RequestManager  *request_manager){

	//request_manager->buttons_up.start=0;
	{
		struct Buttons buttons_up;
		memmove( &buttons_up, &request_manager->buttons_up, sizeof(struct Buttons));
		sweepButtons( &  buttons_up);
		int  equal = 1;
		for  ( int i = 0 ; i < 4 ; i++){
			if ( buttons_up.floor[i] != request_manager->buttons_up.floor[i]){
				equal = 0;
			}
		}
		if ( equal == 0 ){
			for  ( int i = 0 ; i < 4 ; i++){
				printf("%i ", buttons_up.floor[i]);
			}
			printf("\n");
		}
		memmove(  &request_manager->buttons_up,&buttons_up, sizeof(struct Buttons));
	}
	{
		struct Buttons buttons_down;
		memmove( &buttons_down, &request_manager->buttons_down, sizeof(struct Buttons));
		sweepButtons( &  buttons_down);
		int  equal = 1;
		for  ( int i = 0 ; i < 4 ; i++){
			if ( buttons_down.floor[i] != request_manager->buttons_down.floor[i]){
				equal = 0;
			}
		}
		if ( equal == 0 ){
			for  ( int i = 0 ; i < 4 ; i++){
				printf("%i ", buttons_down.floor[i]);
			}
			printf("\n");
		}
		memmove(  &request_manager->buttons_down,&buttons_down, sizeof(struct Buttons));

	}	//sweepButtons( & request_manager-> buttons_down);
}
void request_manager_unknown_enter_list( struct  RequestManager  *request_manager){}
void request_manager_stop_enter_list( struct  RequestManager  *request_manager){}
void request_manager_run_enter_list(  struct RequestManager  *request_manager){}

void request_manager_unknown_do_list(  struct RequestManager  *request_manager){}
void request_manager_stop_do_list(  struct RequestManager  *request_manager){}
void request_manager_run_do_list(  struct RequestManager  *request_manager){}
