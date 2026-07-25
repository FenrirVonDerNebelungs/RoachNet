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
class ConvolHex {
public:
	ConvolHex();
	~ConvolHex();
	unsigned char init(Img* iimg, s_Node* hex[], float Rhex, float sigmaVsR=CONVOLHEX_sigmaVsR, float IMaskRVsR=CONVOLHEX_IMaskRVsR);/*used by HexImg, this and the func below need to be merged: a.k.a. this one should be removed at some point*/
	unsigned char init(Img* iimg, float Rhex, float sigmaVsR = CONVOLHEX_sigmaVsR, float IMaskRVsR = CONVOLHEX_IMaskRVsR);/*init for version that uses threaded, right now used by HexEyeImg*/
	void release();
	inline void update(Img* iimg) { m_img = iimg; }
	unsigned char convulToHex(int col_i);

	inline s_2pt_i& getIMaskBL_offset() { return m_IMaskBL_offset; }
	inline long getMaskHeight() { return m_IMask->getHeight(); }
	inline long getMaskWidth() { return m_IMask->getWidth(); }
	inline float* getMaskF() { return m_IMaskF; }
protected:
	/*not owned*/
	Img*   m_img;
	s_Node** m_hex;
	/*owned*/
	/*integration to fill hex with col, convulution*/
	Img* m_IMask;
	float* m_IMaskF;

	float m_Rhex;

	float m_sigmaVsR;
	float m_IMaskRVsR;

	float m_sigma;
	float  m_gaussNorm;/*1/sigma*sqrt(2pi)*/
	float  m_gaussExpConst;/*2*sigma^2*/
	float  m_IMaskR;
	s_2pt  m_IMaskCenter;
	s_2pt_i m_IMaskBL_offset;

	/* helpers to init */
	unsigned char genIMask();
	/**helpers to generating Integral mask**/
	float calcGaussian(s_2pt& pt);
	/**helpers to convolsion **/

	unsigned char convulMaskToHex(int col_i);/*uses the gausian with all three cols to set hex to single col*//*only one of the two convul functions will generally be used*/
	//unsigned char convulThreshtoHex(int col_i);/*  */
	/****                     ****/
	bool isIMaskInside(long hi, long hj);/*hi and hj represent the coords of the BL (or UL depending on y axis orientation)*/
	inline s_rgba convToRGBA(float r, float g, float b){ return imgMath::convToRGBA(r, g, b); }
	inline s_rgb  convToRGB(float r, float g, float b){ return imgMath::convToRGB(r, g, b); }
	/**                                   **/

	/*                                */
};

struct s_convKernVars {
	unsigned char* img_pix;
	long Img_height;
	long Img_width;
	long Img_bpp;
	long Img_maxIndex;

	float* mask_pix;/*chars divided by 255.f*/
	long MaskBL_offsetX;
	long MaskBL_offsetY;
	long Mask_height;
	long Mask_width;

	long hex_index;

	long num_Hex;
	s_Node** outHex;
};

namespace threadedConvol {
#ifndef MECVISPI_WIN
	inline bool convInImg(long width, long height, long x_i, long y_i);

	void *runConvThread(void* IOVarsVoid);
#endif
}
namespace n_Convol {
	void convCellKernel(s_convKernVars IOVars);
}
#endif
