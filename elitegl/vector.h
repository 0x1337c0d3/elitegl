/*
 * Elite - The New Kind.
 *
 * Reverse engineered from the BBC disk version of Elite.
 * Additional material by C.J.Pinder.
 *
 * The original Elite code is (C) I.Bell & D.Braben 1984.
 * This version re-engineered in C by C.J.Pinder 1999-2001.
 *
 * email: <christian@newkind.co.uk>
 *
 */

/**
 * SDL/OpenGL Port by Mark Follett 2001-2002
 * email: <mef123@geocities.com>
 **/

#ifndef VECTOR_H
#define VECTOR_H

struct vector
{
	double x;
	double y;
	double z;
};

struct vector4
{
	double x;
	double y;
	double z;
	double w;
};

typedef struct vector  Matrix[3];
typedef struct vector  Vector;
typedef struct vector4 Matrix4[4];
typedef struct vector4 Quaternion;

void mult_matrix (struct vector *first, struct vector *second);
void mult_vector (struct vector *vec, struct vector *mat);
double vector_dot_product (struct vector *first, struct vector *second);
struct vector unit_vector (struct vector *vec);
void set_init_matrix (struct vector *mat);
void tidy_matrix (struct vector *mat);

void mult_vector4 (struct vector *vec, Matrix4 matrix);
void matrix_to_quat (Matrix matrix, Quaternion *quat);
void quat_to_matrix (Quaternion *quat, Matrix matrix);
void quaternion_slerp (Quaternion *from, Quaternion *to, double t, Quaternion *res);

#endif

