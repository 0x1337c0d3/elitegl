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

/*
 * The original Elite code did all the vector calculations using 8-bit integers.
 *
 * Writing all the routines in C to use 8 bit ints would have been fairly pointless.
 * I have, therefore, written a new set of routines which use floating point math.
 */

#include <stdlib.h>
#include <math.h>

#include "config.h"
#include "vector.h"



static Matrix start_matrix =
{
	{1.0, 0.0, 0.0},
	{0.0, 1.0, 0.0},
	{0.0, 0.0,-1.0}
};



/*
 * Multiply first matrix by second matrix.
 * Put result into first matrix.
 */


void mult_matrix (struct vector *first, struct vector *second)
{
	int i;
	Matrix rv;

	for (i = 0; i < 3; i++)
	{

		rv[i].x =	(first[0].x * second[i].x) +
				 	(first[1].x * second[i].y) +
					(first[2].x * second[i].z);

		rv[i].y =	(first[0].y * second[i].x) +
					(first[1].y * second[i].y) +
					(first[2].y * second[i].z);

		rv[i].z =	(first[0].z * second[i].x) +
					(first[1].z * second[i].y) +
					(first[2].z * second[i].z);
	}

	for (i = 0; i < 3; i++)
		first[i] = rv[i];
}




void mult_vector (struct vector *vec, struct vector *mat)
{
	double x;
	double y;
	double z;

	x = (vec->x * mat[0].x) +
		(vec->y * mat[0].y) +
		(vec->z * mat[0].z);

	y = (vec->x * mat[1].x) +
		(vec->y * mat[1].y) +
		(vec->z * mat[1].z);

	z = (vec->x * mat[2].x) +
		(vec->y * mat[2].y) +
		(vec->z * mat[2].z);

	vec->x = x;
	vec->y = y;
	vec->z = z;
}


/*
 * Calculate the dot product of two vectors sharing a common point.
 * Returns the cosine of the angle between the two vectors.
 */


double vector_dot_product (struct vector *first, struct vector *second)
{
	return (first->x * second->x) + (first->y * second->y) + (first->z * second->z);	
}



/*
 * Convert a vector into a vector of unit (1) length.
 */

struct vector unit_vector (struct vector *vec)
{
	double lx,ly,lz;
	double uni;
	struct vector res;

	lx = vec->x;
	ly = vec->y;
	lz = vec->z;

	uni = sqrt (lx * lx + ly * ly + lz * lz);

	res.x = lx / uni;
	res.y = ly / uni;
	res.z = lz / uni;
	
	return res;
}





void set_init_matrix (struct vector *mat)
{
	int i;

	for (i = 0; i < 3; i++)
		mat[i] = start_matrix[i];
}



void tidy_matrix (struct vector *mat)
{
	mat[2] = unit_vector (&mat[2]);

	if ((mat[2].x > -1) && (mat[2].x < 1))
	{
		if ((mat[2].y > -1) && (mat[2].y < 1))
		{
			mat[1].z = -(mat[2].x * mat[1].x + mat[2].y * mat[1].y) / mat[2].z;
		}
		else
		{
			mat[1].y = -(mat[2].x * mat[1].x + mat[2].z * mat[1].z) / mat[2].y;
		}
	}
	else
	{
		mat[1].x = -(mat[2].y * mat[1].y + mat[2].z * mat[1].z) / mat[2].x;
	}
	
	mat[1] = unit_vector (&mat[1]);
	

	/* xyzzy... nothing happens. :-)*/
	
	mat[0].x = mat[1].y * mat[2].z - mat[1].z * mat[2].y;
	mat[0].y = mat[1].z * mat[2].x - mat[1].x * mat[2].z;
	mat[0].z = mat[1].x * mat[2].y - mat[1].y * mat[2].x;
}


void mult_vector4 (struct vector *vec, Matrix4 matrix)
{
	double x;
	double y;
	double z;

	x = (vec->x * matrix[0].x) +
		(vec->y * matrix[1].x) +
		(vec->z * matrix[2].x) + matrix[3].x;

	y = (vec->x * matrix[0].y) +
		(vec->y * matrix[1].y) +
		(vec->z * matrix[2].y) + matrix[3].y;

	z = (vec->x * matrix[0].z) +
		(vec->y * matrix[1].z) +
		(vec->z * matrix[2].z) + matrix[3].z;

	vec->x = x;
	vec->y = y;
	vec->z = z;
}


void quat_normalize (Quaternion *q)
{
	double normal;
	normal = ((q->w * q->w) + (q->x * q->x) + (q->y * q->y) + (q->z * q->z));
	q->w = q->w / normal;
	q->x = q->x / normal;
	q->y = q->y / normal;
	q->z = q->z / normal;
}


void matrix_to_quat (Matrix matrix, Quaternion *quat)
{
    double m[3][3];
    double tr, s, q[4];
    int i, j, k;
    int nxt[3] = {1, 2, 0};

    tidy_matrix(matrix);

	m[0][0] = matrix[0].x;
	m[0][1] = matrix[0].y;
	m[0][2] = matrix[0].z;

	m[1][0] = matrix[1].x;
	m[1][1] = matrix[1].y;
	m[1][2] = matrix[1].z;

	m[2][0] = matrix[2].x;
	m[2][1] = matrix[2].y;
	m[2][2] = matrix[2].z;

    tr = m[0][0] + m[1][1] + m[2][2];

    // check the diagonal
    if (tr > 0.005f)
    {
        s = (double)(sqrt (tr + 1.0f));
        quat->w = s * 0.5f;
        s = (double)(0.5f / s);
        quat->x = (m[1][2] - m[2][1]) * s;
        quat->y = (m[2][0] - m[0][2]) * s;
        quat->z = (m[0][1] - m[1][0]) * s;
    } else
    {
	    // diagonal is negative
        i = 0;
        if (m[1][1] > m[0][0]) i = 1;
	    if (m[2][2] > m[i][i]) i = 2;
        j = nxt[i];
        k = nxt[j];

        s = (double)(sqrt ((m[i][i] - (m[j][j] + m[k][k])) + 1.0));
      
	    q[i] = s * 0.5f;
            
        if (s != 0.0) s = (double)(0.5f / s);

	    q[3] = (m[j][k] - m[k][j]) * s;
        q[j] = (m[i][j] + m[j][i]) * s;
        q[k] = (m[i][k] + m[k][i]) * s;

	    quat->x = q[0];
	    quat->y = q[1];
	    quat->z = q[2];
	    quat->w = q[3];
    }

	quat_normalize(quat);
}


void quat_to_matrix (Quaternion *quat, Matrix matrix)
{
    double wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

    quat_normalize(quat);

    // calculate coefficients
    x2 = quat->x + quat->x; y2 = quat->y + quat->y; 
    z2 = quat->z + quat->z;
    xx = quat->x * x2;   xy = quat->x * y2;   xz = quat->x * z2;
    yy = quat->y * y2;   yz = quat->y * z2;   zz = quat->z * z2;
    wx = quat->w * x2;   wy = quat->w * y2;   wz = quat->w * z2;

    matrix[0].x = 1.0f - (yy + zz);
    matrix[1].x = xy - wz;
    matrix[2].x = xz + wy;
 
    matrix[0].y = xy + wz;
    matrix[1].y = 1.0f - (xx + zz);
    matrix[2].y = yz - wx;

    matrix[0].z = xz - wy;
    matrix[1].z = yz + wx;
    matrix[2].z = 1.0f - (xx + yy);

    tidy_matrix(matrix);
}


void quaternion_slerp (Quaternion *from, Quaternion *to, double t, Quaternion *res)
{
    double to1[4];
    double omega, cosom, sinom, scale0, scale1;

    // calc cosine
    cosom = from->x * to->x + from->y * to->y + from->z * to->z
			+ from->w * to->w;

    // adjust signs (if necessary)
    if ( cosom < 0.0 )
    {
        cosom = -cosom; to1[0] = - to->x;
		to1[1] = - to->y;
		to1[2] = - to->z;
		to1[3] = - to->w;
    } else
    {
		to1[0] = to->x;
		to1[1] = to->y;
		to1[2] = to->z;
		to1[3] = to->w;
    }

    // calculate coefficients

    if ( (1.0 - cosom) > 0.0f )
    {
        // standard case (slerp)
        omega = acos(cosom);
        sinom = sin(omega);
        scale0 = sin((1.0 - t) * omega) / sinom;
        scale1 = sin(t * omega) / sinom;
    } else
    {        
        // "from" and "to" quaternions are very close 
	    //  ... so we can do a linear interpolation
        scale0 = 1.0 - t;
        scale1 = t;
    }

	// calculate final values
	res->x = scale0 * from->x + scale1 * to1[0];
	res->y = scale0 * from->y + scale1 * to1[1];
	res->z = scale0 * from->z + scale1 * to1[2];
	res->w = scale0 * from->w + scale1 * to1[3];

	quat_normalize(res);
}
