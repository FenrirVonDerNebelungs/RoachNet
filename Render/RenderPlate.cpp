#include "RenderPlate.h"
unsigned char RenderPlate::init(float r, float grid_line_width) {
	if (Err(RenderBase::init(r, false, true, grid_line_width)))
		return ECODE_FAIL;
	for (int i_web = 0; i_web < 6; i_web++) {
		m_web_start_cols[i_web].r = 0x00;
		m_web_start_cols[i_web].g = 0x00;
		m_web_start_cols[i_web].b = 0x00;
		m_web_end_cols[i_web].r = 0xFF;
		m_web_end_cols[i_web].g = 0xFF;
		m_web_end_cols[i_web].b = 0xFF;
	}
	m_web_start_cols[0].r = 0xFF;
	m_web_start_cols[1].g = 0xFF;
	m_web_start_cols[2].b = 0xFF;
	m_web_start_cols[3].g = 0xFF;
	m_web_start_cols[4].b = 0xFF;
	m_web_start_cols[5].r = 0xFF;

	m_grid_col.r = 0xAA;
	m_grid_col.g = 0xAA;
	m_grid_col.b = 0xAA;
	m_hanging_col.r = 0xFF;
	m_hanging_col.g = 0xaa;
	m_hanging_col.b = 0xFF;
	m_accent_col.r = 0x00;
	m_accent_col.g = 0xFF;
	m_accent_col.b = 0x00;
	return ECODE_OK;
}
unsigned char RenderPlate::spawnWebPlateImg(s_HexPlate* plt, Img* iimg[]) {
	if (plt == NULL || iimg == NULL)
		return ECODE_FAIL;
	unsigned char ercode = ECODE_FAIL;
	m_flag_doGridOverlay = false;
	for (int i_web = 0; i_web < 6; i_web++) {
		if (iimg[i_web] == NULL)
			return ECODE_FAIL;
		if (IsImgInit(iimg[i_web])) {
			if (!(IsImgDimMatch((s_HexPlate*)plt, iimg[i_web])))
				return ECODE_ABORT;
		}
		else
			if (Err(InitImg_for_HexPlate((s_HexPlate*)plt, iimg[i_web])))
				return ECODE_FAIL;
		s_2pt_i img_offset = { 0L, 0L };
		ercode = RenderWebPlate_to_Img(plt, img_offset, i_web, iimg[i_web]);
		if (Err(ercode))
			return ercode;
	}
	return ercode;
}
unsigned char RenderPlate::spawnHangingWebPlateImg(s_HexPlate* hi_plt, s_HexPlate* lo_plt, long plt_nd_i, Img* iimg) {
	if (hi_plt == NULL || lo_plt == NULL || iimg == NULL)
		return ECODE_FAIL;
	s_2pt_i img_offset = { 0L, 0L };
	if(Err(RenderHexPlate(lo_plt, img_offset, iimg)))
		return ECODE_FAIL;
	return RenderHangingWeb_to_img(hi_plt, lo_plt, plt_nd_i, img_offset, iimg);
}

unsigned char RenderPlate::RenderWebPlate_to_Img(s_HexPlate* plt, s_2pt_i& center, int web_i, Img* iimg) {
	unsigned char ercode = ECODE_OK;
	if (iimg == NULL)
		return ECODE_FAIL;
	for (long hex_i = 0; hex_i < plt->N; hex_i++) {
		s_Hex* plate_hex = plt->get(hex_i);
		ercode = RenderHex(plate_hex, center, iimg, true, false);
		if (Err(ercode))
			return ercode;
	}
	for (long hex_i = 0; hex_i < plt->N; hex_i++) {
		s_Hex* plate_hex = plt->get(hex_i);
		ercode = RenderWeb(plate_hex, center, web_i, iimg);
		if (Err(ercode))
			return ercode;
	}
	return ECODE_OK;
}
unsigned char RenderPlate::RenderWeb(s_Hex* hex, s_2pt_i& center, int web_i, Img* iimg) {
	if (hex == NULL)
		return ECODE_FAIL;
	s_2pt_i start_ij = { hex->i + center.x0, hex->j + center.x1 };
	s_Hex* web_end_hex = hex->getWeb(web_i);
	s_2pt_i end_ij = { hex->i + center.x0, hex->j + center.x1 };
	return iimg->DrawLineGrad(start_ij, end_ij, m_web_start_cols[web_i], m_web_end_cols[web_i]);
}
unsigned char RenderPlate::RenderHangingWeb_to_img(s_HexPlate* hi_plt, s_HexPlate* lo_plt, long plt_nd_i, s_2pt_i& center, Img* iimg) {
	if (hi_plt == NULL || lo_plt == NULL || iimg == NULL)
		return ECODE_FAIL;
	s_Hex* hi_hex = hi_plt->get(plt_nd_i);
	if(hi_hex==NULL)
		return ECODE_ABORT;
	for(int i_lo=0; i_lo<hi_hex->N; i_lo++) {
		s_Hex* lo_hex = hi_hex->getHanging(i_lo);
		if(lo_hex==NULL)
			return ECODE_ABORT;
		float intensity = hi_hex->w[i_lo];
		s_rgb lo_hex_col = imgMath::mulIntensity(m_hanging_col, intensity);
		s_2pt_i lo_hex_ij = { lo_hex->i + center.x0, lo_hex->j + center.x1 };
		if (Err(RenderHex(lo_hex, center, iimg, true, true)))
			return ECODE_FAIL;
	}
	s_2pt_i hi_hex_ij = { hi_hex->i + center.x0, hi_hex->j + center.x1 };
	return RenderHex(hi_hex, center, iimg, false, true);
}