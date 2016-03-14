struct Elevator{
	int position;
        //double position;
        double position_estimate;
	int position_known;
	int state;
	int next;
  int direction;
	
};

void Elevator_init( struct Elevator * ele);

void Elevator_find_floor( struct Elevator * ele);

void Elevator_do(struct Elevator * ele);

int Elevator_check_floor();

int Elevator_unset_lamps(int floor);
enum Elevator_state{ HALT, SEEK, RUN};
enum Elevator_Direction{ELE_UP,ELE_DOWN};
