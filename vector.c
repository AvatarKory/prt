
/*
 * vector.c - This module contains all of the vector math related functions.
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

#include "rt.h"

double 
VecNormalize(VECTOR *v)
{
	double          len;

	len = VecLen(*v);
	v->x /= len;
	v->y /= len;
	v->z /= len;

	return (len);
}
