#pragma once
#ifndef MATH_H
#define MATH_H

#ifndef TUPLES_H
#include "Tuples.h"
#endif

#define MATH_RANDMOD 200
const unsigned char IMG_IMGBAK = 0x00;

class Img;

struct s_rgba {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};
struct s_rgb {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};
struct s_rgb_f {
	float r;
	float g;
	float b;
};

struct s_2pt_i {
	long x0;
	long x1;
};
struct s_2pt {
	float x0;
	float x1;
};

struct s_gaussianInt { /*this struct is for the integration of a gaussian centered at zero, from zero up to the values held in I */
	float sigma;
	float max;/*max value for which the integral is calculated*/
	float N;/*N-1 is the number of divisions*/
	float* I;/*integral values beginning at zero and ending at max, array of len N*/
	float* X;/*X values of erf function, array of len N*/
};
namespace n_gaussianInt {
	void clear(s_gaussianInt& gI);
	bool init(s_gaussianInt& gI, float sigma, float max=0.f, int N=1000);/*if max is left at zero then it is set to 2 sigma*/
	void release(s_gaussianInt& gI);
}
namespace utilStruct {
	inline void zeroRGBA(s_rgba& rgba) { rgba.r = 0x00; rgba.g = 0x00; rgba.b = 0x00; rgba.a = 0x00; }
	inline void zeroRGB(s_rgb& rgb) { rgb.r = 0x00; rgb.g = 0x00; rgb.b = 0x00; }
	inline void zeroRGB_f(s_rgb_f& rgb) { rgb.r = 0.f; rgb.g = 0.f; rgb.b = 0.f; }
	inline void zero2pt_i(s_2pt_i& spt) { spt.x0 = 0; spt.x1 = 0; }
	inline void zero2pt(s_2pt& pt) { pt.x0 = 0.f; pt.x1 = 0.f; }
	inline void copy2pt_i(s_2pt_i& ptcopy, const s_2pt_i& pt) { ptcopy.x0 = pt.x0; ptcopy.x1 = pt.x1; }
	inline void copy2pt(s_2pt& ptcopy, const s_2pt& pt) { ptcopy.x0 = pt.x0; ptcopy.x1 = pt.x1; }
}
namespace Math {
	float Ang2PI(float ang);/*limits angle to range [0,2PI)*/
	float StepFunc(float x);/* 1/(1+exp(-x))  takes an x value and returns a step function where - x goes to 0, 0 goes to 1/2 and +x goes to 1*/
	inline float StepFuncSym(float x) { return tanhf(x); }/* hyper bolic tangent function (exp(x) - exp(-x))/(exp(x) + exp(-x)) 0 at 0, -1 at neg inf reached close to 2, symetric pos 1 at pos inf*/
	inline float DStepFuncSym(float x) {
		float t = tanhf(x); return 1.f - t * t;
	}/* derivitave of tanhf with respect to x is 1-tanhf^2(x)*/
	int loop(int i, int n);
	float power(float x, int y);
	float powerXseries(float x, int n);/* computes 1+x+x^2+..x^n */
	float Gaussian(float pt, float norm_const, float center = 0.f);/*norm const is sigma*/
	float GaussianOneMax(float pt, float norm_const);
	float GaussianFast(float pt, float sqrt2_ExpConst, float Norm, float center);
	bool  GaussianNumInt(float norm_const, float max, float N, float I[], float x[]);/*divides gaussian interval into (N-1) spaces and numerically computes the integral from 0 to max
																		    and fills I with it, also fills X*/
	bool  GaussianNumInt(s_gaussianInt& Int);/*runs the aboe from values in Int, the arrays have to have already been generated and of len N*/
	/*random*/
	void timeSeed();
	float randAng(float startRad=0.f, float endRad=2.00001f*PI);/*returns a random angle between start and end, values are given in radians, requires start to end to be */
	float randGausPt(const s_gaussianInt& gI);/*returns a point with the prob of a gaussian, where the integral of the gaussian is given
				                         by I
										 the maximum point this will return possibly is going to be the max dim given in I
							             this function maps the integrated prob of the linear rand() function
										 onto the integrated prob of the gaussian */
	s_2pt randGaus2D(const s_2pt& center, const s_gaussianInt& gI);/*returns a point using randGausPt for the radius */
	float randGausJitterAng(const s_gaussianInt& gI,float center_ang);/*range around center angle is already set in gI, if point goes over range then starts wrapping*/
	long ftol(float f);
	float ltof(long l);
}
namespace arrMath {
	/*puts value into array at i and moves array forward after the value*/
	void put(float v, int i, float ar[], int n);
	void put(int v, int i, int ar[], int n);
	int getFirstBelow(float v, float ar[], int n, int i_start=0);
}
namespace vecMath {
	s_2pt add(const s_2pt& v1, const s_2pt& v2);
	s_2pt_i add(const s_2pt_i& v1, const s_2pt_i& v2);
	s_2pt add(const s_2pt& v1, const s_2pt& v2, const s_2pt& v3);
	s_2pt v12(const s_2pt& v1, const s_2pt& v2);
	Tup3 v12(const Tup3& v1, const Tup3& v2);
	float len(const s_2pt& v);
	float len(const Tup3& v);
	float dist(const s_2pt& v1, const s_2pt& v2);
	float dist(const Tup3& v1, const Tup3& v2);
	float dot(const s_2pt& v1, const s_2pt& v2);
	s_2pt mul(float a, const s_2pt& vec);
	s_2pt perpUR(s_2pt& vec);
	s_2pt perpUL(s_2pt& vec);
	void  setBasis(float angRad, s_2pt& basisU0, s_2pt& basisU1);
	void  revBasis(const s_2pt& basisU0, const s_2pt& basisU1, s_2pt& revBasisU0, s_2pt& revBasisU1);
	s_2pt convBasis(const s_2pt& basisU0, const s_2pt& basisU1, const s_2pt& vecInBasis);
	bool inCircle(float r, const s_2pt& center, const s_2pt& pt);
	float distCircleArc(float r, const s_2pt& center, const s_2pt& pt);/*returns negative if point is inside circle*/
	float distLineGivenPerp(const s_2pt& line_pt, const s_2pt& line_perp, const s_2pt& pt);/* returns pos distance from line in direction of line_perp, line_perp must be of length 1*/
}
namespace imgMath {
	unsigned char add(unsigned char c0, unsigned char c1);
	s_rgba convToRGBA(float r, float g, float b);
	s_rgb  convToRGB(float r, float g, float b);
	s_rgb mulIntensity(const s_rgb& rgb, float intensity);
	void IncRGB(s_rgb& rgb0, const s_rgb& rgb1);
	s_rgb gradRGB(const s_rgb& rgb0, const s_rgb& rgb1, float relI = 0.f);
	void limitRGB(s_rgb_f& rgb);/*limits each to a range between 0 an 255*/

	s_2pt_i convToVint(const s_2pt& vec);
	s_2pt convToVfloat(const s_2pt_i& vi);
	Tup3 rgbToTup3(const s_rgb& rgb);
	s_rgb tup3ToRgb(const Tup3& tup);
	void RGBToFloats(const s_rgb& rgb_c, float rgb_f[]);
	void FloatsToRGB(const float rgb_f[], s_rgb& rgb_c);/*does not automatically scale the float 1-> char 1 255 -> char 255*/

	void drawPoint(long i, long j, const s_rgba& col, Img* canvas);
	void drawLine(long i_start, long j_start, long i_end, long j_end, const s_rgba& col, float thickness, Img* canvas);
	void drawV(const s_2pt& v, long i, long j, const s_rgba& col, float thickness, Img* canvas);

	void nineAveFromPtOffset(const s_2pt& R, const s_2pt& center, s_2pt_i& img_pt, float perc[]); /*finds how each of the 9 squares around a center
														                       square should contribute to the value in the new
														                       square that does not perfectly overlay the old mesh
														                       perc has 9 values corresponding to the square above and below the img*/
	bool nineAveRGB(const s_2pt& R, const s_2pt& center, const Img* img, s_2pt_i& img_pt, s_rgb& retrgb);
}
namespace hexMath {
	bool inHex(float hexR, float hexRS, const s_2pt hexU[], const s_2pt& center, const s_2pt& pt, float padding = 0.f);
}

#endif