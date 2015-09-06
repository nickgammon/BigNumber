// this code is (C) 2015 by folkert@vanheusden.com
// it is released under AGPL v3.0
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "BigNumber.h"

#ifdef FAST
#define NX 40
#define NY 30
#define MAX_IT 80
#else
#define NX 640
#define NY 480
#define MAX_IT 256
#endif

int main(int argc, char *argv[])
{
	BigNumber::begin(128);

	BigNumber two(2.0), radius(16.0);
	BigNumber bbx1(-2.0), bbx2(2.0);
	BigNumber bby1(-2.0), bby2(2.0);
	BigNumber NXBN(NX);
	BigNumber NYBN(NY);
	BigNumber dx((bbx2 - bbx1) / NXBN);
	BigNumber dy((bby2 - bby1) / NYBN);

	printf("P6 %d %d %d\n", NX, NY, MAX_IT - 1);

	BigNumber cy(bby1);
	for(int y=0; y<NY; y++)
	{
		fprintf(stderr, "%d\n", NY - y);

		BigNumber cx(bbx1);

		for(int x=0; x<NX; x++)
		{
			int it = MAX_IT;

			BigNumber wx, wy, xsq, ysq;
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

			char *wx_str = wx.toString();
			char *wy_str = wy.toString();

			printf("%c%c%c", it - atoi(wx_str), it, it - atoi(wy_str));

			free(wy_str);
			free(wx_str);

			cx += dx;
		}

		cy += dy;
	}

	return 0;
}
