/**
 *
 * Elite - The New Kind.
 *
 * SDL/OpenGL version of Graphics routines.
 *
 * The code in this file has not been derived from the original Elite code.
 * Written by C.J.Pinder 1999-2001.
 * email: <christian@newkind.co.uk>
 *
 * Routines for drawing anti-aliased lines and circles by T.Harte.
 *
 **/

/**
 * SDL/OpenGL Port by Mark Follett 2001-2002
 * email: <mef123@geocities.com>
 **/


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

#include "config.h"
#include "gfx.h"
#include "main.h"
#include "elite.h"
#include "space.h"
#include "threed.h"
#include "random.h"
#include "shipface.h"
#include "file.h"

#ifdef WIN32
#  include <windows.h>
#endif

#include <OpenGL/gl.h>
#include <SDL2/SDL.h>

#define PI 3.1415926535898

#define FILLED_CIRCLE    1
#define WIREFRAME_CIRCLE 2

GLuint texture[20];
GLuint Elite_1_base;
GLuint Elite_2_base;

SDL_Window *window;
SDL_GLContext gContext;
SDL_Surface *scanner_image;

int clip_tx;
int clip_ty;
int clip_bx;
int clip_by;

Matrix4 camera;

/*
 * Return the pixel value at (x, y)
 * NOTE: The surface must be locked before calling this!
 */
Uint32 gfx_get_pixel (SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp)
    {
        case 1:
            return *p;

        case 2:
            return *(Uint16 *)p;

        case 3:
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
            else
                return p[0] | p[1] << 8 | p[2] << 16;

        case 4:
            return *(Uint32 *)p;

        default:
            return 0;       /* shouldn't happen, but avoids warnings */
    }
}

void gfx_get_char_size (SDL_Surface *bmp, int x, int y, int *size_x, int *size_y) {
	int dx, dy;
	Uint32 pixel;

	for (dy = 31; dy > 0; dy--)
    {
		for (dx = 31; dx > 0; dx--)
        {
			SDL_LockSurface(bmp);
			pixel = gfx_get_pixel(bmp, (x*32)+dx, (y*32)+dy);
			SDL_UnlockSurface(bmp);

			if (pixel == 0)
            {
				*size_x = dx;
				*size_y = dy;
				return;
			}
		}
	}

	*size_x = 0;
	*size_y = 0;
}

SDL_Surface *gfx_load_bitmap (char *filename)
{
	SDL_Surface *bmap;
	char *path;
	path = concat_paths(TEXTURE_DIRECTORY, filename);
	bmap = SDL_LoadBMP(path);
	free(path);
	return bmap;
}

GLuint gfx_load_texture (char *filename, int x, int y, int size)
{
	SDL_Surface *bmp, *bmp1;
	GLuint texture;
	SDL_Rect rect, rect1;
    Uint32 rmask, gmask, bmask, amask;

    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
       on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x00000000;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0x00000000;
#endif

	bmp = gfx_load_bitmap(filename);
	if (bmp == NULL)
	{
		printf("Error reading bitmap file: %s.\n", filename);
		return 1;
	}

	bmp1 = SDL_CreateRGBSurface(SDL_SWSURFACE, size, size, 24, rmask, gmask, bmask, amask);

	SDL_FillRect(bmp1, NULL, 0);

	rect.x = x;
	rect.y = y;
	rect.w = size;
	rect.h = size;

	rect1.x = 0;
	rect1.y = 0;
	rect1.w = size;
	rect1.h = size;

	SDL_BlitSurface(bmp, &rect, bmp1, &rect1);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, bmp1->pixels);

	SDL_FreeSurface(bmp);
	SDL_FreeSurface(bmp1);

	return texture;
}

GLuint gfx_load_font (char *filename, char *maskname)
{
	SDL_Surface *fontmask;
	GLuint base, texture1, texture2;
	int size_x, size_y;
	int x, y, cx;

	fontmask = gfx_load_bitmap(maskname);
	if (!fontmask)
	{
		printf("Error reading font bitmap file: %s.\n", maskname);
		exit(0);
	}
	
	texture1 = gfx_load_texture(filename, 0, 0, 256);
	texture2 = gfx_load_texture(filename, 256, 0, 256);

	base = glGenLists(96);

	for (y = 0; y < 6; y++) {
		for (x = 0; x < 16; x++) {

			glNewList(base + (y * 16) + x, GL_COMPILE);
			if (x > 7) {
				glBindTexture(GL_TEXTURE_2D, texture2);
				cx = x - 8;
			} else {
				glBindTexture(GL_TEXTURE_2D, texture1);
				cx = x;
			}

			gfx_get_char_size(fontmask, x, y, &size_x, &size_y);

			glBegin(GL_QUADS);
				glTexCoord2d(cx * 0.125, y * 0.125);
				glVertex2i(0,0);
				glTexCoord2d((cx + 1) * 0.125, y * 0.125);
				glVertex2i(32,0);
				glTexCoord2d((cx + 1) * 0.125, (y + 1) * 0.125);
				glVertex2i(32,32);
				glTexCoord2d(cx * 0.125, (y + 1) * 0.125);
				glVertex2i(0,32);
			glEnd();
			glTranslated(size_x,0,0);
			glEndList();
		}
	}

	SDL_FreeSurface(fontmask);

	return base;
}


void gfx_build_gl_circles (void)
{
	GLuint i;
	GLdouble cosine, sine;

	glNewList(FILLED_CIRCLE, GL_COMPILE);
	glBegin(GL_POLYGON);
    	for(i = 0; i < 100; i++) {
	    	cosine = cos(i * 2 * PI / 100.0);
		    sine = sin(i * 2 * PI/100.0);
		    glVertex2d(cosine, sine);
	    }
	glEnd();
	glEndList();

	glNewList(WIREFRAME_CIRCLE, GL_COMPILE);
	glBegin(GL_LINE_LOOP);
	    for(i = 0; i < 100; i++) {
		    cosine = cos(i * 2 * PI / 100.0);
		    sine = sin(i * 2 * PI/100.0);
		    glVertex2d(cosine, sine);
	    }
	glEnd();
	glEndList();
}


int gfx_graphics_startup (void)
{
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    window = SDL_CreateWindow("Elite GL",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              screen_w, screen_h,
                              ((windowed) ? SDL_WINDOW_RESIZABLE : SDL_WINDOW_FULLSCREEN) | SDL_WINDOW_OPENGL);
    if (window == 0) {
        fprintf( stderr, "Video mode set failed: %s\n",
             SDL_GetError( ) );
        exit( 1 );
    }
//    if( SDL_SetVideoMode( screen_w, screen_h, color_depth, SDL_WINDOW_OPENGL | ((windowed) ? SDL_RESIZABLE : SDL_FULLSCREEN) ) == 0 ) {

//  SDL_ShowCursor((windowed) ? SDL_ENABLE : SDL_DISABLE);

    gContext = SDL_GL_CreateContext(window);
    if( gContext == NULL )
    {
        fprintf( stderr, "Video mode set failed: %s\n",
             SDL_GetError( ) );
        exit( 1 );
    }
    
	glClearColor (0, 0, 0, 1.0);
	glClearDepth (1.0);
	glDepthFunc(GL_LESS);
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);

	glCullFace (GL_BACK);
	glFrontFace(GL_CW);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	scanner_image = gfx_load_bitmap(scanner_filename);

	if (!scanner_image)
	{
		printf("Error reading scanner bitmap file: %s.\n", scanner_filename);
		return 1;
	}

	texture[IMG_SCANNER_1] = gfx_load_texture("scanner.bmp",0,0,128);
	texture[IMG_SCANNER_2] = gfx_load_texture("scanner.bmp",128,0,128);
	texture[IMG_SCANNER_3] = gfx_load_texture("scanner.bmp",256,0,128);
	texture[IMG_SCANNER_4] = gfx_load_texture("scanner.bmp",384,0,128);

    texture[IMG_BLAKE]     = gfx_load_texture("blake.bmp",0,1,128);
	texture[IMG_ELITE_TXT] = gfx_load_texture("elitetx3.bmp",0,0,256);
	texture[IMG_GREEN_DOT] = gfx_load_texture("greendot.bmp",0,0,16);
	texture[IMG_MISSILE_GREEN] = gfx_load_texture("missgrn.bmp",0,0,16);
	texture[IMG_MISSILE_RED] = gfx_load_texture("missred.bmp",0,0,16);
	texture[IMG_MISSILE_YELLOW] = gfx_load_texture("missyell.bmp",0,0,16);

	texture[IMG_BIG_E] = gfx_load_texture("ecm.bmp",0,0,32);
	texture[IMG_RED_DOT] = gfx_load_texture("reddot.bmp",0,0,16);
	texture[IMG_BIG_S] = gfx_load_texture("safe.bmp",0,0,32);

	gfx_build_gl_circles();

	Elite_1_base = gfx_load_font("font2.bmp", "font2.bmp");
	Elite_2_base = gfx_load_font("font2.bmp", "font2.bmp");

	return 0;
}

void gfx_set_color (int index)
{
	glColor3ubv((GLubyte *)&(scanner_image->format->palette->colors[index]));
}

void apply_standard_transformation (void)
{
	glTranslatef(GFX_X_OFFSET, GFX_Y_OFFSET, 0.0);
}

void gfx_gl_print (int x, int y, char *string, int base, int col)
{
	glLoadIdentity();
	apply_standard_transformation();
	glTranslated(x,y - 1,0);

	glEnable (GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	gfx_set_color(col);

	glListBase(base-32);
	glCallLists(strlen(string),GL_BYTE,string);

	glDisable (GL_TEXTURE_2D);
	glDisable(GL_BLEND);

}

GLvoid gfx_gl_cen_print (int x, int y, char *string, int base, int col)
{
	GLfloat matrix[16];
	GLint buffer;

	glGetIntegerv(GL_DRAW_BUFFER, &buffer);
	glLoadIdentity();
	glDrawBuffer(GL_NONE);
	glListBase(base-32);
	glCallLists(strlen(string),GL_BYTE,string);

	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	glLoadIdentity();
	apply_standard_transformation();
	glTranslated(x - (matrix[12] / 2),y - 1,0);

	glEnable (GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	glDrawBuffer(buffer);
	gfx_set_color(col);
	
	glListBase(base-32);
	glCallLists(strlen(string),GL_BYTE,string);

	glDisable (GL_TEXTURE_2D);
	glDisable(GL_BLEND);

}


void gfx_graphics_shutdown (void)
{
	SDL_FreeSurface(scanner_image);
}


void gfx_update_screen (void)
{
	glFlush();
#ifdef DOUBLEBUFFER
	 SDL_GL_SwapWindow(window);
#endif
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthMask(GL_FALSE);
}


void gfx_plot_pixel (int x, int y, int col)
{
	glLoadIdentity();
	apply_standard_transformation();
	gfx_set_color(col);
	glBegin(GL_POINTS);
		glVertex2d(x,y);
	glEnd();
}

void gfx_draw_gl_circle (int cx, int cy, int radius, int circle_colour, int type)
{
	glLoadIdentity();
	apply_standard_transformation();
	glTranslated((double)cx, (double)cy, 0.0);
	glScaled((double)radius,(double)radius,1.0);
	gfx_set_color(circle_colour);
	glCallList(type);
}

void gfx_draw_filled_circle (int cx, int cy, int radius, int circle_colour)
{
	gfx_draw_gl_circle(cx, cy, radius, circle_colour, FILLED_CIRCLE);
}

void gfx_draw_circle (int cx, int cy, int radius, int circle_colour)
{
	gfx_draw_gl_circle(cx, cy, radius, circle_colour, WIREFRAME_CIRCLE);
}

void gfx_draw_line (int x1, int y1, int x2, int y2)
{
	gfx_draw_colour_line(x1, y1, x2, y2, GFX_COL_WHITE);
}

void gfx_draw_colour_line (int x1, int y1, int x2, int y2, int line_colour)
{
	glLoadIdentity();
	apply_standard_transformation();
	gfx_set_color(line_colour);
	glBegin(GL_LINES);
		glVertex2d(x1,y1);
		glVertex2d(x2,y2);
	glEnd();
}

void gfx_draw_triangle (int x1, int y1, int x2, int y2, int x3, int y3, int col)
{
	glLoadIdentity();
	apply_standard_transformation();
	gfx_set_color(col);
	glBegin(GL_TRIANGLES);
		glVertex2d(x1,y1);
		glVertex2d(x2,y2);
		glVertex2d(x3,y3);
	glEnd();
}

void gfx_display_text (int x, int y, char *txt)
{
	gfx_display_colour_text(x, y, txt, GFX_COL_WHITE);
}


void gfx_display_colour_text (int x, int y, char *txt, int col)
{
	gfx_gl_print(x, y, txt, Elite_1_base, col);
}

void gfx_display_centre_text (int y, char *str, int psize, int col)
{

	int txt_size;
	int txt_colour;

	if (psize == 140)
	{
		txt_size = Elite_2_base;
		txt_colour = GFX_COL_WHITE;
	}
	else
	{
		txt_size = Elite_1_base;
		txt_colour = col;
	}
	gfx_gl_cen_print((128 * GFX_SCALE), y, str, txt_size, txt_colour);
}

void gfx_draw_rectangle (int tx, int ty, int bx, int by, int col)
{
	glLoadIdentity();
	apply_standard_transformation();
	gfx_set_color(col);
	glBegin(GL_QUADS);
		glVertex2d(tx,ty);
		glVertex2d(bx,ty);
		glVertex2d(bx,by);
		glVertex2d(tx,by);
	glEnd();
}


void gfx_display_pretty_text (int tx, int ty, int bx, int by, char *txt)
{
	char strbuf[100];
	char *str;
	char *bptr;
	int len;
	int pos;
	int maxlen;
	
	maxlen = (bx - tx) / 8;

	str = txt;	
	len = strlen(txt);
	
	while (len > 0)
	{
		pos = maxlen;
		if (pos > len)
			pos = len;

		while ((str[pos] != ' ') && (str[pos] != ',') &&
			   (str[pos] != '.') && (str[pos] != '\0'))
		{
			pos--;
		}

		len = len - pos - 1;
	
		for (bptr = strbuf; pos >= 0; pos--)
			*bptr++ = *str++;

		*bptr = '\0';

		gfx_display_colour_text(tx, ty, strbuf, GFX_COL_WHITE);
		ty += (8 * GFX_SCALE);
	}
}


void gfx_draw_scanner (void)
{
	int i;
	int scanner_textures[4] = {IMG_SCANNER_1,
							   IMG_SCANNER_2,
							   IMG_SCANNER_3,
							   IMG_SCANNER_4};

	glLoadIdentity();
	apply_standard_transformation();
	glTranslatef(0.0, 385.0, 0.0);

	glColor4f(1.0,1.0,1.0,1.0);
	glEnable (GL_TEXTURE_2D);

	for (i = 0; i < 4; i++) {
		glBindTexture(GL_TEXTURE_2D, texture[scanner_textures[i]]);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); 
			glVertex2d(0.0,0.0);
			glTexCoord2f(1.0f, 0.0f); 
			glVertex2d(128.0,0.0);
			glTexCoord2f(1.0f, 1.0f); 
			glVertex2d(128.0,128.0);
			glTexCoord2f(0.0f, 1.0f); 
			glVertex2d(0.0,128.0);
		glEnd();
		glTranslatef(128.0,0.0,0.0);
	}

	glDisable (GL_TEXTURE_2D);

	gfx_draw_line (0, 1, 0, 384);
	gfx_draw_line (0, 1, 511, 1);
	gfx_draw_line (511, 1, 511, 384);
}


void gfx_set_clip_region (int tx, int ty, int bx, int by)
{
	clip_tx = tx;
	clip_ty = ty;
	clip_bx = bx;
	clip_by = by;

	glViewport((tx + GFX_X_OFFSET) * screen_w / 800, (600 - (by + GFX_Y_OFFSET)) * screen_h / 600, (bx - tx) * screen_w / 800, (by - ty) * screen_h / 600);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho(tx + GFX_X_OFFSET,bx + GFX_X_OFFSET,by + GFX_Y_OFFSET,ty + GFX_Y_OFFSET,0.0,1.0);
	glMatrixMode (GL_MODELVIEW);
}

void gfx_resize_window (int width, int height)
{
	screen_w = width;
	screen_h = height;
	gfx_set_clip_region(clip_tx, clip_ty, clip_bx, clip_by);
}

void gfx_draw_sprite (int sprite_no, int x, int y)
{
	GLfloat height, width;

	glLoadIdentity();
	apply_standard_transformation();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0);					
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glEnable (GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	glBindTexture(GL_TEXTURE_2D, texture[sprite_no]);
	glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	if (x == -1) x = ((256 * GFX_SCALE) - 192) / 2;
	glTranslatef((float)x, (float)y, 0.0f);
		
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); 
		glVertex2f(0.0,0.0);
		glTexCoord2f(1.0f, 0.0f); 
		glVertex2f(width,0.0);
		glTexCoord2f(1.0f, 1.0f); 
		glVertex2f(width,height);
		glTexCoord2f(0.0f, 1.0f); 
		glVertex2f(0.0,height);
	glEnd();

	glDisable (GL_TEXTURE_2D);
	glDisable(GL_BLEND);

}


void gfx_draw_view (void)
{
	int i;
	struct univ_object copy;

	gfx_set_clip_region (1, 2, 510, 383);

	for (i = 0; i < MAX_UNIV_OBJECTS; i++)
	{
		if ((universe[i].type == 0) ||
            (universe[i].distance < 170))
			continue;

		copy = universe[i];

		quaternion_slerp(&universe[i].oldquat, &universe[i].quat, timeslice, &copy.quat);
		quat_to_matrix(&copy.quat, copy.rotmat);

		copy.location.x = ((universe[i].location.x - universe[i].oldlocation.x) * timeslice) + universe[i].oldlocation.x;
		copy.location.y = ((universe[i].location.y - universe[i].oldlocation.y) * timeslice) + universe[i].oldlocation.y;
		copy.location.z = ((universe[i].location.z - universe[i].oldlocation.z) * timeslice) + universe[i].oldlocation.z;



		draw_ship(&copy);
//        draw_ship(&universe[i]);




		universe[i].flags &= ~FLG_FIRING;
	}

	gfx_set_clip_region (0, 0, 512, 512);
}


void gfx_advance_frame (void)
{
	int i;

	for (i = 0; i < MAX_UNIV_OBJECTS; i++)
	{
		universe[i].oldlocation = universe[i].location;
		universe[i].oldquat = universe[i].quat;
	}
}


void gfx_set_camera (int angle)
{
    double radians;

    radians = (double)angle * PI / 180.0;

	camera[0].x = cos(radians);
	camera[1].x = 0;
	camera[2].x = -sin(radians);
    camera[3].x = 0;
	camera[0].y = 0;
	camera[1].y = 1;
	camera[2].y = 0;
	camera[3].y = 0;
	camera[0].z = sin(radians);
	camera[1].z = 0;
	camera[2].z = cos(radians);
	camera[3].z = 0;
	camera[0].w = 0;
	camera[1].w = 0;
	camera[2].w = 0;
	camera[3].w = 1;
}


void get_object_matrix (struct univ_object *univ, Matrix4 matrix)
{
	matrix[0].x =  univ->rotmat[0].x;
	matrix[0].y =  univ->rotmat[0].y;
	matrix[0].z = -univ->rotmat[0].z;
	matrix[0].w =  0;
	matrix[1].x =  univ->rotmat[1].x;
	matrix[1].y =  univ->rotmat[1].y;
	matrix[1].z = -univ->rotmat[1].z;
	matrix[1].w =  0;
	matrix[2].x =  univ->rotmat[2].x;
	matrix[2].y =  univ->rotmat[2].y;
	matrix[2].z = -univ->rotmat[2].z;
	matrix[2].w =  0;
	matrix[3].x =  univ->location.x;
	matrix[3].y =  univ->location.y;
	matrix[3].z = -univ->location.z;
	matrix[3].w =  1;
}


void transform_ship_point (struct univ_object *univ, struct ship_point *point, Vector *result)
{
	Matrix4 trans_mat;

	get_object_matrix(univ, trans_mat);
	result->x = point->x;
	result->y = point->y;
	result->z = point->z;

	mult_vector4(result, trans_mat);
}


void draw_gl_face (struct ship_face *face_data, struct ship_point *points)
{
	int num_points;

	num_points = face_data->points;

	glBegin((num_points == 2) ? GL_LINES : GL_POLYGON);

	glVertex3iv((GLint *)&points[face_data->p1]);
	glVertex3iv((GLint *)&points[face_data->p2]);
	if (num_points > 2)
		glVertex3iv((GLint *)&points[face_data->p3]);
	if (num_points > 3)
		glVertex3iv((GLint *)&points[face_data->p4]);
	if (num_points > 4)
		glVertex3iv((GLint *)&points[face_data->p5]);
	if (num_points > 5)
		glVertex3iv((GLint *)&points[face_data->p6]);
	if (num_points > 6)
		glVertex3iv((GLint *)&points[face_data->p7]);
	if (num_points > 7)
		glVertex3iv((GLint *)&points[face_data->p8]);

	glEnd();
}


void gfx_draw_gl_ship (struct univ_object *univ)
{
	int i;
	struct ship_face *face_data;
	struct ship_solid *solid_data;
	struct ship_data *ship;
	int lasv;
	int col;

	Vector laser_point;
//	char outputx[100], outputy[100], outputz[100];
	Matrix4 object_matrix;

	ship = ship_list[univ->type];
	solid_data = &ship_solids[univ->type];
	face_data = solid_data->face_data;
/*
	sprintf(outputx, "X: %f", univ->location.x);
	sprintf(outputy, "Y: %f", univ->location.y);
	sprintf(outputz, "Z: %f", univ->location.z);

	gfx_display_text(16, 16, outputx);
	gfx_display_text(16, 32, outputy);
	gfx_display_text(16, 48, outputz);
*/

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glFrustum(-8, 8, -6, 6, 16, 65535);
	glMatrixMode(GL_MODELVIEW);

	glLoadMatrixd((GLdouble *)camera);

	get_object_matrix(univ, object_matrix);
	glMultMatrixd((GLdouble *)object_matrix);

	glEnable (GL_CULL_FACE);
	
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	if (wireframe)
	{
		for (i = 0; i < solid_data->num_faces; i++)
		{
			glColor3d(0,0,0);
			draw_gl_face(&face_data[i], ship->points);

			gfx_set_color(GFX_COL_WHITE);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			draw_gl_face(&face_data[i], ship->points);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		}

	} else
	{
		for (i = 0; i < solid_data->num_faces; i++)
		{
			gfx_set_color(face_data[i].colour);
			draw_gl_face(&face_data[i], ship->points);
		}

	}

    /*
	glColor3d(1.0,1.0,0.0);
	glBegin(GL_LINES);
	glVertex3d(0.0,0.0,0.0);
	glVertex3d(0.0,0.0,1000.0);
	glVertex3d(0.0,0.0,0.0);
	glVertex3d(0.0,1000.0,0.0);
	glVertex3d(0.0,0.0,0.0);
	glVertex3d(1000.0,0.0,0.0);
	glEnd();*/

	glDepthMask(GL_TRUE);
	glColor4d(0,0,0,0);
	glEnable(GL_BLEND);

//	glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);

	for (i = 0; i < solid_data->num_faces; i++)
	{
		draw_gl_face(&face_data[i], ship->points);
	}

//	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

	glDisable(GL_BLEND);
	glDepthMask(GL_FALSE);

	glDisable(GL_DEPTH_TEST);
	
	glDisable (GL_CULL_FACE);

	if (univ->flags & FLG_FIRING)
	{
		lasv = ship_list[univ->type]->front_laser;
		col = (univ->type == SHIP_VIPER && (!wireframe)) ? GFX_COL_CYAN : GFX_COL_WHITE; 

		glLoadIdentity();
		transform_ship_point(univ, &ship->points[lasv], &laser_point);
		mult_vector4(&laser_point, camera);

		gfx_set_color(col);

		glBegin(GL_LINES);
		    glVertex3dv((GLdouble *)&laser_point);
		    glVertex3i(laser_point.x > 0 ? -8: 8, rand255() / 16 - 10, -16);
		glEnd();
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}
