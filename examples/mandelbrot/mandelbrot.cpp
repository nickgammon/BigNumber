// this code is (C) 2015 by folkert@vanheusden.com
// it is released under AGPL v3.0
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "BigNumber.h"

#define NX 640
#define NY 480

int main(int argc, char *argv[])
{
	BigNumber::begin(16);

	BigNumber two(2.0), radius(16.0);
	BigNumber bbx1(-2.0), bbx2(2.0);
	BigNumber bby1(-2.0), bby2(2.0);
	BigNumber NXBN(NX);
	BigNumber NYBN(NY);
	BigNumber dx((bbx2 - bbx1) / NXBN);
	BigNumber dy((bby2 - bby1) / NYBN);

	for(int y=0; y<NY; y++)
	{
		for(int x=0; x<NX; x++)
		{
			BigNumber bnx(x);
			BigNumber bny(y);
			BigNumber cx(dx * bnx + bbx1);
			BigNumber cy(dy * bny + bby1);
			BigNumber wx(0), wy(0);

			int it = 80;

			BigNumber xsq(0), ysq(0);
			do
			{
				BigNumber z = xsq - ysq + cx;
				wy = wx * wy * two + cy;
				wx = z;

				xsq = wx * wx;
				ysq = wy * wy;

				it--;
			}
			while(it > 0 && xsq + ysq <= radius);

			printf("%d,%d: %d\n", x, y, it);
		}
	}

	return 0;
}
