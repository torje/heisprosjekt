struct Elevator{
	int position;
        //double position;
        double position_estimate;
	int position_known;
	int state;
	int next;
	
};

void Elevator_init( struct Elevator * ele);

void Elevator_find_floor( struct Elevator * ele);

void Elevator_do(struct Elevator * ele);

enum Elevator_state{ HALT, SEEK, RUN};
