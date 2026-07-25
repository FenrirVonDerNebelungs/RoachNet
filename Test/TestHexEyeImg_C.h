#pragma once
#ifndef TESTHEXEYEIMG_C_H
#define TESTHEXEYEIMG_C_H
#ifndef RENDERBASE_H
#include "../Render/RenderBase.h"
#endif
#ifndef HEXEYEIMG_H
#include "../HexedImg/HexEyeImg.h"
#endif

class TestHexEyeImg_C : public Base {
public:
	TestHexEyeImg_C();
	~TestHexEyeImg_C();

	unsigned char init(
		Img* baseImg, 
		float r,/*little hex r*/
		int N_eye_levels
		);
	void release();

	unsigned char genImg(Img* hexed_img, int level=-1) {
		return genHexedImg(hexed_img, level);
	}/*hexed_img must exist but not be initalized*/
private:
	/*owned*/
	RenderBase* m_render;
	HexEyeImg* m_genHexedImg;
	/** generators **/
	HexEye* m_genHexEye;
	/** generated **/
	s_HexEye* m_seye;

	s_2pt_i m_img_center;
	s_2pt_i m_render_center;
	/*not owned*/
	Img* m_baseImg;/*image that will be converted into a hexed image*/

	unsigned char genHexedImg(Img* hexed_img, int level);/*hexed_img should exist but not be initialized*/
};

#endif
