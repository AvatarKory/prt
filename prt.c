/*
 * prt - Paralel Ray Tracer
 *
 * Copyright (C) 1990-2015, Kory Hamzeh.
 *
 * This program acts as a front-end to 'rt'. It takes the user given list
 * of hostnames, and using rsh, it spawns off copys of rt running on different
 * machines. It will gather up the image fragments that each sub-process sends
 * it, and builds one final image. The usage syntax is as follows:
 *
 * 	prt input-file output-file host [host ... ]
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

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define VERSION "1.1"

#define MAX_HOSTS	64

typedef struct rt_info {
    char	hostname[32];
    int	pid;
    int	cur_bytes;
    int	tot_bytes;
    int	offset;
    int	bld_offset;
    long	time_st;
    long	time_en;
    int	out_pipe[2];
    int	err_pipe[2];
} RT_INFO;

RT_INFO rt_tab[MAX_HOSTS];

int num_hosts = 0;
int tot_bytes = 0;
int cur_bytes = 0;
int percent_done = 0;
int verbose = 0;
char in_file[64];
char out_file[64];
FILE *in_fp;
FILE *out_fp;
int image_x;
int image_y;
char *image;
char *my_name;
fd_set work_fds;
fd_set orig_fds;
long time_st;
long time_en;
char *rt_args[32];
char *rshell = "ssh";
/*
** Dead_baby()
**
** Death of a child signal is vectored here.
*/

void Dead_baby(int x)
{
    int status;

    wait(&status);
    signal(SIGCHLD, Dead_baby);
}


/*
** Usage()
**
** Print usage message.
*/

void Usage(void)
{
    fprintf(stderr, "Usage: %s [-v] [-V] input-file output-file host [host ..]\n",
	    my_name);
    exit(1);
}


/*
** Get_image_size()
**
** Scan the object database looking for the resolution key word to figure
** out how big this image will be.
*/

void Get_image_size(void)
{
    int found = 0;
    int line = 1;
    char buf[512];

    if((in_fp = fopen(in_file, "r")) == NULL)
    {
	fprintf(stderr, "%s: unable to open input file '%s'\n", 
		my_name, in_file);
	exit(1);
    }

    /*
    ** Scan the file looking for 'resolution xxx yyy'.
    */

    while(fgets(buf, sizeof(buf), in_fp))
    {
	if(!strncmp(buf, "resolution", 10))
	{
	    if(sscanf(buf + 10, "%d %d", &image_x, &image_y) != 2)
	    {
		fprintf(stderr, "%s: invalid resolution specified in input file on line %d.\n", 
			my_name, line);
		fclose(in_fp);
		exit(1);
	    }

	    found = 1;
	    break;
	}

	++line;
    }

    fclose(in_fp);
    if(!found)
    {
	fprintf(stderr, "%s: image resolution not found in file '%s'.\n",
		my_name, in_file);
	exit(1);
    }
}

/*
** Kill_tracers()
**
*/

void Kill_tracers(int count)
{
    int h;

    for(h = 0; h < count; h++)
	kill(rt_tab[h].pid, 9); 	/* a sure kill */
}

/*
** Start_tracers()
**
** This routine is a bit hairy. It creates the input, output, and error
** pipe, and then rsh's rt.
*/

void Start_tracers(void)
{
    int rc, h, in_fd, i, arg_ndx;
    char starty[32], incy[32], *prog;

    /* iterate through the list of all of the hosts and start
       an rt task */
    for(h = 0; h < num_hosts; h++)
    {
	/* First, create the pipe. */
	rc = pipe(rt_tab[h].out_pipe);
	rc += pipe(rt_tab[h].err_pipe);

	if(rc != 0)
	{
	    fprintf(stderr, "%s: pipe() failed.\n", my_name);
	    Kill_tracers(h);
	}

	if(verbose)
	    fprintf(stderr, "%s: starting rt on host %s.\n", my_name,
		    rt_tab[h].hostname);
	/* fork */
	if((rt_tab[h].pid = fork()) < 0)
	{
	    fprintf(stderr, "%s: fork() failed.\n", my_name);
	    Kill_tracers(h);
	}

	if(rt_tab[h].pid > 0)		/* parent */
	{
	    /* Close the write end of the pipes. */
	    close(rt_tab[h].out_pipe[1]);
	    close(rt_tab[h].err_pipe[1]);
	    /* Set for no delay on read */
	    fcntl(rt_tab[h].out_pipe[0], F_SETFL, O_NDELAY);
	    fcntl(rt_tab[h].err_pipe[0], F_SETFL, O_NDELAY);
	    /* set the start time */
	    time(&rt_tab[h].time_st);
	}
	else
	{
	    sprintf(starty, "%d", h);
	    sprintf(incy, "%d", num_hosts);

	    /* Close the read end of the pipe */
	    close(rt_tab[h].out_pipe[0]);
	    close(rt_tab[h].err_pipe[0]);

	    in_fd = open(in_file, O_RDONLY);
	    close(0);		/* the data file */
	    dup(in_fd);
	    close(1);
	    dup(rt_tab[h].out_pipe[1]);

	    close(2);
	    dup(rt_tab[h].err_pipe[1]);

	    for(i = 3; i < 100; i++)
		close(i);

	    rt_args[1] = rt_tab[h].hostname;
	    rt_args[5] = starty;
	    rt_args[6] = incy;

	    /* If we are running an instance of rt locally (i.e. hostname == 'localhost"),
	       we don't need to stat rsh. Just exec rt directly */

	    if(!strcmp(rt_tab[h].hostname, "localhost"))
	    {
		prog = "rt";
		arg_ndx = 2;
	    }
	    else
	    {
		prog = rshell;
		arg_ndx = 0;
	    }

	    execvp(prog, &rt_args[ arg_ndx ]);

	    fprintf(stderr, "%s: starting %s on host %s failed.\n", my_name,
		    prog, rt_tab[h].hostname); 
	    exit(1);
	}
    }
}

/*
** Build_fd_set() - Build an fd_set for select()
*/

void Build_fd_set(void)
{
    int h;

    FD_ZERO(&orig_fds);

    for(h = 0; h < num_hosts; h++)
    {
	if(rt_tab[h].pid == 0)
	    continue;

	FD_SET(rt_tab[h].out_pipe[0], &orig_fds);
	FD_SET(rt_tab[h].err_pipe[0], &orig_fds); 
    }
}

/*
** Rt_done()
**
** A rt sub-process fisnished, let the user know.
*/

void Rt_done(int h)
{
    int min, sec;

    time(&rt_tab[h].time_en);
    min = (rt_tab[h].time_en - rt_tab[h].time_st) / 60;
    sec = (rt_tab[h].time_en - rt_tab[h].time_st) % 60;

    fprintf(stderr, "\n%s: rt on host %s is done. Execution time: %02d:%02d.\n", 
	    my_name, rt_tab[h].hostname, min, sec);
}

/*
** Get_image_frag(host)
**
** Get the image fragment which just came form one of the tracers. Just store
** it in the buffer for now, and we'll sort it out later.
*/

void Get_image_frag(int h)
{
    int fd, count;
    char *p;
    extern int errno;

    if(rt_tab[h].pid == 0)
	return;

    fd = rt_tab[h].out_pipe[0];
    p = image + rt_tab[h].offset + rt_tab[h].cur_bytes;

    while((count = read(fd, p, image_x * 3)) > 0)
    {
	p += count;
	rt_tab[h].cur_bytes += count;
	cur_bytes += count;
	if(rt_tab[h].cur_bytes == rt_tab[h].tot_bytes)
	{
	    if(verbose)
		Rt_done(h);
	    rt_tab[h].pid = 0;
	    return ;
	}
    }

    if(count == 0 && errno != EWOULDBLOCK && errno != EINTR)
    {
	rt_tab[h].pid = 0;	/* this guy is done */
	if(verbose)
	    fprintf(stderr, "\n%s: rt on host %s is done.\n", my_name,
		    rt_tab[h].hostname);
    }

}

/*
** Tracer_error_report()
**
** A rt sub-process sent us a message through its stderr pipe. Display the
** message and kill them all.
*/

void Tracer_error_report(int h)
{
    char ebuf[512];
    int count;

    if(rt_tab[h].pid == 0)
	return ;

    count = read(rt_tab[h].err_pipe[0], ebuf, sizeof(ebuf));
    if(count > 0)
    {
	fprintf(stderr, "\n%s: Error from rt on host %s\007\n", my_name,
		rt_tab[h].hostname);
	ebuf[count] = 0;
	fprintf(stderr, "%s", ebuf);
	Kill_tracers(num_hosts);
	exit(0);
    }
}


/*
** Write_image()
**
** Take the image fragments that came back from all of the tracers and
** write one big image.
*/

void Write_image(void)
{
    int h, rs;
    char *p;

    if(verbose)
	fprintf(stderr, "\n%s: writing image ... ", my_name);

    cur_bytes = 0;
    rs = image_x * 3;
    h = 0;

    while(cur_bytes < tot_bytes)
    {
	p = image + rt_tab[h].bld_offset;
	fwrite(p, rs, 1, out_fp);
	rt_tab[h].bld_offset += rs;
	cur_bytes += rs;

	h++;
	if(h == num_hosts)
	    h = 0;
    }

    if(verbose)
	fprintf(stderr, "\n");
}

/*
** Gather_image()
**
** This routine gathers the bits of images flying back from the various 
** sub-processes, and build one big final image.
*/

void Gather_image(void)
{
    int h, width, rc;
    extern int errno;

    /*
    ** Build fd list for select.
    */

    Build_fd_set();

    /*
    ** Write the image header.
    */

    fprintf(out_fp, "P6\n%d %d\n255\n", image_x, image_y);
    tot_bytes = image_x * image_y * 3;
    cur_bytes = 0;
    percent_done = 0;
    width = MAX_HOSTS * 2;

    /*
    ** Main loop.
    */

    if(verbose)
	fprintf(stderr, "%s: %d%% done         ", my_name, percent_done);

    while(cur_bytes < tot_bytes)
    {
	Build_fd_set();
	work_fds = orig_fds;
	rc = select(width, &work_fds, NULL, NULL, NULL);
	if(rc < 0)
	    continue;

	for(h = 0; h < num_hosts; h++)
	{
	    if(rt_tab[h].pid == 0)
		continue;

	    if(FD_ISSET(rt_tab[h].out_pipe[0], &work_fds))
	    {
		Get_image_frag(h);
		FD_CLR(rt_tab[h].out_pipe[0], &work_fds);
	    }

	    if(FD_ISSET(rt_tab[h].err_pipe[0], &work_fds))
	    {
		Tracer_error_report(h);
		FD_CLR(rt_tab[h].err_pipe[0], &work_fds);
	    }

	}

	if(verbose)
	{
	    if(((cur_bytes * 100) / tot_bytes) != percent_done)
	    {
		percent_done = (cur_bytes * 100) / tot_bytes;
		fprintf(stderr, "\r%s: %d%% done        ", my_name, 
			percent_done);
	    }
	}
    }

    /* Write out the image */
    Write_image();

}


/*
** Main startup code.
*/

int main(int argc, char *argv[])
{
    int i, pix1, pix2;

    time(&time_st);

    my_name = argv[0];
    ++argv;
    --argc;

    if((argc == 1) && !strcmp(argv[0], "-V"))
    {
	printf("prt version %s\n", VERSION);
	exit(0);
    }

    if(argc < 3)
	Usage();

    /*
    ** Check for the verbose option.
    */

    while(argc > 3)
    {
	if(!strcmp(argv[0], "-v"))
	{
	    verbose = 1;
	    ++argv;
	    --argc;
	}
	else if(!strcmp(argv[0], "--shell"))
	{
	    rshell = argv[1];
	    argv += 2;
	    argc -= 2;
	}
	else
	{
	    break;
	}
    }

    if(argc < 2)
	Usage();

    rt_args[0] = rshell;
    rt_args[2] = "rt";
    rt_args[3] = "-z";
    rt_args[4] = "-y";

    i = 7;
    rt_args[i] = NULL;
    while(argc > 2 && *argv[0] == '-')
    {
	rt_args[i] = *argv;
	rt_args[i + 1] = NULL;
	i++;
	++argv;
	--argc;
    }

    strcpy(in_file, argv[0]);
    ++argv;
    --argc;

    strcpy(out_file, argv[0]);
    strcat(out_file, ".ppm");
    ++argv;
    --argc;

    signal(SIGCHLD, Dead_baby);

    /*
    ** Get the list of hosts from the command line.
    */

    while(argc > 0 && num_hosts < MAX_HOSTS)
    {
	strcpy(rt_tab[num_hosts].hostname, argv[0]);

	rt_tab[num_hosts].cur_bytes = 0;
	rt_tab[num_hosts].tot_bytes = 0;

	++argv;
	--argc;
	++num_hosts;
    }

    if(num_hosts == 0)
    {
	fprintf(stderr, "%s: no host were specified.\n", my_name);
	exit(0);
    }

    /*
    ** Check to see if too many hosts were specified.
    */

    if(num_hosts == MAX_HOSTS)
    {
	fprintf(stderr, "%s: Too many hosts. Max is %d.\n", 
		my_name, MAX_HOSTS);
	exit(1);
    }

    /*
    ** Check the image size.
    */

    Get_image_size();

    if(verbose)
    {
	fprintf(stderr, "%s: image size is %d x %d\n", 
		my_name, image_x, image_y);
    }

    /*
    ** Allocate an image buffer.
    */

    if((image = (char *) malloc(image_x * image_y * 3)) == NULL)
    {
	fprintf(stderr, "%s: malloc of %d failed.\n", 
		my_name,
		image_x *image_y);
	exit(1);
    }

    /*
    ** Open the output file.
    */

    if((out_fp = fopen(out_file, "w")) == NULL)
    {
	fprintf(stderr, "%s: unable to create file '%s'.\n", my_name, out_file);
	exit(1);
    }

    /*
    ** Calculate the total number of pixels that each sub-process
    ** must render.
    */

    pix1 = image_y / num_hosts;
    pix2 = image_y % num_hosts;

    for(i = 0; i < num_hosts; i++)
	rt_tab[i].tot_bytes = pix1 * (image_x * 3);

    for(i = 0; i < pix2; i++)
	rt_tab[i].tot_bytes += image_x * 3;

    /*
    ** Calculate the offset into the image buffer which each host
    ** will use.
    */

    for(i = 0, pix1 = 0; i < num_hosts; i++, pix1 += rt_tab[i].tot_bytes)
	rt_tab[i].offset  = rt_tab[i].bld_offset = pix1;

    /*
    ** Start the sub-processes.
    */

    Start_tracers();

    /*
    ** Gather bits of image and build one final big image.
    */

    Gather_image();

    /*
    ** Exit and go home.
    */

    time(&time_en);

    if(verbose)
	fprintf(stderr, "%s: Total execution time: %02ld:%02ld.\n",
		my_name, (time_en - time_st) / 60, (time_en - time_st) % 60);

    fclose(out_fp);
    exit(0);
}

