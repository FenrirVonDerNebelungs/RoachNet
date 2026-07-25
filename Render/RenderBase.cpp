#include "RenderBase.h"


unsigned char n_RenderBase::setPlateIJ_toXY(s_HexPlate* plate) {
	for (long ii = 0; ii < plate->N; ii++) {
		s_Hex* cur_hex = plate->get(ii);
		long i = Math::ftol(cur_hex->x);
		long j = Math::ftol(cur_hex->y);
		cur_hex->i = i;
		cur_hex->j = j;
	}
	return ECODE_OK;
}
unsigned char n_RenderBase::setPlate_hex_o_toRGB(s_HexPlate* plate, s_rgb& col) {
	float rgbf_col[] = {(float)col.r, (float)col.g, (float)col.b};
	for (long ii = 0; ii < plate->N; ii++) {
		s_Hex* cur_hex = plate->get(ii);
		float cur_o = cur_hex->o;
		for (int i_col = 0; i_col < 3; i_col++) {
			cur_hex->rgb[i_col] = cur_o * rgbf_col[i_col];
		}
	}
	return ECODE_OK;
}

RenderBase::RenderBase() : m_flag_rgb_unit_scaled(false), m_flag_doGridOverlay(false), m_grid_line_width(0.f), m_R(0.f), m_RS(0.f), m_hex_mask(NULL), m_hex_grid_mask(NULL), m_half_hex_masks(NULL) {
	utilStruct::zeroRGB(m_grid_col);
	for (int i = 0; i < 6; i++)
		utilStruct::zero2pt(m_U[i]);
	utilStruct::zero2pt_i(m_hex_mask_center);
}
RenderBase::~RenderBase() {
	;
}
unsigned char RenderBase::init(float r, bool rgb_unit_scaled, bool do_grid_overlay, float grid_line_width) {
	if (m_hex_mask != NULL || m_hex_grid_mask != NULL)
		return ECODE_FAIL;
	m_flag_rgb_unit_scaled = rgb_unit_scaled;
	m_flag_doGridOverlay = do_grid_overlay;
	m_grid_line_width = grid_line_width;
	m_grid_col.r = 0x30;
	m_grid_col.g = 0x30;
	m_grid_col.b = 0x00;
	m_hex_mask = new Img;
	m_hex_grid_mask = new Img;
	m_hex_mask_center.x0 = 0L;
	m_hex_mask_center.x1 = 0L;
	m_half_hex_masks = new Img * [6];
	for (int i_web = 0; i_web < 6; i_web++)
		m_half_hex_masks[i_web] = new Img;
	s_HexPlate s_dummy_hplate;
	s_dummy_hplate.init(3);
	for (int i_web = 0; i_web < 6; i_web++)
		m_U[i_web] = s_dummy_hplate.hexU[i_web];
	s_dummy_hplate.release();
	m_R = r;
	m_RS = sqrtf(3.f) / 2.f;
	m_RS *= m_R;
	if(Err(InitMask_for_hexes(m_U, m_R, m_RS)))
		return ECODE_FAIL;
	if(Err(InitHalfMasks_for_hexes(m_U, m_R, m_RS)))
		return ECODE_FAIL;
	return ECODE_OK;
}
unsigned char RenderBase::resetR(float r) {
	if (m_hex_mask == NULL || m_hex_grid_mask == NULL || m_half_hex_masks == NULL)
		return ECODE_FAIL;
	for (int i_web = 0; i_web < 6; i_web++) {
		if (m_half_hex_masks[i_web] == NULL)
			return ECODE_FAIL;
	}
	m_R = r;
	m_RS = sqrtf(3.f) / 2.f;
	m_RS *= m_R;
	m_hex_mask->release();
	m_hex_grid_mask->release();
	for (int i_web = 0; i_web < 6; i_web++)
		m_half_hex_masks[i_web]->release();
	if (Err(InitMask_for_hexes(m_U, m_R, m_RS)))
		return ECODE_FAIL;
	if (Err(InitHalfMasks_for_hexes(m_U, m_R, m_RS)))
		return ECODE_FAIL;

	return ECODE_OK;
}
void RenderBase::release() {
	if (m_half_hex_masks != NULL) {
		for (int i_web = 0; i_web < 6; i_web++) {
			if (m_half_hex_masks[i_web] != NULL) {
				m_half_hex_masks[i_web]->release();
				delete m_half_hex_masks[i_web];
			}
			m_half_hex_masks[i_web] = NULL;
		}
		delete[] m_half_hex_masks;
	}
	m_half_hex_masks = NULL;
	if (m_hex_grid_mask != NULL) {
		m_hex_grid_mask->release();
		delete m_hex_grid_mask;
	}
	m_hex_grid_mask = NULL;
	if (m_hex_mask != NULL) {
		m_hex_mask->release();
		delete m_hex_mask;
	}
	m_hex_mask = NULL;
}
unsigned char RenderBase::spawnHexPlateImg(s_HexPlate* plt, Img* iimg) {
	if (plt == NULL || iimg == NULL)
		return ECODE_FAIL;
	if (IsImgInit(iimg)) {
		if (!(IsImgDimMatch((s_HexPlate*)plt, iimg)))
			return ECODE_ABORT;
	}else
		if (Err(InitImg_for_HexPlate((s_HexPlate*)plt, iimg)))
			return ECODE_FAIL;
	s_2pt_i img_offset = { 0L, 0L };
	return RenderHexPlate_to_Img((s_HexPlate*)plt, img_offset, iimg);
}
void RenderBase::despawnHexPlateImg(Img* iimg) {
	if (iimg == NULL)
		return;
	iimg->release();
}
unsigned char RenderBase::RenderHex(s_Hex* hex, s_2pt_i& center, Img* iimg, bool doGridOverlay, bool doFill, int half_hex_web_i) {
	unsigned char ercode = ECODE_OK;
	if (hex == NULL)
		return ECODE_FAIL;
	s_rgb plate_col;
	float scaled_rgb[3];
	if (m_flag_rgb_unit_scaled)
		for (int i_col = 0; i_col < 3; i_col++)
			scaled_rgb[i_col] = hex->rgb[i_col] * RENDERBASE_pix_max;
	else
		for (int i_col = 0; i_col < 3; i_col++)
			scaled_rgb[i_col] = hex->rgb[i_col];
	imgMath::FloatsToRGB(scaled_rgb, plate_col);
	s_2pt_i ij = { hex->i + center.x0, hex->j + center.x1 };
	if (doFill)
		ercode |= iimg->PrintMaskedImg(ij.x0, ij.x1, *m_hex_mask, plate_col);
	if (doGridOverlay)
		ercode |= iimg->PrintMaskedImg(ij.x0, ij.x1, *m_hex_grid_mask, plate_col);
	if (half_hex_web_i >= 0 && half_hex_web_i < 6) {
		ercode |= iimg->PrintMaskedImg(ij.x0, ij.x1, *(m_half_hex_masks[half_hex_web_i]), plate_col);
	}
	return ercode;
}
bool RenderBase::IsImgInit(Img* iimg) {
	if (iimg == NULL)
		return false;
	unsigned char* internal_image_ptr = iimg->getImg();
	if (internal_image_ptr == NULL)
		return true;
	else return false;
}
bool RenderBase::IsImgDimMatch(s_HexPlate* plt, Img* iimg) {
	if (plt == NULL || iimg == NULL)
		return false;
	if (plt->width != iimg->getWidth())
		return false;
	if (plt->height != iimg->getHeight())
		return false;
	return true;
}
unsigned char RenderBase::InitImg_for_HexPlate(s_HexPlate* plt, Img* iimg) {
	if(Err( iimg->init(plt->width, plt->height, 3L))  )
		return ECODE_FAIL;
	iimg->clearToChar(RENDERBASE_rgb_empty.r);
	return ECODE_OK;
}
unsigned char RenderBase::RenderHexPlate_to_Img(s_HexPlate* plt, s_2pt_i& center, Img* iimg) {
	if (iimg == NULL)
		return ECODE_FAIL;
	unsigned char ercode = ECODE_OK;
	for (long hex_i = 0; hex_i < plt->N; hex_i++) {
		s_Hex* plate_hex = plt->get(hex_i);
		ercode=RenderHex(plate_hex, center, iimg, m_flag_doGridOverlay);
	}
	if (IsErrFail(ercode))
		return ECODE_FAIL;
	return ECODE_OK;
}
unsigned char RenderBase::RenderHalfHexPlate_to_Img(int half_hex_web_i, s_HexPlate* plt, s_2pt_i& center, Img* iimg) {
	if (iimg == NULL)
		return ECODE_FAIL;
	unsigned char ercode = ECODE_OK;
	for (long hex_i = 0; hex_i < plt->N; hex_i++) {
		s_Hex* plate_hex = plt->get(hex_i);
		ercode = RenderHex(plate_hex, center, iimg, m_flag_doGridOverlay, false, half_hex_web_i);
	}
	if (IsErrFail(ercode))
		return ECODE_FAIL;
	return ECODE_OK;
}


unsigned char RenderBase::InitMask_for_hex_dim(float r, float rs) {
	if (m_hex_mask == NULL || m_hex_grid_mask==NULL)
		return ECODE_FAIL;
	if (m_hex_mask->getWidth() >= 1 || m_hex_mask->getHeight() >= 1)
		m_hex_mask->release();
	if (m_hex_grid_mask->getWidth() >= 1 || m_hex_grid_mask->getHeight() >= 1)
		m_hex_grid_mask->release();
	long longest_possible_rad = (long)ceil(r);
	long longest_possible_dim = 2*longest_possible_rad;
	longest_possible_dim += 2L;
	if(Err(m_hex_mask->init(longest_possible_dim, longest_possible_dim, 3L)))
		return ECODE_FAIL;
	if (Err(m_hex_grid_mask->init(longest_possible_dim, longest_possible_dim, 3L)))
		return ECODE_FAIL;
	m_hex_mask_center.x0 = longest_possible_rad;/*0 is start not 1 so don't  need to add 1*/
	m_hex_mask_center.x1 = longest_possible_rad;
	m_hex_mask->clearToChar(0x00);
	m_hex_grid_mask->clearToChar(0x00);
	return ECODE_OK;
}



unsigned char RenderBase::InitMask_for_hexes(s_2pt hexU[], float Rhex, float RShex) {
	if (Err(InitMask_for_hex_dim(Rhex, RShex)))
		return ECODE_FAIL;
	s_rgb rgb_filled = { 0xff, 0xff, 0xff };
	s_rgb rgb_empty = { 0x00, 0x00, 0x00 };
	unsigned char errok = FillMask_for_U(hexU, RShex, rgb_filled, m_hex_mask);
	errok |= FillMask_for_U(hexU, RShex, rgb_filled, m_hex_grid_mask);
	float inner_RS = RShex - m_grid_line_width;
	if (inner_RS <= 0.f)
		inner_RS = RShex;
	errok |= FillMask_for_U(hexU, inner_RS, rgb_empty, m_hex_grid_mask);
	return errok;
}
unsigned char RenderBase::InitHalfMasks_for_hexes(s_2pt hexU[], float Rhex, float RShex) {
	for (int i_web = 0; i_web < 6; i_web++)
		if (Err(InitHalfMask_for_hex(hexU, i_web, m_half_hex_masks[i_web])))
			return ECODE_FAIL;
	return ECODE_OK;
}
unsigned char RenderBase::FillMask_for_U(s_2pt hexU[], float RShex, s_rgb& rgb_filled, Img* hex_mask) {
	for (long i_row = 0; i_row<hex_mask->getHeight(); i_row++) {
		for (long i_col = 0; i_col<hex_mask->getWidth(); i_col++) {
			long cur_index = i_row * hex_mask->getWidth() + i_col;
			s_2pt cur_loc = { (float)i_col, (float)i_row };
			s_2pt hex_mask_center = { (float)m_hex_mask_center.x0, (float)m_hex_mask_center.x1 };
			s_2pt pt = vecMath::v12(hex_mask_center, cur_loc);
			/*try rotating around the U's defined by the plate*/
			bool is_found_in_hex_slice = false;
			for (int u_i = 0; u_i < 6; u_i++) {
				int u_i_prev = Math::loop(u_i - 1, 6);
				int u_i_next = Math::loop(u_i + 1, 6);
				is_found_in_hex_slice = do_FillHexSeg(pt, hexU[u_i_prev], hexU[u_i], hexU[u_i_next], RShex);
				if (is_found_in_hex_slice)
					break;
			}
			if (is_found_in_hex_slice)
				hex_mask->SetRGB(i_col, i_row, rgb_filled);
		}
	}
	return ECODE_OK;
}
bool RenderBase::do_FillHexSeg(const s_2pt& pt, const s_2pt& U_prev, const s_2pt& U, const s_2pt& U_next, float rs) {
	float dot_p = vecMath::dot(U_prev, pt);
	float dot_u = vecMath::dot(U, pt);
	float dot_n = vecMath::dot(U_next, pt);
	if (dot_u < 0.f)
		return false;
	if (dot_u < dot_p || dot_u < dot_n)
		return false;
	if (dot_u > rs)
		return false;
	return true;
}
unsigned char RenderBase::InitHalfMask_for_hex(s_2pt hexU[], int i_web, Img* halfMask) {
	if (halfMask == NULL)
		return ECODE_FAIL;
	s_2pt web_U = hexU[i_web];
	if (halfMask->getWidth() > 1 || halfMask->getHeight() > 1)
		halfMask->release();
	halfMask->init(*m_hex_mask);
	for (long i_row = 0; i_row<m_hex_mask->getHeight(); i_row++) {
		for (long i_col = 0; i_col<m_hex_mask->getWidth(); i_col++) {
			long cur_index = i_row * m_hex_mask->getWidth() + i_col;
			s_2pt cur_loc = { (float)i_col, (float)i_row };
			s_2pt hex_mask_center = { (float)m_hex_mask_center.x0, (float)m_hex_mask_center.x1 };
			s_2pt pt = vecMath::v12(hex_mask_center, cur_loc);
			if (vecMath::dot(web_U, pt) < 0.f)
				halfMask->SetRGB(i_col, i_row, RENDERBASE_rgb_empty);
		}
	}
	return ECODE_OK;
}