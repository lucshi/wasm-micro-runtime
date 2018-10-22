#include "bh_log.h"
#include "bh_platform.h"
#include "bh_platform_log.h"
#include "bh_memory.h"

#define __LITTLE_ENDIAN
#define __FDLIBM_STDC__

typedef union u32double_tag {
  int *pint;
  double *pdouble;
} U32DOUBLE;

static inline int *
pdouble2pint(double *pdouble)
{
  U32DOUBLE u;
  u.pdouble = pdouble;
  return u.pint;
}

#ifdef __LITTLE_ENDIAN
#define __HI(x) *(1+pdouble2pint(&x))
#define __LO(x) *(pdouble2pint(&x))
#define __HIp(x) *(1+(int*)x)
#define __LOp(x) *(int*)x
#else
#define __HI(x) *(int*)&x
#define __LO(x) *(1+(int*)&x)
#define __HIp(x) *(int*)x
#define __LOp(x) *(1+(int*)x)
#endif

#ifdef __FDLIBM_STDC__
static const double huge = 1.0e300;
#else
static double huge = 1.0e300;
#endif

#ifdef __STDC__
static const double
#else
static double
#endif
tiny  = 1.0e-300;

#ifdef __STDC__
static const double
#else
static double
#endif
one=  1.00000000000000000000e+00; /* 0x3FF00000, 0x00000000 */

#ifdef __STDC__
static const double
#else
static double
#endif
TWO52[2]={
  4.50359962737049600000e+15, /* 0x43300000, 0x00000000 */
 -4.50359962737049600000e+15, /* 0xC3300000, 0x00000000 */
};


static double fdlibm_sqrt(double x);
static double fdlibm_floor(double x);
static double fdlibm_ceil(double x);
static double fdlibm_fabs(double x);
static double fdlibm_rint(double x);
static int fdlibm_isnan(double x);

static double fdlibm_sqrt(double x)		/* wrapper sqrt */
{
	double z;
	int sign = (int)0x80000000;
	unsigned r,t1,s1,ix1,q1;
	int ix0,s0,q,m,t,i;
    if(x<0.0) {
		/*printf("you can't apply sqrt to a negative number");*/
	    return 0; /* sqrt(negative) */
	}
	else{
	ix0 = __HI(x);			/* high word of x */
	ix1 = __LO(x);		/* low word of x */

    /* take care of Inf and NaN */
	if((ix0&0x7ff00000)==0x7ff00000) {
	    return x*x+x;		/* sqrt(NaN)=NaN, sqrt(+inf)=+inf
					   sqrt(-inf)=sNaN */
	}
    /* take care of zero */
	if(ix0<=0) {
	    if(((ix0&(~sign))|ix1)==0) return x;/* sqrt(+-0) = +-0 */
	    else if(ix0<0)
		return (x-x)/(x-x);		/* sqrt(-ve) = sNaN */
	}
    /* normalize x */
	m = (ix0>>20);
	if(m==0) {				/* subnormal x */
	    while(ix0==0) {
		m -= 21;
		ix0 |= (ix1>>11); ix1 <<= 21;
	    }
	    for(i=0;(ix0&0x00100000)==0;i++) ix0<<=1;
	    m -= i-1;
	    ix0 |= (ix1>>(32-i));
	    ix1 <<= i;
	}
	m -= 1023;	/* unbias exponent */
	ix0 = (ix0&0x000fffff)|0x00100000;
	if(m&1){	/* odd m, double x to make it even */
	    ix0 += ix0 + ((ix1&sign)>>31);
	    ix1 += ix1;
	}
	m >>= 1;	/* m = [m/2] */

    /* generate sqrt(x) bit by bit */
	ix0 += ix0 + ((ix1&sign)>>31);
	ix1 += ix1;
	q = q1 = s0 = s1 = 0;	/* [q,q1] = sqrt(x) */
	r = 0x00200000;		/* r = moving bit from right to left */

	while(r!=0) {
	    t = s0+r;
	    if(t<=ix0) {
		s0   = t+r;
		ix0 -= t;
		q   += r;
	    }
	    ix0 += ix0 + ((ix1&sign)>>31);
	    ix1 += ix1;
	    r>>=1;
	}

	r = sign;
	while(r!=0) {
	    t1 = s1+r;
	    t  = s0;
	    if((t<ix0)||((t==ix0)&&(t1<=ix1))) {
		s1  = t1+r;
		if(((t1&sign)==sign)&&(s1&sign)==0) s0 += 1;
		ix0 -= t;
		if (ix1 < t1) ix0 -= 1;
		ix1 -= t1;
		q1  += r;
	    }
	    ix0 += ix0 + ((ix1&sign)>>31);
	    ix1 += ix1;
	    r>>=1;
	}

    /* use floating add to find out rounding direction */
	if((ix0|ix1)!=0) {
	    z = one-tiny; /* trigger inexact flag */
	    if (z>=one) {
	        z = one+tiny;
	        if (q1==(unsigned)0xffffffff) { q1=0; q += 1;}
		else if (z>one) {
		    if (q1==(unsigned)0xfffffffe) q+=1;
		    q1+=2;
		} else
	            q1 += (q1&1);
	    }
	}
	ix0 = (q>>1)+0x3fe00000;
	ix1 =  q1>>1;
	if ((q&1)==1) ix1 |= sign;
	ix0 += (m <<20);
	__HI(z) = ix0;
	__LO(z) = ix1;
	/*printf("fdlibm_sqrt iner: x:%.2f\n", z);*/

	if(x<0.0) {
		/*printf("you can't apply sqrt to a negative number");*/
	    return 0; /* sqrt(negative) */
	} else
	{
	    /*printf("fdlibm_sqrt: x:%.2f\n", z);*/

	}
}
return z;
}

static double fdlibm_floor(double x)
{
	int i0,i1,j0;
	unsigned i,j;
	i0 =  __HI(x);
	i1 =  __LO(x);
	j0 = ((i0>>20)&0x7ff)-0x3ff;
	if(j0<20) {
	    if(j0<0) { 	/* raise inexact if x != 0 */
		if(huge+x>0.0) {/* return 0*sign(x) if |x|<1 */
		    if(i0>=0) {i0=i1=0;}
		    else if(((i0&0x7fffffff)|i1)!=0)
			{ i0=0xbff00000;i1=0;}
		}
	    } else {
		i = (0x000fffff)>>j0;
		if(((i0&i)|i1)==0) return x; /* x is integral */
		if(huge+x>0.0) {	/* raise inexact flag */
		    if(i0<0) i0 += (0x00100000)>>j0;
		    i0 &= (~i); i1=0;
		}
	    }
	} else if (j0>51) {
	    if(j0==0x400) return x+x;	/* inf or NaN */
	    else return x;		/* x is integral */
	} else {
	    i = ((unsigned)(0xffffffff))>>(j0-20);
	    if((i1&i)==0) return x;	/* x is integral */
	    if(huge+x>0.0) { 		/* raise inexact flag */
		if(i0<0) {
		    if(j0==20) i0+=1;
		    else {
			j = i1+(1<<(52-j0));
			if(j<i1) i0 +=1 ; 	/* got a carry */
			i1=j;
		    }
		}
		i1 &= (~i);
	    }
	}
	__HI(x) = i0;
	__LO(x) = i1;
	return x;
}

static double fdlibm_ceil(double x)
{
	int i0,i1,j0;
	unsigned i,j;

	i0 =  __HI(x);
	i1 =  __LO(x);
	j0 = ((i0>>20)&0x7ff)-0x3ff;
	if(j0<20) {
	    if(j0<0) { 	/* raise inexact if x != 0 */
	if(huge+x>0.0) {
		    if(i0<0) {i0=0x80000000;i1=0;}
		    else if((i0|i1)!=0) { i0=0x3ff00000;i1=0;}
		}
	    } else {
		i = (0x000fffff)>>j0;
		if(((i0&i)|i1)==0) return x;
		if(huge+x>0.0) {
		    if(i0>0) i0 += (0x00100000)>>j0;
		    i0 &= (~i); i1=0;
		}
	    }
	} else if (j0>51) {
	    if(j0==0x400) return x+x;
	    else return x;
	} else {
	    i = ((unsigned)(0xffffffff))>>(j0-20);
	    if((i1&i)==0) return x;
	    if(huge+x>0.0) {
		if(i0>0) {
		    if(j0==20) i0+=1;
		    else {
			j = i1 + (1<<(52-j0));
			if(j<i1) i0+=1;
			i1 = j;
		    }
		}
		i1 &= (~i);
	    }
	}
	__HI(x) = i0;
	__LO(x) = i1;
	return x;
}

static double fdlibm_rint(double x)
{
	int i0,j0,sx;
	unsigned i,i1;
	double w,t;
	i0 =  __HI(x);
	sx = (i0>>31)&1;
	i1 =  __LO(x);
	j0 = ((i0>>20)&0x7ff)-0x3ff;
	if(j0<20) {
	    if(j0<0) {
		if(((i0&0x7fffffff)|i1)==0) return x;
		i1 |= (i0&0x0fffff);
		i0 &= 0xfffe0000;
		i0 |= ((i1|-i1)>>12)&0x80000;
		__HI(x)=i0;
	        w = TWO52[sx]+x;
	        t =  w-TWO52[sx];
	        i0 = __HI(t);
	        __HI(t) = (i0&0x7fffffff)|(sx<<31);
	        return t;
	    } else {
		i = (0x000fffff)>>j0;
		if(((i0&i)|i1)==0) return x; /* x is integral */
		i>>=1;
		if(((i0&i)|i1)!=0) {
		    if(j0==19) i1 = 0x40000000; else
		    i0 = (i0&(~i))|((0x20000)>>j0);
		}
	    }
	} else if (j0>51) {
	    if(j0==0x400) return x+x;	/* inf or NaN */
	    else return x;		/* x is integral */
	} else {
	    i = ((unsigned)(0xffffffff))>>(j0-20);
	    if((i1&i)==0) return x;	/* x is integral */
	    i>>=1;
	    if((i1&i)!=0) i1 = (i1&(~i))|((0x40000000)>>(j0-20));
	}
	__HI(x) = i0;
	__LO(x) = i1;
	w = TWO52[sx]+x;
	return w-TWO52[sx];
}

static int fdlibm_isnan(double x)
{
  int hx,lx;
  hx = (__HI(x)&0x7fffffff);
  lx = __LO(x);
  hx |= (unsigned)(lx|(-lx))>>31;
  hx = 0x7ff00000 - hx;
  return ((unsigned)(hx))>>31;
}

static double fdlibm_fabs(double x)
{
  __HI(x) &= 0x7fffffff;
  return x;
}

double sqrt(double x)
{
  return fdlibm_sqrt(x);
}

double floor(double x)
{
  return fdlibm_floor(x);
}

double ceil(double x)
{
  return fdlibm_ceil(x);
}

double fmin(double x, double y)
{
  return x < y ? x : y;
}

double fmax(double x, double y)
{
  return x > y ? x : y;
}

double rint(double x)
{
  return fdlibm_rint(x);
}

double fabs(double x)
{
  return fdlibm_fabs(x);
}

int isnan(double x)
{
  return fdlibm_isnan(x);
}

double trunc(double x)
{
  return (x > 0) ? fdlibm_floor(x) : fdlibm_ceil(x);
}

int signbit(double x)
{
  return ((__HI(x) & 0x80000000) >> 31);
}

