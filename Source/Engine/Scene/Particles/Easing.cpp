/* NekoEngine
 *
 * Easing.h
 * Author: Alexandru Naiman
 *
 * Easing functions
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2017, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _USE_MATH_DEFINES
#include <math.h>

#include <Scene/Particles/Easing.h>

double linearEase(double p)
{
	return p;
}

double quadraticEaseIn(double p)
{
	return p * p;
}

double quadraticEaseOut(double p)
{
	return -(p * (p - 2.0));
}

double quadraticEaseInOut(double p)
{
	if (p < 0.5)
		return 2.0 * p * p;
	else
		return (-2.0 * p * p) + (4.0 * p) - 1.0;
}

double cubicEaseIn(double p)
{
	return p * p * p;
}

double cubicEaseOut(double p)
{
	double f = (p - 1.0);
	return f * f * f + 1.0;
}

double cubicEaseInOut(double p)
{
	if (p < 0.5)
		return 4.0 * p * p * p;
	else
	{
		double f = ((2.0 * p) - 2.0);
		return 0.5 * f * f * f + 1.0;
	}
}

double quarticEaseIn(double p)
{
	return p * p * p * p;
}

double quarticEaseOut(double p)
{
	double f = (p - 1.0);
	return f * f * f * (1.0 - p) + 1.0;
}

double quarticEaseInOut(double p)
{
	if (p < 0.5)
		return 8.0 * p * p * p * p;
	else
	{
		double f = (p - 1.0);
		return -8.0 * f * f * f * f + 1.0;
	}
}

double quinticEaseIn(double p)
{
	return p * p * p * p * p;
}

double quinticEaseOut(double p)
{
	double f = (p - 1.0);
	return f * f * f * f * f + 1.0;
}

double quinticEaseInOut(double p)
{
	if (p < 0.5)
		return 16.0 * p * p * p * p * p;
	else
	{
		double f = ((2.0 * p) - 2.0);
		return  0.5 * f * f * f * f * f + 1.0;
	}
}

double sineEaseIn(double p)
{
	return sin((p - 1.0) * M_PI_2) + 1.0;
}

double sineEaseOut(double p)
{
	return sin(p * M_PI_2);
}

double sineEaseInOut(double p)
{
	return 0.5 * (1.0 - cos(p * M_PI));
}

double circularEaseIn(double p)
{
	return 1.0 - sqrt(1.0 - (p * p));
}

double circularEaseOut(double p)
{
	return sqrt((2.0 - p) * p);
}

double circularEaseInOut(double p)
{
	if (p < 0.5)
		return 0.5 * (1.0 - sqrt(1.0 - 4.0 * (p * p)));
	else
		return 0.5 * (sqrt(-((2.0 * p) - 3.0) * ((2.0 * p) - 1.0)) + 1.0);
}

double exponentialEaseIn(double p)
{
	return (p == 0.0) ? p : pow(2.0, 10.0 * (p - 1.0));
}

double exponentialEaseOut(double p)
{
	return (p == 1.0) ? p : 1.0 - pow(2.0, -10.0 * p);
}

double exponentialEaseInOut(double p)
{
	if (p == 0.0 || p == 1.0) return p;

	if (p < 0.5)
		return 0.5 * pow(2.0, (20.0 * p) - 10.0);
	else
		return -0.5 * pow(2.0, (-20.0 * p) + 10.0) + 1.0;
}

double elasticEaseIn(double p)
{
	return sin(13.0 * M_PI_2 * p) * pow(2.0, 10.0 * (p - 1.0));
}

double elasticEaseOut(double p)
{
	return sin(-13.0 * M_PI_2 * (p + 1)) * pow(2.0, -10.0 * p) + 1.0;
}

double elasticEaseInOut(double p)
{
	if (p < 0.5)
		return 0.5 * sin(13.0 * M_PI_2 * (2.0 * p)) * pow(2.0, 10.0 * ((2.0 * p) - 1.0));
	else
		return 0.5 * (sin(-13.0 * M_PI_2 * ((2.0 * p - 1.0) + 1.0)) * pow(2.0, -10.0 * (2.0 * p - 1.0)) + 2.0);
}

double backEaseIn(double p)
{
	return p * p * p - p * sin(p * M_PI);
}

double backEaseOut(double p)
{
	double f = (1.0 - p);
	return 1.0 - (f * f * f - f * sin(f * M_PI));
}

double backEaseInOut(double p)
{
	if(p < 0.5)
	{
		double f = 2.0 * p;
		return 0.5 * (f * f * f - f * sin(f * M_PI));
	}
	else
	{
		double f = (1.0 - (2.0 * p - 1.0));
		return 0.5 * (1.0 - (f * f * f - f * sin(f * M_PI))) + 0.5;
	}
}

double bounceEaseOut(double p)
{
	if (p < 0.36)
		return (121.0 * p * p) / 16.0;
	else if (p < 0.73)
		return (9.075 * p * p) - (9.9* p) + 3.4;
	else if (p < 0.9)
		return (12.06 * p * p) - (19.64 * p) + 8.89;
	else
		return (10.8 * p * p) - (20.52 * p) + 10.72;
}

double bounceEaseIn(double p)
{
	return 1.0 - bounceEaseOut(1.0 - p);
}

double bounceEaseInOut(double p)
{
	if (p < 0.5)
		return 0.5 * bounceEaseIn(p * 2.0);
	else
		return 0.5 * bounceEaseOut(p * 2.0 - 1.0) + 0.5;
}