#pragma once
#ifndef TESTTWISTED_H
#define TESTTWISTED_H

#ifndef CTARGAIMAGE_H
#include "../FileIO/CTargaImage.h"
#endif

#ifndef EYE_H
#include "../Eye/Eye.h"
#endif
#ifndef RENDERPLATE_H
#include "../Render/RenderPlate.h"
#endif

const int g_numTwistedRot = 6;
const int g_numTwistedShadows = 9;

class TestTwisted : public Base{
public:
	TestTwisted();
	~TestTwisted();

	unsigned char init(
		s_Eye* seye,
		CTargaImage* tgaImgOut,
		int twisted_rot_i = 0
	);
	void release();

	unsigned char runWrite();
protected:
	s_Eye* m_seye;/*not owned*/
	CTargaImage* m_tgaImgOut;/*not owned*/
	RenderPlate* m_renderPlateTwist;/*owned*/
	RenderPlate* m_renderPlate;/*owned*/
	int m_twisted_rot_i;
	int m_twisted_shadow_i[9];


	unsigned char genTwistedShadows(Img* hexed_imgs[]);
	unsigned char genTwistedShadow(Img* hexed_img, int twisted_hex_i);/*hexed_img must exist but not be initalized*/
	unsigned char genTwistedHexImg(Img* hexed_img);
	unsigned char writeImages(Img* hexed_img, Img* shadow_imgs[]);
};
#endif