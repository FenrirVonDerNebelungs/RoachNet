#pragma once
#ifndef RENDERPLATE_H
#define RENDERPLATE_H

#ifndef RENDERBASE_H
#include "RenderBase.h"
#endif
const std::string g_baseDir = "Dat/";

class RenderPlate : public RenderBase {
public:
	RenderPlate();
	~RenderPlate();
	unsigned char init(float r, bool use_dv_loc = false, float grid_line_width = 1.f, s_2pt* Us=NULL);
	void release();
	unsigned char spawnWebPlateImg(s_HexPlate* plt, Img* iimg[]); /*iimg*'s should be non null but not initialized there should be 6 for the web dim */
	unsigned char spawnHangingWebPlateImg(s_HexPlate* hi_plt, s_HexPlate* lo_plt, long plt_nd_i, Img* iimg);
	void despawnWebPlateImg(Img* iimg[]);
	unsigned char renderHangingWeb(s_HexPlate* hi_plt, s_HexPlate* lo_plt, long hi_plt_nd_i, Img* iimg) {
		s_2pt_i center = { 0L, 0L };
		return RenderHangingWeb_to_img(hi_plt, lo_plt, hi_plt_nd_i, center, iimg);
	}
	inline void setHangingCol(s_rgb& col) { m_hanging_col = col; }

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