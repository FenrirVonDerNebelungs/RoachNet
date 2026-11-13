#pragma once
#ifndef TESTHEXIMG_H
#define TESTHEXIMG_H

#ifndef CTARGAIMAGE_H
#include "../FileIO/CTargaImage.h"
#endif
#ifndef RENDERPLATE_H
#include "../Render/RenderPlate.h"
#endif
#ifndef HEXIMG_H
#include "../HexedImg/HexImg.h"
#endif

//const std::string g_baseDir = "Dat/";

class TestHexImg : public Base {
public:
	TestHexImg();
	~TestHexImg();

	unsigned char init(
		HexImg* hexImg,
		s_rtHexPlate* m_imgHexedPlate,
		CTargaImage* tgaImgOut
		);
	void release();

	unsigned char runWrite();

private:
	/*not owned*/
	HexImg* m_hexImg;
	s_rtHexPlate* m_imgHexedPlate;
	/*owned*/
	RenderPlate* m_render;
	CTargaImage* m_tgaImgOut;

	s_2pt_i m_img_center;
	s_2pt_i m_render_center;
	
	unsigned char genHexedImg(Img* hexed_img);
	unsigned char genWebImgs(Img* web_imgs[]);
	unsigned char writeImages(Img* hexed_img, Img* web_imgs[]);
};

#endif
