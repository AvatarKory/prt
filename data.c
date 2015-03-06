
/*
 * data.c - Contains all of the data declarations.
 */

#include <math.h>
#include "rt.h"

int             verbose = 0;
char           *my_name;
char            input_file[64];
char            output_file[64];
int             nlights = 0;
int             nobjects = 0;
int             shadow = 1;
int             reflect = 1;
int             refract = 1;
int		y_start = 0;
int		y_inc = 1;
int		do_image_size = 1;

VIEW_INFO       view;
SURFACE        *cur_surface;
BACKGROUND      bkgnd;
LIGHT          *lights[MAX_LIGHTS];
OBJECT         *objects[MAX_PRIMS];
OBJECT         *object_stack[STACK_SIZE];
OBJECT         *root;
INSTANCE       *instances[MAX_INSTANCE];

int             num_instance = 0;
int             stack_cnt = 0;
int             sample_cnt = 1;

int             n_rays = 0;
int             n_intersects = 0;
int             n_shadows = 0;
int             n_shadinter = 0;
int             n_reflect = 0;
int             n_refract = 0;

