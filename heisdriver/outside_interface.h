#pragma once

struct outside_interface{
	int down_lights[3];
	int up_lights[3];
};

enum out_light_direction {UP,DOWN};
enum light_state{ON,OFF};

void interface_light( struct outside_interface *interface, enum out_light_direction which, enum light_state state, int floor);
void set_down_light(struct outside_interface *interface, int floor);
void reset_down_light(struct outside_interface *interface, int floor);


void set_up_light(struct outside_interface *interface, int floor);

void reset_up_light(struct outside_interface *interface, int floor);
