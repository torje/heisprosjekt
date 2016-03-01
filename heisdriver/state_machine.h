#include "interface.h"
struct RequestManager{
	int super_state;
	int runstate;
	int up_queue[4];
	int down_queue[4];
	struct Buttons buttons_up;
	struct Buttons buttons_down;
	struct Buttons buttons_elevator;
};

void request_manager_init( struct RequestManager  *request_manager);
void request_manager_update( struct RequestManager  *request_manager);
void request_manager_unknown_enter_list( struct  RequestManager  *request_manager);
void request_manager_stop_enter_list( struct  RequestManager  *request_manager);
void request_manager_run_enter_list(  struct RequestManager  *request_manager);

void request_manager_unknown_do_list(  struct RequestManager  *request_manager);
void request_manager_stop_do_list(  struct RequestManager  *request_manager);
void request_manager_run_do_list(  struct RequestManager  *request_manager);
