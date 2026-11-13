#pragma once
#ifndef TESTEYEROOT_H
#define TESTEYEROOT_H

#ifndef CTARGAIMAGE_H
#include "../FileIO/CTargaImage.h"
#endif

#ifndef EYE_H
#include "../Eye/Eye.h"
#endif
#ifndef RENDERPLATE_H
#include "../Render/RenderPlate.h"
#endif
//const std::string g_baseDir = "Dat/";

class TestEyeRoot : public Base {
public:
	TestEyeRoot();
	~TestEyeRoot();

	unsigned char init(
		s_rtHexPlate* loPlate,
		s_Eye* seye,
		Img* eyeBaseImg,
		CTargaImage* tgaImgOut
	);
	void release();

	unsigned char runWrite();

protected:
	s_rtHexPlate* m_loPlate;/*not owned*/
	Img* m_eyeBaseImg;/*not owned*/
	s_Eye* m_seye;/*not owned*/
	CTargaImage* m_tgaImgOut;/*not owned*/

	RenderPlate* m_renderPlate;/*owned*/
	RenderPlate* m_renderPlateHi;/*owned*/

	unsigned char genHexedImg(Img* hexed_img);/*hexed_img must exist but not be initalized*/
	unsigned char writeImage(Img* hexed_img);
};
#endif