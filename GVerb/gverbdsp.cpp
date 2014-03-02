

/*

        Copyright (C) 1999 Juhana Sadeharju
                       kouhia at nic.funet.fi

		Max/MSP port 2004 by Olaf Matthes, <olaf.matthes@gmx.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */

#include "gverbdsp.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>


ty_diffuser *diffuser_make(int size, float coeff)
{
	ty_diffuser *p;
	int i;

	p = (ty_diffuser *)malloc(sizeof(ty_diffuser));
	if(!p) return (NULL);
	p->size = size;
	p->coeff = coeff;
	p->idx = 0;
	p->buf = (float *)malloc(size*sizeof(float));
	if(!p->buf) return (NULL);
	for (i = 0; i < size; i++) p->buf[i] = 0.0;
	return(p);
}


void diffuser_flush(ty_diffuser *p)
{
	memset(p->buf, 0, p->size * sizeof(float));
}

ty_damper *damper_make(float damping)
{
	ty_damper *p;

	p = (ty_damper *)malloc(sizeof(ty_damper));
	if(!p) return (NULL);
	p->damping = damping;
	p->delay = 0.0;
	return(p);
}


void damper_flush(ty_damper *p)
{
	p->delay = 0.0f;
}

void fixeddelay_flush(ty_fixeddelay *p)
{
	memset(p->buf, 0, p->size * sizeof(float));
}

ty_fixeddelay *fixeddelay_make(int size)
{
	ty_fixeddelay *p;
	int i;

	p = (ty_fixeddelay *)malloc(sizeof(ty_fixeddelay));
	if(!p) return (NULL);
	p->size = size;
	p->idx = 0;
	p->buf = (float *)malloc(size*sizeof(float));
	if(!p->buf) return (NULL);
	for (i = 0; i < size; i++)
		p->buf[i] = 0.0;
	return(p);
}

int isprime(int n)
{
  unsigned int i;
  const unsigned int lim = (int)sqrtf((float)n);

  if (n == 2) return(1);
  if ((n & 1) == 0) return(0);
  for(i = 3; i <= lim; i += 2)
    if ((n % i) == 0) return(0);
  return(1);
}

int nearest_prime(int n, float rerror)
     /* relative error; new prime will be in range
      * [n-n*rerror, n+n*rerror];
      */
{
  int bound,k;

  if (isprime(n)) return(n);
  /* assume n is large enough and n*rerror enough smaller than n */
  bound = n*rerror;
  for(k = 1; k <= bound; k++) {
    if (isprime(n+k)) return(n+k);
    if (isprime(n-k)) return(n-k);
  }
  return(-1);
}

// Truncate float to int
int ff_trunc(float f) {
        f -= 0.5f;
        f += (3<<22);
        return *((int*)&f) - 0x4b400000;
}

// Round float to int (faster than f_trunc)
int ff_round(float f) {
        f += (3<<22);
        return *((int*)&f) - 0x4b400000;
}

