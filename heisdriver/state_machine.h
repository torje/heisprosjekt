#include <time.h>
#include "interface.h"
struct RequestManager{
  int super_state;
  int runstate;
  int prev_state;
  int up_queue[4];
  int down_queue[4];
  int direction;
  int next_floor;
  struct Elevator *ele;
  struct Buttons buttons_up;
  struct Buttons buttons_down;
  struct Buttons buttons_elevator;
  struct timespec timer;
  double stop_time;
  double ease_in_time;
};

enum Runstate{ RS_STOP, RS_RUN, RS_FLOOR_STOP };
void request_manager_init( struct RequestManager  *request_manager);
void request_manager_update( struct RequestManager  *request_manager);

void request_manager_take_orders( struct RequestManager *request_manager);


void request_manager_unknown_enter_list( struct  RequestManager  *request_manager);
void request_manager_stop_enter_list( struct  RequestManager  *request_manager);
void request_manager_floor_stop_enter_list( struct  RequestManager  *request_manager);
void request_manager_run_enter_list(  struct RequestManager  *request_manager);

void request_manager_unknown_do_list(  struct RequestManager  *request_manager);
void request_manager_stop_do_list(  struct RequestManager  *request_manager);
void request_manager_floor_stop_do_list( struct  RequestManager  *request_manager);
void request_manager_run_do_list(  struct RequestManager  *request_manager);
int requestManager_next( struct RequestManager *rm );
