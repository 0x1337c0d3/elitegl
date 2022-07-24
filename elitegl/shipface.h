#ifndef SHIPFACE_H
#define SHIPFACE_H

struct ship_face
{
	int colour;
	int norm_x;
	int norm_y;
	int norm_z;
	int points;	
	int p1;
	int p2;
	int p3;
	int p4;
	int p5;
	int p6;
	int p7;
	int p8;
};


struct ship_solid
{
	int num_faces;
	struct ship_face *face_data;
};

extern struct ship_solid ship_solids[];

#endif
