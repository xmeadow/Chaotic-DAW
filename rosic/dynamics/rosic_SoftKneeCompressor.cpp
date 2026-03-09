#include "rosic_SoftKneeCompressor.h"
using namespace rosic;

// Helper function to fit a cubic polynomial y = a0 + a1*x + a2*x^2 + a3*x^3
// given two points (x1, y1), (x2, y2) and their derivatives yd1, yd2
static void fitCubicWithDerivative(double x1, double x2, double y1, double y2,
                                   double yd1, double yd2,
                                   double* a3, double* a2, double* a1, double* a0)
{
    // We need to solve for coefficients a0, a1, a2, a3 in:
    // y1 = a0 + a1*x1 + a2*x1^2 + a3*x1^3
    // y2 = a0 + a1*x2 + a2*x2^2 + a3*x2^3
    // yd1 = a1 + 2*a2*x1 + 3*a3*x1^2
    // yd2 = a1 + 2*a2*x2 + 3*a3*x2^2
    
    // Let's solve using matrix algebra
    // Create matrix M = [[1, x1, x1^2, x1^3],
    //                    [1, x2, x2^2, x2^3],
    //                    [0, 1,  2*x1, 3*x1^2],
    //                    [0, 1,  2*x2, 3*x2^2]]
    // Solve M * [a0, a1, a2, a3]^T = [y1, y2, yd1, yd2]^T
    
    // For simplicity, we can solve using direct formulas for cubic Hermite interpolation
    // The cubic Hermite polynomial is:
    // h00(t) = 2t^3 - 3t^2 + 1
    // h10(t) = t^3 - 2t^2 + t
    // h01(t) = -2t^3 + 3t^2
    // h11(t) = t^3 - t^2
    // where t = (x - x1)/(x2 - x1)
    
    double dx = x2 - x1;
    if (fabs(dx) < 1e-12) {
        // Points are too close, use linear interpolation
        *a3 = 0.0;
        *a2 = 0.0;
        *a1 = yd1;
        *a0 = y1 - yd1 * x1;
        return;
    }
    
    // Compute coefficients using cubic Hermite interpolation formulas
    // Convert to standard polynomial form: a0 + a1*x + a2*x^2 + a3*x^3
    
    double t = 1.0 / dx;
    double t2 = t * t;
    double t3 = t2 * t;
    
    // Coefficients in terms of (x - x1)/dx
    double c0 = y1;
    double c1 = yd1 * dx;
    double c2 = -3*y1 + 3*y2 - 2*yd1*dx - yd2*dx;
    double c3 = 2*y1 - 2*y2 + yd1*dx + yd2*dx;
    
    // Now convert from form: c0 + c1*t + c2*t^2 + c3*t^3 where t = (x - x1)/dx
    // to form: a0 + a1*x + a2*x^2 + a3*x^3
    
    // t = (x - x1)/dx = x/dx - x1/dx
    // Let u = x/dx, v = x1/dx, then t = u - v
    
    // Expand (u - v)^n using binomial theorem:
    // (u - v)^1 = u - v
    // (u - v)^2 = u^2 - 2uv + v^2
    // (u - v)^3 = u^3 - 3u^2v + 3uv^2 - v^3
    
    // So:
    // c0
    // + c1*(u - v)
    // + c2*(u^2 - 2uv + v^2)
    // + c3*(u^3 - 3u^2v + 3uv^2 - v^3)
    
    // Collect terms by powers of u (where u = x/dx):
    // u^3: c3
    // u^2: c2 - 3*c3*v
    // u^1: c1 - 2*c2*v + 3*c3*v^2
    // u^0: c0 - c1*v + c2*v^2 - c3*v^3
    
    // But u = x/dx, so u^n = x^n / dx^n
    // Therefore:
    // a3 = c3 / dx^3
    // a2 = (c2 - 3*c3*v) / dx^2
    // a1 = (c1 - 2*c2*v + 3*c3*v^2) / dx
    // a0 = c0 - c1*v + c2*v^2 - c3*v^3
    
    double v = x1 / dx;
    double v2 = v * v;
    double v3 = v2 * v;
    
    *a3 = c3 * t3;
    *a2 = (c2 - 3.0 * c3 * v) * t2;
    *a1 = (c1 - 2.0 * c2 * v + 3.0 * c3 * v2) * t;
    *a0 = c0 - c1 * v + c2 * v2 - c3 * v3;
}

//-------------------------------------------------------------------------------------------------
// construction/destruction:

SoftKneeCompressor::SoftKneeCompressor(int newLookAheadBufferSize) 
: Compressor(newLookAheadBufferSize)
{
  kneeWidth = 0.0;
  a0        = 0.0;
  a1        = 1.0;  // coeff for x^1
  a2        = 0.0;
  a3        = 0.0;
  a4        = 0.0;
  kneeShape = QUARTIC;
  antiAlias = true;
}

SoftKneeCompressor::~SoftKneeCompressor()
{

}

//-------------------------------------------------------------------------------------------------
// parameter settings:

void SoftKneeCompressor::setKneeWidth(double newKneeWidth)
{
  if( newKneeWidth >= 0.0 )
  {
    kneeWidth = newKneeWidth;
    calculateCoefficients();
  }
}

void SoftKneeCompressor::setKneeShape(int newKneeShape)
{
  if( newKneeShape >= CUBIC && newKneeShape <= QUARTIC )
  {
    kneeShape = newKneeShape;
    calculateCoefficients();
  }
}

//-------------------------------------------------------------------------------------------------
// others:

void SoftKneeCompressor::calculateCoefficients()
{
  if( kneeShape == CUBIC )
    calculateCubicCoefficients();
  else
    calculateQuarticCoefficients();
  updateAutoGainFactor();
}

void SoftKneeCompressor::calculateCubicCoefficients()
{
  double x1  = threshold - 0.5*kneeWidth;
  double y1  = threshold - 0.5*kneeWidth;
  double yd1 = 1.0;
  double x2  = threshold + 0.5*kneeWidth;
  double y2  = threshold + (1.0/ratio)*(0.5*kneeWidth);
  double yd2 = 1.0/ratio;
  if( limiterMode )
  {
    y2  = threshold;
    yd2 = 0.0;
  }
  a4 = 0.0;
  fitCubicWithDerivative(x1, x2, y1, y2, yd1, yd2, &a3, &a2, &a1, &a0);
}

void SoftKneeCompressor::calculateQuarticCoefficients()
{   
  double t = threshold;
  double r = ratio;
  double c = 0.5*kneeWidth;

  double t2 = t*t;
  double t3 = t*t2;
  double t4 = t2*t2;
  double c2 = c*c;
  double c3 = c*c2;
  double c4 = c2*c2;

  if( r == INF || limiterMode == true )
  {
    a0 = (t4 - 6*c2*t2 + 8*c3*t - 3*c4) / (16*c3);
    a1 = -(t3 - 3*c2*t - 2*c3) / (4*c3);
    a2 = (3*t2 - 3*c2) / (8*c3);
    a3 = -t / (4*c3);
    a4 = 1  / (16*c3);
  }
  else
  {
    a0 = (r-1)*t4 + c2*(6-6*r)*t2 + c3*(8*r-8)*t + c4*(3-3*r);
    a0 = a0 / (16*c3*r);
    a1 = (r-1)*t3 + c2*(3-3*r)*t + c3*(-2*r-2);
    a1 = -a1 / (4*c3*r);
    a2 = (3*r-3)*t2 + c2*(3-3*r);
    a2 = a2 / (8*c3*r);
    a3 = -(r-1)*t / (4*c3*r);
    a4 = (r-1) / (16*c3*r);
  }
}
