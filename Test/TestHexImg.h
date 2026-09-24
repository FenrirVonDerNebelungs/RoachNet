#pragma once
#ifndef TESTHEXIMG_H
#define TESTHEXIMG_H
#ifndef RENDERBASE_H
#include "../Render/RenderBase.h"
#endif
#ifndef HEXIMG_H
#include "../HexedImg/HexImg.h"
#endif

class TestHexImg : public Base {
public:
	TestHexImg();
	~TestHexImg();

	unsigned char init(
		HexImg* hexImg
		);
	void release();

	unsigned char genHexedImg(Img* hexed_img);
	unsigned char genWebImg(Img* web_img, int web_i);
private:
	/*owned*/
	RenderBase* m_render;

	s_2pt_i m_img_center;
	s_2pt_i m_render_center;
	/*not owned*/

};

#endif
