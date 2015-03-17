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
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <libgen.h>
#include <getopt.h>

#include "rt.h"
#include "externs.h"

#define VERSION "1.02"

// Support command line options (passed to getopt())
const struct option long_options[] = {
    {"help",			no_argument,           0, 'h'},
    {"version",			no_argument,           0, 'V'},
    {"verbose",			no_argument,           0, 'v'},
    {"no-shadow",		no_argument,           0, 's'},
    {"no-reflection",		no_argument,           0, 'l'},
    {"no-refraction",		no_argument,           0, 'r'},
    {"no-sample",		no_argument,           0, 'd'},
    {"no-image-header",	no_argument,           0, 'z'},
    {"sample-count",		required_argument,  0, 'c'},
    {"start-y",			required_argument,  0, 'y'},
    {"inc-y",			required_argument,  0, 'i'},
    {"threads",			required_argument,  0, 't'},
    {0, 0, 0,  0}
};

const char *help_msg =
    "Usage: %s [options] [input-data-file output-image-file]\n"
    "\n"
    "Where options is 0 or more of:\n"
    "\n"
    "    -h, --help\n"
    "        Print this message and exit\n\n"
    "    -V, --version\n"
    "        Print the version number and exit\n\n"
    "    -v, --verbose\n"
    "        Turn on verbose output mode\n\n"
    "    -s, --no-shadow\n"
    "        Disable checking for shadow rays\n\n"
    "    -l, --no-reflection\n"
    "        Disable checking for reflected rays\n\n"
    "    -r, --no-refraction\n"
    "        Disable checking for refracted rays\n\n"
    "    -d, --no-sample\n"
    "        Disable pixel sampling\n\n"
    "    -z, --no-image-header\n"
    "        Don't write the PPM image header, just the pixels\n\n"
    "    -c count, --sample-count count\n"
    "        Set sample count per pixel to 'count'\n\n"
    "    -y starty --start-y starty \n"
    "        Set the starting image row to 'starty'. This option\n"
    "        is used when rt is invoked by prt.\n\n"
    "    -i inc-y, --start-y inc-y\n"
    "        Set the increment count per row (image row skip count)\n"
    "        to 'incy'. This option is used when rt is invoked by prt.\n\n"
    "    -t thread-count, --threads thread-count\n"
    "        Set the number of ray tracer threads to 'thread_count' for this\n"
    "        process. Not fully implemented or debugged!\n"
    "\n";

/*
 * Usage() - Print usage and exit.
 */

void Usage()
{
    fprintf(stderr, help_msg, my_name);
    exit(1);
}

void bad_opt_value(const char *opt_name)
{
    fprintf(stderr, "%s: bad %s value: %s\n\n", my_name, opt_name, optarg);
    Usage();
}

int main(int argc, char *argv[])
{
    long		timest, timeend;
    int		i;
    int		c;

    time(&timest);

    my_name = basename( argv[ 0 ] );	

    /*
     * check command line options
     */

    while (1)
    {
	int option_index = 0;


	c = getopt_long(argc, argv, "hvVslrdzc:y:i:t:",
			long_options, &option_index);

	if (c == -1)
	{
	    break;
	}

	switch( c )
	{
	case 'h':
	    Usage();
	    break;

	case 'v':
	    verbose = 1;
	    break;

	case 'V':
	    printf("rt Version %s, Copyright (C) 1990-2015, Kory Hamzeh\n", VERSION);
	    exit(0);

	case 's':
	    shadow = 0;
	    break;

	case 'l':
	    reflect = 0;
	    break;

	case 'r':
	    refract = 0;
	    break;

	case 'd':
	    sample_cnt = 0;
	    break;

	case 'z':
	    do_image_size = 0;
	    break;

	case 't':
	    num_threads = atol( optarg );
	    if( num_threads < 1 || num_threads > 256)
	    {
		bad_opt_value("num-threads");
	    }
	    break;

	case 'c':
	    sample_cnt = atol( optarg );
	    if( sample_cnt < 1 )
	    {
		bad_opt_value("sample count");
	    }
	    break;

	case 'y':
	    y_start = atoi( optarg );
	    if (y_start < 0)
	    {
		bad_opt_value("start y");
	    }
	    break;

	case 'i':
	    y_inc = atoi( optarg );
	    if (y_inc < 1)
	    {
		bad_opt_value("inc-y");
	    }
	    break;

	default:
	    Usage();
	}
    }

    if (argc == optind)
    {
	use_stdio = 1;
	strcpy(input_file, "STDIN");
	strcpy(output_file, "STDOUT");
    }
    else if ((argc - optind) == 2)
    {
	use_stdio = 0;
	strcpy(input_file, argv[optind]);
	strcpy(output_file, argv[optind + 1]);
	strcat(output_file, ".ppm");
    }
    else
    {
	// At this point, there must have been either 0 or 2 CLI args. Anything
	// else is an error
	Usage();
    }

    /*
     * Read the input file. Will exit on error.
     */

    if (use_stdio)
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
	
    if (use_stdio)
	Init_output_file(NULL);
    else
	Init_output_file(output_file);

    /*
     * If verbose flag is on, print some info.
     */

    if (verbose)
    {
	fprintf(stderr, "%s: version %s\n", my_name, VERSION);
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

    if(output_file[0] == 0)
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

