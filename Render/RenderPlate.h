#pragma once
#ifndef RENDERPLATE_H
#define RENDERPLATE_H

#ifndef RENDERBASE_H
#include "RenderBase.h"
#endif

class RenderPlate : public RenderBase {
public:
	unsigned char init(float r, float grid_line_width = 1.f);
	unsigned char spawnWebPlateImg(s_HexPlate* plt, Img* iimg[]); /*iimg*'s should be non null but not initialized there should be 6 for the web dim */
	unsigned char spawnHangingWebPlateImg(s_HexPlate* hi_plt, s_HexPlate* lo_plt, long plt_nd_i, Img* iimg);

protected:
	s_rgb m_web_start_cols[6];
	s_rgb m_web_end_cols[6];
	s_rgb m_hanging_col;
	s_rgb m_accent_col;
	unsigned char RenderWebPlate_to_Img(s_HexPlate* plt, s_2pt_i& center, int web_i, Img* iimg);
	unsigned char RenderWeb(s_Hex* hex, s_2pt_i& center, int web_i, Img* iimg);
	unsigned char RenderHangingWeb_to_img(s_HexPlate* hi_plt, s_HexPlate* lo_plt, long plt_nd_i, s_2pt_i& center, Img* iimg);

};
 
#endif