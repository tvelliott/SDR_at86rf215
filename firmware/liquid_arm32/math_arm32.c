/*
//
//t.elliott converted from original liquid-dsp code to something that would be usable on the STM32H7 series
//
 * Copyright (c) 2007 - 2015 Joseph Gaeddert
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

//
// Useful mathematical formulae
//
// References:
//  [Helstrom:1960] Helstrom, C. W., Statistical Theory of Signal
//      Detection. New York: Pergamon Press, 1960
//  [Helstrom:1992] Helstrom, C. W. "Computing the Generalized Marcum Q-
//      Function," IEEE Transactions on Information Theory, vol. 38, no. 4,
//      July, 1992.
//  [Proakis:2001] Proakis, J. Digital Communications. New York:
//      McGraw-Hill, 2001

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "globals.h"
#include "math_arm32.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                    infty
// Q(z) = 1/sqrt(2 pi) int { exp(-u^2/2) du }
//                      z
//
// Q(z) = (1/2)*(1 - erf(z/sqrt(2)))
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float liquid_Qf(float _z)
{
    return 0.5f * (1.0f - erff(_z*M_SQRT1_2));
}

#define NUM_MARCUMQ_ITERATIONS 16
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Marcum Q-function
// TODO : check this computation
// [Helstrom:1960], [Proakis:2001], [Helstrom:1992]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float liquid_MarcumQf(int _M,
                      float _alpha,
                      float _beta)
{
#if 0
    // expand as:
    //                               infty
    // Q_M(a,b) = exp(-(a^2+b^2)/2) * sum { (a/b)^k I_k(a*b) }
    //                               k=1-M
    return 0.0f
#else

    // use approximation [Helstrom:1992] (Eq. 25)
    // Q_M(a,b) ~ erfc(x),
    //   x = (b-a-M)/sigma^2,
    //   sigma = M + 2a

    // compute sigma
    float sigma = (float)(_M) + 2.0f*_alpha;

    // compute x
    float x = (_beta - _alpha - (float)_M) / (sigma*sigma);

    // return erfc(x)
    return erfcf(x);
#endif
}

#define NUM_MARCUMQ1_ITERATIONS 64
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Marcum Q-function (M=1)
// TODO : check this computation
// [Helstrom:1960], [Proakis:2001]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float liquid_MarcumQ1f(float _alpha,
                       float _beta)
{
#if 1
    // expand as:                    infty
    // Q_1(a,b) = exp(-(a^2+b^2)/2) * sum { (a/b)^k I_k(a*b) }
    //                                k=0

    float t0 = expf( -0.5f*(_alpha*_alpha + _beta*_beta) );
    float t1 = 1.0f;

    float a_div_b = _alpha / _beta;
    float a_mul_b = _alpha * _beta;

    float y = 0.0f;
    unsigned int k;
    for (k=0; k<NUM_MARCUMQ1_ITERATIONS; k++) {
        // accumulate y
        y += t1 * liquid_besselif((float)k, a_mul_b);

        // update t1
        t1 *= a_div_b;
    }

    return t0 * y;
#else
    
    // call generalized Marcum-Q function with M=1
    return liquid_MarcumQf(1, _alpha, _beta);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// compute sinc(x) = sin(pi*x) / (pi*x)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float sincf(float _x) {
    // _x ~ 0 approximation
    //if (fabsf(_x) < 0.01f)
    //    return expf(-lngammaf(1+_x) - lngammaf(1-_x));

    // _x ~ 0 approximation
    // from : http://mathworld.wolfram.com/SincFunction.html
    // sinc(z) = \prod_{k=1}^{\infty} { cos(\pi z / 2^k) }
    if (fabsf(_x) < 0.01f)
        return cosf(M_PI*_x/2.0f)*cosf(M_PI*_x/4.0f)*cosf(M_PI*_x/8.0f);

    return sinf(M_PI*_x)/(M_PI*_x);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// next power of 2 : y = ceil(log2(_x))
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int liquid_nextpow2(unsigned int _x)
{
    if (_x == 0) {
        //fprintf(stderr,"error: liquid_nextpow2(), input must be greater than zero\n");
        //exit(1);
    }

    _x--;
    unsigned int n=0;
    while (_x > 0) {
        _x >>= 1;
        n++;
    }
    return n;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// (n choose k) = n! / ( k! (n-k)! )
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float liquid_nchoosek(unsigned int _n, unsigned int _k)
{
    // 
    if (_k > _n) {
        //fprintf(stderr,"error: liquid_nchoosek(), _k cannot exceed _n\n");
        //exit(1);
    } else if (_k == 0 || _k == _n) {
        return 1;
    }

    // take advantage of symmetry and take larger value
    if (_k < _n/2)
        _k = _n - _k;

    // use lngamma() function when _n is large
    if (_n > 12) {
        double t0 = lgamma((double)_n + 1.0f);
        double t1 = lgamma((double)_n - (double)_k + 1.0f);
        double t2 = lgamma((double)_k + 1.0f);

        return round(exp( t0 - t1 - t2 ));
    }

    // old method
    float rnum=1, rden=1;
    unsigned int i;
    for (i=_n; i>_k; i--)
        rnum *= i;
    for (i=1; i<=_n-_k; i++)
        rden *= i;
    return rnum / rden;
}


#define NUM_BESSELI_ITERATIONS 64
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Bessel Functions
//
// log(I_v(z)) : log Modified Bessel function of the first kind
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float liquid_lnbesselif(float _nu,
                        float _z)
{
    // TODO : validate input
    // TODO : compute low-signal approximation to avoid log(0)

#if 0
    // high-signal approximation: _z >> _nu
    //     I_v(z) ~ exp(z) / sqrt(2*pi*z)
    //  ln I_v(z) ~ z - 0.5*ln(2*pi*z)
    if ( _nu > 0.0f && logf(_z/_nu) > 8.0f )
        return _z - 0.5f*logf(2*M_PI*_z);
#endif

    float t0 = _nu*logf(0.5f*_z);
    float t1 = 0.0f;
    float t2 = 0.0f;
    float t3 = 0.0f;
    float y = 0.0f;

    unsigned int k;
    for (k=0; k<NUM_BESSELI_ITERATIONS; k++) {
        // compute log( (z^2/4)^k )
        t1 = 2.0f * k * logf(0.5f*_z);

        // compute: log( k! * Gamma(nu + k +1) )
        t2 = liquid_lngammaf((float)k + 1.0f);
        t3 = liquid_lngammaf(_nu + (float)k + 1.0f);

        // accumulate y
        y += expf( t1 - t2 - t3 );
    }

    return t0 + logf(y);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// I_v(z) : Modified Bessel function of the first kind
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float liquid_besselif(float _nu,
                      float _z)
{
    return expf( liquid_lnbesselif(_nu, _z) );
}

#define NUM_BESSELI0_ITERATIONS 32
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// I_0(z) : Modified bessel function of the first kind (order zero)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float liquid_besseli0f(float _z)
{
    // TODO : use better low-signal approximation
    if (_z == 0.0f)
        return 1.0f;

    unsigned int k;
    float t, y=0.0f;
    for (k=0; k<NUM_BESSELI0_ITERATIONS; k++) {
#if 1
        t = powf(_z/2, (float)k) / tgamma((float)k+1);
        y += t*t;
#else
        t = k * logf(0.5f*_z) - liquid_lngammaf((float)k + 1.0f);
        y += expf(2*t);
#endif
    }
    return y;
}

#define NUM_BESSELJ_ITERATIONS 128
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// J_v(z) : Bessel function of the first kind
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float liquid_besseljf(float _nu,
                      float _z)
{
    // TODO : validate input

    float J = 0.0f;

    float abs_nu = fabsf(_nu);

    unsigned int k;
    for (k=0; k<NUM_BESSELJ_ITERATIONS; k++) {
        // compute: (2k + |nu|)
        float t0 = (2.0f*k + abs_nu);

        // compute: (2k + |nu|)*log(z)
        float t1 = t0 * logf(_z);

        // compute: (2k + |nu|)*log(2)
        float t2 = t0 * logf(2.0f);

        // compute: log(Gamma(k+1))
        float t3 = liquid_lngammaf((float)k + 1.0f);

        // compute: log(Gamma(|nu|+k+1))
        float t4 = liquid_lngammaf(abs_nu + (float)k + 1.0f);

        // accumulate J
        if ( (k%2) == 0) J += expf(t1 - t2 - t3 - t4);
        else             J -= expf(t1 - t2 - t3 - t4);
    }

    return J;
}

#define NUM_BESSELJ0_ITERATIONS 16
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// J_0(z) : Bessel function of the first kind (order zero)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float liquid_besselj0f(float _z)
{
    // large signal approximation, see
    // Gross, F. B "New Approximations to J0 and J1 Bessel Functions,"
    //   IEEE Trans. on Antennas and Propagation, vol. 43, no. 8,
    //   August, 1995
    if (fabsf(_z) > 10.0f)
        return sqrtf(2/(M_PI*fabsf(_z)))*cosf(fabsf(_z)-M_PI/4);

    unsigned int k;
    float t, y=0.0f;
    for (k=0; k<NUM_BESSELJ0_ITERATIONS; k++) {
        t = powf(_z/2, (float)k) / tgamma((float)k+1);
        y += (k%2) ? -t*t : t*t;
    }
    return y;
}


#define NUM_LNGAMMA_ITERATIONS (256)
#define EULER_GAMMA            (0.57721566490153286)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float liquid_lngammaf(float _z)
{
    float g;
    if (_z < 0) {
        //fprintf(stderr,"error: liquid_lngammaf(), undefined for z <= 0\n");
        //exit(1);
    } else if (_z < 10.0f) {
#if 0
        g = -EULER_GAMMA*_z - logf(_z);
        unsigned int k;
        for (k=1; k<NUM_LNGAMMA_ITERATIONS; k++) {
            float t0 = _z / (float)k;
            float t1 = logf(1.0f + t0);

            g += t0 - t1;
        }
#else
        // Use recursive formula:
        //    gamma(z+1) = z * gamma(z)
        // therefore:
        //    log(Gamma(z)) = log(gamma(z+1)) - ln(z)
        return liquid_lngammaf(_z + 1.0f) - logf(_z);
#endif
    } else {
        // high value approximation
        g = 0.5*( logf(2*M_PI)-log(_z) );
        g += _z*( logf(_z+(1/(12.0f*_z-0.1f/_z)))-1);
    }
    return g;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float liquid_gammaf(float _z)
{
    if (_z < 0) {
        // use identities
        //  (1) gamma(z)*gamma(-z) = -pi / (z*sin(pi*z))
        //  (2) z*gamma(z) = gamma(1+z)
        //
        // therefore:
        //  gamma(z) = pi / ( gamma(1-z) * sin(pi*z) )
        float t0 = liquid_gammaf(1.0 - _z);
        float t1 = sinf(M_PI*_z);
        //if (t0==0 || t1==0) fprintf(stderr,"warning: liquid_gammaf(), divide by zero\n");
        return M_PI / (t0 * t1);
    } else {
        return expf( liquid_lngammaf(_z) );
    }
}

#define LOWERGAMMA_MIN_ITERATIONS 50    // minimum number of iterations
#define LOWERGAMMA_MAX_ITERATIONS 1000  // maximum number of iterations
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ln( gamma(z,alpha) ) : lower incomplete gamma function
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float liquid_lnlowergammaf(float _z, float _alpha)
{
    float t0 = _z * logf(_alpha);
    float t1 = liquid_lngammaf(_z);
    float t2 = -_alpha;
    float t3 = 0.0f;

    unsigned int k = 0;
    float log_alpha = logf(_alpha);
    float tprime = 0.0f;
    float tmax = 0.0f;
    float t = 0.0f;
    for (k=0; k<LOWERGAMMA_MAX_ITERATIONS; k++) {
        // retain previous value for t
        tprime = t;

        // compute log( alpha^k / Gamma(_z + k + 1) )
        //         = k*log(alpha) - lnGamma(_z + k + 1)
        t = k*log_alpha - liquid_lngammaf(_z + (float)k + 1.0f);

        // accumulate e^t
        t3 += expf(t);

        // check premature exit criteria
        if (k==0 || t > tmax)
            tmax = t;

        // conditions:
        //  1. minimum number of iterations met
        //  2. surpassed inflection point: k*log(alpha) - log(Gamma(z+k+1))
        //     has an inverted parabolic shape
        //  3. sufficiently beyond peak
        if ( k > LOWERGAMMA_MIN_ITERATIONS && tprime > t && (tmax-t) > 20.0f)
            break;
    }

    return t0 + t1 + t2 + logf(t3);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ln( Gamma(z,alpha) ) : upper incomplete gamma function
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float liquid_lnuppergammaf(float _z, float _alpha)
{
    return logf( liquid_gammaf(_z) - liquid_lowergammaf(_z,_alpha) );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// gamma(z,alpha) : lower incomplete gamma function
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float liquid_lowergammaf(float _z, float _alpha)
{
    return expf( liquid_lnlowergammaf(_z,_alpha) );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Gamma(z,alpha) : upper incomplete gamma function
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float liquid_uppergammaf(float _z, float _alpha)
{
    return expf( liquid_lnuppergammaf(_z,_alpha) );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// compute _n!
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float liquid_factorialf(unsigned int _n) {
    return fabsf(liquid_gammaf((float)(_n+1)));
}

