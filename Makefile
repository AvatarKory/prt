#
# Makefile for rt. A bitchin' Raytracer
#
# Copyright (C) 1990-2015, Kory Hamzeh
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
CFLAGS= -g -O -Wall -Werror
YFLAGS=-d
LDFLAGS=-g
LIBS=-lm
CC=gcc
CPP=g++

#
# .h files go here
#
HFILES= \
	rt.h \
	externs.h

#
# .c files here
#
CFILES= \
	main.c \
	data.c \
	input.c \
	output.c \
	trace.c \
	sphere.c \
	hsphere.c \
	poly.c \
	cone.c \
	ring.c \
	quadric.c \
	intersect.c \
	shade.c \
	bound.c \
	stack.c \
	vector.c

#
# .o files here
#
OFILES = \
	main.o \
	data.o \
	input.o \
	output.o \
	trace.o \
	sphere.o \
	hsphere.o \
	poly.o \
	cone.o \
	ring.o \
	quadric.o \
	intersect.o \
	shade.o \
	stack.o \
	bound.o \
	vector.o

all: rt prt nff2prt
	chmod +x nff2prt

rt: $(OFILES)
	$(CC) $(LDFLAGS) -o rt $(OFILES) $(LIBS)

prt: prt.c
	$(CC) $(CFLAGS) -o prt prt.c

install: all
	cp rt /usr/local/bin
	cp prt /usr/local/bin
	cp nff2prt /usr/local/bin
	chmod +x /usr/local/bin/nff2prt

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f prt rt core *.o

#
# AUTOMATICALLY UPDATED BY MAKEDEPEND
bound.o: bound.c
bound.o: rt.h
bound.o: externs.h
cone.o: cone.c
cone.o: rt.h
cone.o: externs.h
data.o: data.c
data.o: rt.h
hsphere.o: hsphere.c
hsphere.o: rt.h
hsphere.o: externs.h
input.o: input.c
input.o: rt.h
input.o: externs.h
intersect.o: intersect.c
intersect.o: rt.h
intersect.o: externs.h
main.o: main.c
main.o: rt.h
main.o: externs.h
mtile.o: mtile.c
noise.o: noise.c
noise.o: rt.h
noise.o: externs.h
output.o: output.c
output.o: rt.h
output.o: externs.h
poly.o: poly.c
poly.o: rt.h
poly.o: externs.h
quadric.o: quadric.c
quadric.o: rt.h
quadric.o: externs.h
ring.o: ring.c
ring.o: rt.h
ring.o: externs.h
shade.o: shade.c
shade.o: rt.h
shade.o: externs.h
sphere.o: sphere.c
sphere.o: rt.h
sphere.o: externs.h
stack.o: stack.c
stack.o: rt.h
stack.o: externs.h
trace.o: trace.c
trace.o: rt.h
trace.o: externs.h
vector.o: vector.c
vector.o: rt.h
