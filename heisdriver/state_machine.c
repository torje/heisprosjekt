#include <stdio.h>
#include <string.h>
#include "state_machine.h"




void request_manager_init( struct RequestManager  *request_manager){
	Buttons_init( 	&request_manager->buttons_up, BUTTON_CALL_UP, 0,3);
	Buttons_init( 	&request_manager->buttons_down, BUTTON_CALL_DOWN, 1,4);
	Buttons_init( 	&request_manager->buttons_elevator, BUTTON_COMMAND, 0,4);
}
void request_manager_update( struct RequestManager  *request_manager){
	buttons_update(&request_manager->buttons_up);
	buttons_update(&request_manager->buttons_down);
	buttons_update(&request_manager->buttons_elevator);
}
void request_manager_unknown_enter_list( struct  RequestManager  *request_manager){}
void request_manager_stop_enter_list( struct  RequestManager  *request_manager){}
void request_manager_run_enter_list(  struct RequestManager  *request_manager){}

void request_manager_unknown_do_list(  struct RequestManager  *request_manager){}
void request_manager_stop_do_list(  struct RequestManager  *request_manager){}
void request_manager_run_do_list(  struct RequestManager  *request_manager){}
