#pragma once
#ifndef TEST_H
#define TEST_H

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifndef TESTHEXIMG_H
#include "TestHexImg.h"
#endif
#ifndef TESTEYEROOT_H
#include "TestEyeRoot.h"
#endif
#ifndef TESTTWISTED_H
#include "TestTwisted.h"
#endif
#ifndef EYE_H
#include "../Eye/Eye.h"
#endif

const int g_numStackLevels = 3;
const long g_colorMode = 3L;

class Test : public Base {
public:
	Test();
	~Test();

	unsigned char init(
		float m_imgRHex = 9.f,
		float RHex=27.f,
		float twisted_root_radius=3.f,
		float sigma_hexImg=1.f
	);

	void release();

	unsigned char runWrite();
private:
	float m_imgRHex;
	float m_RHex;
	float m_twisted_root_radius;
	float m_sigma_hexImg;
	/*owned*/
	CTargaImage* m_tgaImg;
	CTargaImage* m_tgaImgOut;
	Img* m_eyeBaseImg;
	HexImg* m_hexImg;
	Eye* m_Eye;

	s_rtHexPlate* m_imgHexedPlate;
	s_ConvolHex m_eyeBaseConvolHexMaskVars;
	s_Eye* m_seye;

	/*test*/
	TestHexImg* m_testHexImg;
	TestEyeRoot* m_testEyeRoot;
	TestTwisted* m_testTwisted;
	

	unsigned char readInSourceImg();
	void clearSource();
	unsigned char runEye();

};
#endif