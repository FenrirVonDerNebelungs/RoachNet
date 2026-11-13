#pragma once
#ifndef TESTLUNA_C_H
#define TESTLUNA_C_H

#ifndef CTARGAIMAGE_H
#include "../FileIO/CTargaImage.h"
#endif
#ifndef EYE_H
#include "../Eye/Eye.h"
#endif
#ifndef RENDERPLATE_H
#include "../Render/RenderPlate.h"
#endif


const int g_numTestHex = 3;

class TestLuna : public Base {

public:
	TestLuna();
	~TestLuna();

	unsigned char init(
		s_Eye* seye,
		CTargaImage* tgaImgOut
	);
	void release();

private:
	s_Eye* m_seye;/*not owned*/
	s_EyeCore* m_selCore;/*not owned*/
	s_rtHexPlate* m_selTwistedPlate;/*not owned*/
	s_HexPlateLayer* m_lunaPlates;/*not owned*/
	CTargaImage* m_tgaImgOut;/*not owned*/
	RenderPlate* m_renderPlateLo;/*owned*/
	RenderPlate* m_renderPlate;/*owned*/
	int m_test_hex_i[3] = {0, 4, 8};
	
	unsigned char genLunaPrints(Img* lunaPrints[]);/*6 prints*/
	unsigned char genLunaPrint(Img* lunaPrint, int lunaPrint_i);
	unsigned char genLunaPlates(Img* lunaPlates[]);/*6 plates*/
	unsigned char writeImages(Img* lunaPrints[], Img* lunaPlates[]);
};
#endif