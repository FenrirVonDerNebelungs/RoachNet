#pragma once
#ifndef TESTCOLOR_C_H
#define TESTCOLOR_C_H
#ifndef RENDERBASE_H
#include "../Render/RenderBase.h"
#endif
#ifndef COLOR_H
#include "../HexedImg/Color.h"
#endif

const float TestColor_C_pixMax = 255.f;

class TestColor_C : public Base {
public:
	TestColor_C();
	~TestColor_C();

	unsigned char init(
		Img* baseImg,
		float r = 7.f,
		int num_eye_lev = 3,
		int num_cols = 6,
		float Dhue = 0.3f,
		float DI = 0.9f,
		float DSat = 0.8f,
		float I_target = 0.5f,
		float Hue_targ_0 = 1.f,
		float Hue_targ_1 = 0.f,
		float Sat_target = 0.5f,
		float HueFadeV = 0.01f,
		float scale_factor_for_col_stepfunc = 10.f,
		long pix_between_imgs = 20
		);
	void release();

	unsigned char render(Img** imgs);/*Imgs should be an array of Imgs that are new but not initialized of at least length num_cols+1*/
private:
	long m_pix_between_imgs;
	s_2pt_i m_eye_center;
	/*not owned*/
	Img* m_baseImg;

	/*owned*/
	RenderBase* m_render;
	/** generators **/
	HexEye* m_genHexEye;
	HexEyeImg* m_genHexedImg;
	Col* m_genColLayer;
	/** data objs **/
	s_HexEye* m_seye;
	s_ColPlateLayer* m_colPlates;

	s_ColWheel* m_cols;/*color wheel setup to select the proper color for the color plate*/
	int m_n_cols;

	unsigned char genColWheels();

	unsigned char fillColPlateWithColor(s_ColPlate* colorPlate);
};
#endif