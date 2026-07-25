#pragma once
#ifndef TESTLUNA_C_H
#define TESTLUNA_C_H

#ifndef RENDERBASE_H
#include "../Render/RenderBase.h"
#endif
#ifndef COLOR_H
#include "../HexedImg/Color.h"
#endif
#ifndef LUNALAYER_H
#include "../Luna/LunaLayer.h"
#endif

class TestLuna_C : public Base {

public:
	TestLuna_C();
	~TestLuna_C();

	unsigned char init(
		Img* baseImg,
		float r,
		long pix_between_imgs = 20,
		float luna_hue_ang = 0.37f,
		int num_eye_lev=4
	);
	void release();

	unsigned char render(Img* colPlateImg, Img* lunasImg, Img* lunaPlatesImg[]);/*these objects passed should be new, but not init, should be 6 pointers*/
	unsigned char renderBase(Img* hexedImg);
	inline Img* getBaseImg() { return m_baseImg; }
private:
	long m_pix_between_imgs;
	s_2pt_i m_eye_center;
	/*not owned*/
	Img* m_baseImg;

	/*owned*/
	RenderBase* m_render;
	s_2pt m_luna_hue_start;/*first vector in the set of 6 vectors that give the luna hues*/
	s_rgb_f m_luna_hues[6];
	/** generators **/
	/*** generators of data objects ***/
	HexEye* m_genHexEye;
	HexEyeImg* m_genHexedImg;
	Col* m_genColLayer;
	LunaLayer* m_genLunaLayer;

	/** data objs **/
	s_HexEye* m_seye;
	s_ColPlateLayer* m_colPlates;
	s_HexPlateLayer* m_lunaPlates;

	s_ColWheel m_sCol;

	unsigned char runLunaPlates();
	unsigned char renderHexLunas(Img* lunImg);
	unsigned char renderHexedBase(Img* hexedImg);
	unsigned char renderColPlate(Img* colImg);
	unsigned char renderLunaPlates(Img* lunPlates[]);/*lunPlates is an arry of new plates not init*/

	/*helper functions*/
	unsigned char renderLunaPlate(int luna_i, s_HexPlate* lunaPlate, Img* lunaPlateImg);/*luna Plate is new but not init*/

	/*init helper functions*/
	unsigned char genWhiteColWheel();
	unsigned char genLunaColHues(float luna_hue_ang);

	/*utility functions*/
	void setColFromI(const s_rgb_f& hue, float Intensity, float col_rgb[]);
	
};
#endif