#pragma once
#ifndef CONVOLHEX_H
#define CONVOLHEX_H
#ifndef STRUCTS_H
#include "../Base/Structs.h"
#endif
#ifndef IMG_H
#include "../Base/Img.h"
#endif

#define CONVOLHEX_sigmaVsR 0.75f
#define CONVOLHEX_IMaskRVsR 1.5f

#define THREADEDCONVOL_NUMTHREADS 4
struct s_ConvolHex {
	Img* Mask_img;
	float* Mask;
	long height;
	long width;
	float MaskCenter_x0;
	float MaskCenter_x1;
	long MaskBL_offset_x0;
	long MaskBL_offset_x1;
};
struct s_convKernVars {
	unsigned char* img_pix;
	long Img_height;
	long Img_width;
	long Img_bpp;
	long Img_maxIndex;


	long hex_index;
	long num_Hex;

	s_Node** outHex;
};

class ConvolHex {
public:
	ConvolHex();
	~ConvolHex();
	unsigned char init(Img* iimg, float Rhex, float sigmaVsR = CONVOLHEX_sigmaVsR, float IMaskRVsR = CONVOLHEX_IMaskRVsR);/*init for version that uses threaded, right now used by HexEyeImg*/
	void release();

	inline s_ConvolHex getMask() { return m_Mask; }
protected:
	/*not owned*/
	Img*   m_img;/*must have correct dimensions*/
	/*owned*/
	/*integration to fill hex with col, convulution*/
	s_ConvolHex m_Mask;

	float m_Rhex;

	float m_sigmaVsR;
	float m_IMaskRVsR;

	float m_sigma;
	float  m_gaussNorm;/*1/sigma*sqrt(2pi)*/
	float  m_gaussExpConst;/*2*sigma^2*/
	float  m_IMaskR;

	/* helpers to init */
	unsigned char genIMask();
	/**helpers to generating Integral mask**/
	float calcGaussian(s_2pt& pt);
	/**helpers to convolsion **/

};
namespace n_ConvolHex {
	bool convulMaskToHex(const Img* img, const s_ConvolHex& Mask, const s_2pt_i& hex_loc, float rgb[]);/*uses the gausian with all three cols to set hex to single col*/
	void convulMaskToHex(s_ConvolHex MaskVars, s_convKernVars IOVars);
}


namespace threadedConvol {
#ifndef MECVISPI_WIN
	inline bool convInImg(long width, long height, long x_i, long y_i);

	void *runConvThread(void* IOVarsVoid);
#endif
}

#endif
