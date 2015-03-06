
/*
 * externs.h - This file contains all of the external data definitions.
 * 
 * Copyright (C) 1990, Kory Hamzeh
 */

extern int      verbose;
extern char    *my_name;
extern char     input_file[];
extern char     output_file[];
extern int      nlights;
extern int      nobjects;
extern int      shadow;
extern int      reflect;
extern int      refract;
extern int	y_start;
extern int	y_inc;
extern int 	do_image_size;

extern VIEW_INFO view;
extern SURFACE *cur_surface;
extern BACKGROUND bkgnd;
extern LIGHT   *lights[];
extern OBJECT  *objects[];
extern OBJECT  *object_stack[];
extern OBJECT  *root;
extern INSTANCE *instances[];

extern int      num_instance;
extern int      stack_cnt;
extern int      sample_cnt;
extern int      n_rays;
extern int      n_intersects;
extern int      n_shadows;
extern int      n_shadinter;
extern int      n_reflect;
extern int      n_refract;

/* Global functions */
void Read_input_file(char *filename);
void Init_output_file(char *filename);
void Close_output_file(char *output_file);

void Build_bounding_slabs(void);
void Raytrace(void);

void Build_cone(CONE *cd);

void Build_sphere(SPHERE *sd);
void Build_hsphere(HSPHERE *sd);
void Build_poly(POLYGON *pd);
void Build_ring(RING *r);
void Build_quadric(QUADRIC *q);

void Write_pixel(COLOR *c);
void Flush_output_file(void);

int Intersect(RAY *ray, INTERSECT *inter);

void Push_object(OBJECT *obj);
OBJECT *Pop_object(void);

