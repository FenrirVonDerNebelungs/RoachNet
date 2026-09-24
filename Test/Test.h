#pragma once
#ifndef TEST_H
#define TEST_H

#ifndef CTARGAIMAGE_H
#include "../FileIO/CTargaImage.h"
#endif
#ifndef EYE_H
#include "../Eye/Eye.h"
#endif

class Test : public Base {
public:
	Test();
	~Test();

	unsigned char init();

	
private:
	/*owned*/
	CTargaImage* m_tgaImg;
	CTargaImage* m_tgaImgOut;
	Img* m_eyeBaseImg;
	HexImg* m_hexImg;
	Eye* m_Eye;

	s_rtHexPlate* m_imgHexedPlate;
	s_ConvolHex m_eyeBaseConvolHexMaskVars;
	s_Eye* m_seye;

	unsigned char readInSourceImg();
	unsigned char runEye();
};
#endif