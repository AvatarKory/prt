/*
 * main.c - Main module for raytracer *
 * 
 * Copyright (C) 1990-2015, Kory Hamzeh
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License V3
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "rt.h"
#include "externs.h"

#define VERSION "1.01"

/*
 * Usage() - Print usage and exit.
 */

void Usage()
{
    fprintf(stderr, 
	    "Usage: %s: [-V] [-v] [-s] [-l] [-r] [-d] [-z] [-c sample_count]\n", my_name);
    fprintf(stderr, 
	    "           [-y y_start y_inc]  [input-file output-file]\n");
    exit(1);
}

int main(int argc, char *argv[])
{
    long            timest, timeend;
    int             i;

    time(&timest);

    my_name = argv[0];

    /*
     * check command line options
     */

    ++argv;
    --argc;
    while (argc > 0 && *argv[0] == '-')
    {
	if (!strcmp(*argv, "-v"))	/* verbose mode		 */
	{
	    verbose = 1;
	    --argc;
	    ++argv;
	}
	else if(!strcmp(argv[0], "-V"))
	{
	    printf("rt version %s\n", VERSION);
	    exit(0);
	}
	else if (!strcmp(*argv, "-s"))	/* shadows off		 */
	{
	    shadow = 0;
	    --argc;
	    ++argv;
	}
	else if (!strcmp(*argv, "-l"))	/* disable reflections	 */
	{
	    reflect = 0;
	    --argc;
	    ++argv;
	}
	else if (!strcmp(*argv, "-r"))	/* disable refractions	 */
	{
	    refract = 0;
	    ++argv;
	    --argc;
	}
	else if (!strcmp(*argv, "-d"))	/* disable sampling	 */
	{
	    sample_cnt = 0;
	    ++argv;
	    --argc;
	}
	else if (!strcmp(*argv, "-z"))	/* disable image size	 */
	{
	    do_image_size = 0;
	    ++argv;
	    --argc;
	}
	else if (!strcmp(*argv, "-c"))	/* sample count		 */
	{
	    ++argv;
	    sample_cnt = atoi(*argv);
	    ++argv;
	    argc -= 2;
	    if (sample_cnt < 0)
	    {
		fprintf(stderr, "%s: sample count must be > 0\n", my_name);
		exit(1);
	    }
	}
	else if (!strcmp(*argv, "-y"))	/* y start and inc */
	{
	    ++argv;
	    y_start = atoi(*argv);
	    ++argv;
	    y_inc = atoi(*argv);
	    ++argv;
	    argc -= 3;
	    if (y_start < 0 || y_inc < 1)
	    {
		fprintf(stderr, "%s: bad y_start and y_inc\n", my_name);
		exit(1);
	    }
	}
	else
	{
	    Usage();
	}
    }

    if (argc == 0)
    {
	strcpy(input_file, "STDIN");
	strcpy(output_file, "STDOUT");
    }
    else if (argc == 2)
    {
	strcpy(input_file, argv[0]);
	strcpy(output_file, argv[1]);
	strcat(output_file, ".ppm");
    }
    else
	Usage();

    /*
     * Read the input file. Will exit on error.
     */

    if (argc == 0)
	Read_input_file(NULL);
    else
	Read_input_file(input_file);

    /*
     * Check to make sure that there was at least one object and one
     * light source specified.
     */

    if (nlights == 0)
    {
	fprintf(stderr, "%s: no light sources were specified.\n", my_name);
	exit(1);
    }

    if (nobjects == 0)
    {
	fprintf(stderr, "%s: no objects were specified.\n", my_name);
	exit(1);
    }

    /*
     * Adjust the intensity of each light
     */

    for (i = 0; i < nlights; i++)
    {
	lights[i]->intensity = sqrt((double) nlights) / (double) nlights;
    }

    /*
     * Open the output file.
     */
	
    if (argc == 0)
	Init_output_file(NULL);
    else
	Init_output_file(output_file);

    /*
     * If verbose flag is on, print some info.
     */

    if (verbose)
    {
	printf("%s: version %s\n", my_name, VERSION);
	fprintf(stderr, "%s: input file = %s\n", my_name, input_file);
	fprintf(stderr, "%s: output file = %s\n", my_name, output_file);
	fprintf(stderr, "%s: %d objects were specified\n", my_name, nobjects);
	fprintf(stderr, "%s: %d lights were specified\n", my_name, nlights);
	fprintf(stderr, "%s: output image is %d x %d\n", my_name,
		view.x_res, view.y_res);
    }

    /*
     * Build the bounding box structures.
     */

    Build_bounding_slabs();

    if (verbose)
    {
	fprintf(stderr, "%s: %d objects after adding bounding volumes\n", my_name,
		nobjects);
    }

    /*
     * Raytrace the picture.
     */

    Raytrace();

    /*
     * Close output file and exit
     */

    if(argc == 0)
	Close_output_file(NULL);
    else
	Close_output_file(output_file);

    if(verbose)
	fprintf(stderr, "\n");

    /*
     * If verbose mode is on, then print some stats.
     * 
     */

    if (verbose)
    {
	time(&timeend);
	fprintf(stderr, "%s: total execution time: %ld:%02ld\n", my_name,
		(timeend - timest) / 60, (timeend - timest) % 60);
	fprintf(stderr, "%s: number of rays traced: %d\n", my_name, n_rays);
	fprintf(stderr, "%s: number of non-shadow intersections: %d\n", my_name,
		n_intersects);
	fprintf(stderr, "%s: number of shadow rays: %d\n", my_name, n_shadows);
	fprintf(stderr, "%s: number of shadow hits: %d\n", my_name, n_shadinter);
	fprintf(stderr, "%s: number of reflected rays: %d\n", my_name, n_reflect);
	fprintf(stderr, "%s: number of refracted rays: %d\n", my_name, n_refract);
    }

    exit(0);
}

