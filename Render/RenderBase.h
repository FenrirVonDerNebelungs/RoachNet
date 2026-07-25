#pragma once
#ifndef RENDERBASE_H
#define RENDERBASE_H
#ifndef STRUCTS_H
#include "../Base/Structs.h"
#endif
#ifndef IMG_H
#include "../Base/Img.h"
#endif
#ifndef COLPLATE_H
#include "../HexedImg/ColPlate.h"
#endif
const s_rgb RENDERBASE_rgb_empty = { 0x00, 0x00, 0x00 };
const float RENDERBASE_pix_max = 255.f;
namespace n_RenderBase {
	/*utility*/
	unsigned char setPlateIJ_toXY(s_HexPlate* plate);
	unsigned char setPlate_hex_o_toRGB(s_HexPlate* plate, s_rgb& col);
}
class RenderBase : public Base {
public:
	RenderBase();
	~RenderBase();

	unsigned char init(float r, bool rgb_unit_scaled=false, bool do_grid_overlay=true, float grid_line_width=1.f);
	unsigned char resetR(float r);
	void setGridOverlay(bool do_grid_overlay) { m_flag_doGridOverlay = do_grid_overlay; }
	void setGridLineWidth(float grid_line_width) { m_grid_line_width = grid_line_width; }
	inline void setGridCol(s_rgb& grid_col) { m_grid_col = grid_col; }
	void release();

	unsigned char spawnHexPlateImg(s_HexPlate* plt, Img* iimg);/*image pointer must already point to an object
									                             if image is of 0 width and height it is assumed it is not initialized 
																 and it will be initialized to the dimensions of the plate
																 if the image is already initalized it must be of the same dimensions of the 
																 plate inorder to not return an error */
	void despawnHexPlateImg(Img* iimg);

	unsigned char RenderHexPlate(s_HexPlate* plt, s_2pt_i& center, Img* iimg) { return RenderHexPlate_to_Img(plt, center, iimg); }
	unsigned char RenderHalfHexPlate(int half_hex_web_i, s_HexPlate* plt, s_2pt_i& center, Img* iimg) { return RenderHalfHexPlate_to_Img(half_hex_web_i, plt, center, iimg); }
	unsigned char RenderHex(s_Hex* hex, s_2pt_i& center, Img* iimg, bool doGridOverlay=false, bool doFill=true, int half_hex_web_i=-1);

	inline Img* getHexMask() { return m_hex_mask; }/*returns pointer but does not transfer ownership*/
	inline Img* getHexGridMask() { return m_hex_grid_mask; }
protected:
	bool m_flag_rgb_unit_scaled;
	bool m_flag_doGridOverlay;
	float m_grid_line_width;
	s_rgb m_grid_col;
	/*owned*/
	s_2pt m_U[6];
	float m_R;
	float m_RS;
	Img* m_hex_mask;/*mask of hex image from plate rounded to bigger*/
	Img* m_hex_grid_mask;
	s_2pt_i m_hex_mask_center;
	Img** m_half_hex_masks;

	bool IsImgInit(Img* iimg);
	bool IsImgDimMatch(s_HexPlate* plt, Img* iimg);
	unsigned char InitImg_for_HexPlate(s_HexPlate* plt, Img* iimg);
	unsigned char RenderHexPlate_to_Img(s_HexPlate* plt, s_2pt_i& center, Img* iimg);
	unsigned char RenderHalfHexPlate_to_Img(int half_hex_web_i, s_HexPlate* plt, s_2pt_i& center, Img* iimg);

	/*helpers*/
	unsigned char InitMask_for_hex_dim(float r, float rs);/*r is the long radius rs is the direct short side out radius*/

	/*helpers to helpers and base functions*/
	unsigned char InitMask_for_hexes(s_2pt hexU[], float Rhex, float RShex);
	unsigned char InitHalfMasks_for_hexes(s_2pt hexU[], float Rhex, float RShex);/*assumes m_hex_mask has already been filled, assumes m_half_hex_mask is an array of new but not init images*/
	unsigned char FillMask_for_U(s_2pt hexU[], float RShex, s_rgb& rgb_filled, Img* hex_mask);
	bool do_FillHexSeg(const s_2pt& pt, const s_2pt& U_prev, const s_2pt& U, const s_2pt& U_next, float rs);
	/**** helper to helpers to helpers... ****/
	unsigned char InitHalfMask_for_hex(s_2pt hexU[], int i_web, Img* halfMask);/*assumes m_hex_mask is already filled needs halfMask new but not init*/
};
#endif
