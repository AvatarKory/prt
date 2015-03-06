
/*
 * output.c - This files contains all of the output image related functions.
 * 
 * Copyright (C) 1990-2015, Kory Hamzeh
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "rt.h"
#include "externs.h"

#define CLAMPING	FALSE

FILE           *out_fp;

/*
 * Init_output_file()
 * 
 * Create and initialize the output image file.
 */

void Init_output_file(char *filename)
{
    if (filename)
    {
	if ((out_fp = fopen(output_file, "w")) == NULL)
	{
	    fprintf(stderr, "%s: unable to create output file '%s'\n",
		    my_name, output_file);
	    exit(1);
	}
    }
    else
    {
	out_fp = stdout;
    }

    if(do_image_size)
    {
	/* Write the PPM image file header */
	fprintf(out_fp, "P6\n%d %d\n255\n", view.x_res, view.y_res);
    }
}

/*
 * Close_output_file()
 * 
 * Do just like it sez.
 */

void Close_output_file(char *filename)
{
    if (filename)
	fclose(out_fp);
}

/*
 * Write_pixel()
 * 
 * Write the given RGB color pixel to the output file. Apply clamping if
 * necessary.
 */

void Write_pixel(COLOR *c)
{
    unsigned char   rr, gg, bb;
    double          mc;

#if CLAMPING
    if (c->r > 1.0)
	c->r = 1.0;
    if (c->g > 1.0)
	c->g = 1.0;
    if (c->b == 1.0)
	c->b = 1.0;
#endif

    mc = c->r;
    if (c->g > mc)
	mc = c->g;
    if (c->b > mc)
	mc = c->b;

    if (mc > 1)
    {
	c->r /= mc;
	c->g /= mc;
	c->b /= mc;
    }

    rr = 255.0 * c->r;
    gg = 255.0 * c->g;
    bb = 255.0 * c->b;

    putc(rr, out_fp);
    putc(gg, out_fp);
    putc(bb, out_fp);
}

void Flush_output_file()
{
    return ;
}

