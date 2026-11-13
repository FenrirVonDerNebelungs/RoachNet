#include "RenderPlate.h"

RenderPlate::RenderPlate() {
	for (int i = 0; i < 6; i++) {
		m_web_start_cols[i].r = 0x00;
		m_web_start_cols[i].g = 0x00;
		m_web_start_cols[i].b = 0x00;
		m_web_end_cols[i].r = 0x00;
		m_web_end_cols[i].g = 0x00;
		m_web_end_cols[i].b = 0x00;
	}
	utilStruct::zeroRGB(m_hanging_col);
	utilStruct::zeroRGB(m_accent_col);
}
RenderPlate::~RenderPlate() {
}

unsigned char RenderPlate::init(float r, bool use_dv_loc, float grid_line_width, s_2pt* Us) {
	if (Err(RenderBase::init(r, false, true, use_dv_loc, grid_line_width, Us)))
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
void RenderPlate::release() {
	RenderBase::release();
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
		if (m_flag_dv_loc) {
			float half_w = (float)iimg[i_web]->getWidth() / 2.f;
			float half_h = (float)iimg[i_web]->getHeight() / 2.f;
			img_offset.x0 = Math::ftol(half_w);
			img_offset.x1 = Math::ftol(half_h);
		}
		ercode = RenderWebPlate_to_Img(plt, img_offset, i_web, iimg[i_web]);
		if (Err(ercode))
			return ercode;
	}
	return ercode;
}
unsigned char RenderPlate::spawnHangingWebPlateImg(s_HexPlate* hi_plt, s_HexPlate* lo_plt, long plt_nd_i, Img* iimg) {
	if (hi_plt == NULL || lo_plt == NULL || iimg == NULL)
		return ECODE_FAIL;
	if (IsImgInit(iimg)) {
		if (!(IsImgDimMatch((s_HexPlate*)lo_plt, iimg)))
			return ECODE_ABORT;
	}
	else
		if (Err(InitImg_for_HexPlate((s_HexPlate*)lo_plt, iimg)))
			return ECODE_FAIL;

	s_2pt_i img_offset = { 0L, 0L };
	float half_w = (float)lo_plt->width / 2.f;
	float half_h = (float)lo_plt->height / 2.f;
	img_offset.x0 = Math::ftol(half_w);
	img_offset.x1 = Math::ftol(half_h);

	m_flag_dv_loc = true;
	m_R = lo_plt->Rhex;
	for (int i_u = 0; i_u < 3; i_u++)
		m_U[i_u] = lo_plt->hexU[i_u];
	if(Err(RenderHexPlate(lo_plt, img_offset, iimg)))
		return ECODE_FAIL;
	return RenderHangingWeb_to_img(hi_plt, lo_plt, plt_nd_i, img_offset, iimg);
}
void RenderPlate::despawnWebPlateImg(Img* iimg[]) {
	if (iimg == NULL)
		return;
	for (int i_web = 0; i_web < 6; i_web++) {
		if (iimg[i_web] != NULL) {
			iimg[i_web]->release();
		}
	}
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
	const float RS = m_R * sqrtf(3.f) / 2.f;
	if (m_flag_dv_loc) {
		s_2pt pixLoc = n_HexBase::Loc((s_rtHex*)hex, m_U, RS); //hexU is twisted
		/* set the i, j loc in the twistetd plate relative to the center of the plate, or eye center but in pixels */
		start_ij.x0 = Math::ftol(pixLoc.x0) + center.x0;
		start_ij.x1 = Math::ftol(pixLoc.x1) + center.x1;
	}
	s_Hex* web_end_hex = hex->getWeb(web_i);
	unsigned char err = ECODE_OK;
	if (web_end_hex != NULL) {
		s_2pt_i end_ij = { web_end_hex->i + center.x0, web_end_hex->j + center.x1 };
		if (m_flag_dv_loc) {
			s_2pt pixLoc = n_HexBase::Loc((s_rtHex*)web_end_hex, m_U, RS); //hexU is twisted
			/* set the i, j loc in the twistetd plate relative to the center of the plate, or eye center but in pixels */
			end_ij.x0 = Math::ftol(pixLoc.x0) + center.x0;
			end_ij.x1 = Math::ftol(pixLoc.x1) + center.x1;
		}
		err=iimg->DrawLineGrad(start_ij, end_ij, m_web_start_cols[web_i], m_web_end_cols[web_i]);
	}
	return err;
}
unsigned char RenderPlate::RenderHangingWeb_to_img(s_HexPlate* hi_plt, s_HexPlate* lo_plt, long plt_nd_i, s_2pt_i& center, Img* iimg) {
	if (hi_plt == NULL || lo_plt == NULL || iimg == NULL)
		return ECODE_FAIL;
	s_Hex* hi_hex = hi_plt->get(plt_nd_i);
	if(hi_hex==NULL)
		return ECODE_ABORT;
	m_R = lo_plt->Rhex;
	for (int i_u = 0; i_u < 3; i_u++)
		m_U[i_u] = lo_plt->hexU[i_u];
	for(int i_lo=0; i_lo<hi_hex->N; i_lo++) {
		s_Hex* lo_hex = hi_hex->getHanging(i_lo);
		if(lo_hex==NULL)
			return ECODE_ABORT;
		float intensity = hi_hex->w[i_lo];
		const float intensity_scale = 10.f;
		intensity *= intensity_scale;
		s_rgb lo_hex_col = imgMath::mulIntensity(m_hanging_col, intensity);
		s_rtHex* low_hex_copy = new s_rtHex(*(s_rtHex*)lo_hex);
		low_hex_copy->setRGB(lo_hex_col.r, lo_hex_col.g, lo_hex_col.b);
		if (Err(RenderHex(low_hex_copy, center, iimg, true, true)))
			return ECODE_FAIL;
		delete low_hex_copy;
	}
	m_R = hi_plt->Rhex;
	for (int i_u = 0; i_u < 3; i_u++)
		m_U[i_u] = hi_plt->hexU[i_u];
	hi_hex->rgb[2] = 0xff;
	return RenderHex(hi_hex, center, iimg, false, true);
}