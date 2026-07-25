#include "TestHexEye_C.h"
TestHexEye_C::TestHexEye_C() :m_genEye(NULL), m_render(NULL), m_seye(NULL), m_intensity(0.f), m_hue_start_rad(0.f),
m_hue_Drad(0.f), m_shortest_rgb(0.f), m_pix_between_lev_imgs(0)
{
	n_ColWheel::clear(m_colWheel);
}
TestHexEye_C::~TestHexEye_C() {
	;
}
unsigned char TestHexEye_C::init(float r, int NLevels, float intensity, 
	float hue_start_rad, float hue_Drad, float shortest_rgb, 
	long pix_between_lev_imgs) {
	m_genEye = new HexEye;
	m_genEye->init(r, NLevels);
	m_render = new RenderBase;
	m_render->init(r,true);

	m_seye = new s_HexEye;
	m_genEye->spawn(m_seye);

	n_ColWheel::clear(m_colWheel);
	n_ColWheel::setColWheelUnitVectors(m_colWheel);
	m_intensity = intensity;
	m_hue_start_rad = hue_start_rad;
	m_hue_Drad = hue_Drad;
	m_shortest_rgb = shortest_rgb;
	m_pix_between_lev_imgs = pix_between_lev_imgs;
	return ECODE_OK;
}
void TestHexEye_C::release() {
	if (m_genEye != NULL) {
		m_genEye->despawn(m_seye);
		if (m_seye != NULL)
			delete m_seye;
		m_seye = NULL;
		m_genEye->release();
		delete m_genEye;
	}
	m_genEye = NULL;
	if (m_render != NULL) {
		m_render->release();
		delete m_render;
	}
	m_render = NULL;
}
unsigned char TestHexEye_C::runTestMasks(Img** test_img) {
	(*test_img) = m_render->getHexGridMask();
	return ECODE_OK;
}
unsigned char TestHexEye_C::runTestHex(Img* test_hex) {
	test_hex->init(30L, 30L, 3L);
	s_Hex thex;
	thex.i = 15L;
	thex.j = 15L;
	thex.rgb[0] = 1.f;
	thex.rgb[1] = 0.2f;
	thex.rgb[2] = 0.7f;
	s_2pt_i center = { 0L, 0L };
	return m_render->RenderHex(&thex, center, test_hex);
}
unsigned char TestHexEye_C::run(Img* retImg_levels, Img* retImg_web, Img* retImg_lower) {
	if (retImg_levels == NULL || retImg_web==NULL || retImg_lower==NULL)
		return ECODE_ABORT;
	if (retImg_levels->getWidth() > 0 || retImg_web->getWidth()>0 || retImg_lower->getWidth()>0)
		return ECODE_ABORT;
	drawEyeLevels(m_seye, retImg_levels);
	drawAllWebLinksForLevel(m_seye->getBottom(), retImg_web);
	drawLevelAndLowerLinks(m_seye, (m_seye->N - 2), retImg_lower);
	return ECODE_OK;
}
unsigned char TestHexEye_C::drawEyeLevel(s_HexEye* seye, int eye_lev, s_2pt& lev_center, Img* lev_img) {
	float level_r = seye->get(eye_lev)->Rhex;
	m_render->resetR(level_r);
	return drawColorRotLevel(seye->get(eye_lev), lev_center, lev_img);
}
unsigned char TestHexEye_C::drawEyeLevels(s_HexEye* seye,  Img* lev_img) {
	if (lev_img == NULL)
		return ECODE_FAIL;
	if (lev_img->getHeight() > 0)
		return ECODE_ABORT;
	/*find culmulative width and height */
	long width = m_pix_between_lev_imgs;
	long height = m_pix_between_lev_imgs;
	float fpix_spacer = (float)m_pix_between_lev_imgs;
	s_2pt* centers = new s_2pt[seye->N];
	for (int i_lev = 0; i_lev < seye->N; i_lev++) {
		s_HexPlate* lev = seye->get(i_lev);
		float fwidth = (float)lev->width;
		fwidth /= 2.f;
		centers[i_lev].x0 = fwidth + (float)width;
		width += lev->width;
		width += m_pix_between_lev_imgs;
		if ((lev->height + 2 * m_pix_between_lev_imgs) >= height)
			height = lev->height + 2 * m_pix_between_lev_imgs;

	}
	for (int i_lev = 0; i_lev < seye->N; i_lev++)
		centers[i_lev].x1 = ((float)height) / 2.f;
	unsigned char err = lev_img->init(width, height, 3L);
	if (Err(err))
		return err;
	lev_img->clearToChar(RENDERBASE_rgb_empty.r);


	for (int i_lev = 0; i_lev < seye->N; i_lev++)
		if (Err(drawEyeLevel(seye, i_lev, centers[i_lev], lev_img))) {
			delete[] centers;
			return ECODE_FAIL;
		}

	delete[] centers;
	return ECODE_OK;
}

unsigned char TestHexEye_C::drawWebLinkForLevel(s_HexPlate* lev, s_2pt& lev_center, int web_link_i, Img* lev_img) {
	/*draw the level then draw the web links on it*/
	if (Err(fillHexLevelWithColorRot(lev)))
		return ECODE_FAIL;
	if (Err(drawColorRotLevel(lev, lev_center, lev_img)))
		return ECODE_FAIL;
	if (Err(drawWebLinksOnLevel(lev, lev_center, web_link_i, lev_img)))
		return ECODE_FAIL;
	return ECODE_OK;
}
unsigned char TestHexEye_C::drawAllWebLinksForLevel(s_HexPlate* lev, Img* lev_img) {
	m_render->resetR(lev->Rhex);
	long width = m_pix_between_lev_imgs + lev->width;
	long height = 2*m_pix_between_lev_imgs + lev->height;
	width *= 6;
	width += m_pix_between_lev_imgs;
	if (Err(lev_img->init(width, height, 3L)))
		return ECODE_FAIL;
	lev_img->clearToChar(RENDERBASE_rgb_empty.r);
	s_2pt lev_center;
	lev_center.x0 = ((float)m_pix_between_lev_imgs) + ((float)lev->width) / 2.f;
	lev_center.x1 = ((float)height) / 2.f;
	for (int i_web = 0; i_web < 6; i_web++) {
		if (Err(drawWebLinkForLevel(lev, lev_center, i_web, lev_img)))
			return ECODE_FAIL;
		lev_center.x0 += (float)(lev->width + m_pix_between_lev_imgs);
	}
	return ECODE_OK;
}
unsigned char TestHexEye_C::drawLevelAndLowerLinks(s_HexEye* seye, int eye_lev, Img* lev_img){
	int lev_down = eye_lev + 1;
	if (lev_down >= seye->N || eye_lev < 0)
		return ECODE_FAIL;
	s_HexPlate* lev = seye->get(eye_lev);
	long N_upper_hexes = lev->N;
	s_HexPlate* lower_lev = seye->get(lev_down);
	long width = m_pix_between_lev_imgs + lower_lev->width;
	float halflev_widthf = Math::ltof(lower_lev->width) / 2.f;
	long halflev_width = Math::ftol(halflev_widthf);
	width *= N_upper_hexes;
	width += m_pix_between_lev_imgs;
	long height = 2 * m_pix_between_lev_imgs + lower_lev->height;
	unsigned char err = lev_img->init(width, height, 3L);
	if (Err(err))
		return err;
	lev_img->clearToChar(RENDERBASE_rgb_empty.r);

	float fheight_2 = Math::ltof(height);
	fheight_2 /= 2.f;
	float fwidth = Math::ltof(m_pix_between_lev_imgs+lower_lev->width);
	s_2pt lev_center = { Math::ltof(m_pix_between_lev_imgs + halflev_width), fheight_2 };
	for (int i_top = 0; i_top < N_upper_hexes; i_top++) {
		drawUpperHexAndLowerLinksWithBackground(seye, eye_lev, i_top, lev_center, lev_img);
		lev_center.x0 += fwidth;
	}
	return ECODE_OK;
}
unsigned char TestHexEye_C::drawUpperHexAndLowerLinksWithBackground(s_HexEye* seye, int eye_lev, long lev_hex_i, s_2pt& lev_center, Img* lev_img) {
	int lev_down = eye_lev + 1;
	if (lev_down >= seye->N || eye_lev<0)
		return ECODE_FAIL;
	s_HexPlate* lower_lev = seye->get(lev_down);
	m_intensity = 0.4f;
	drawColorRotLevel(lower_lev, lev_center, lev_img);
	m_intensity = 0.9f;
	fillHexLevelWithColorRot(lower_lev);
	s_HexPlate* upper_lev = seye->get(eye_lev);
	drawLevelHexAndLowerLinks(upper_lev, lev_hex_i, lev_center, lev_img);
	return ECODE_OK;
}
unsigned char TestHexEye_C::drawLevelHexAndLowerLinks(s_HexPlate* lev, long lev_hex_i, s_2pt& lev_center, Img* lev_img) {
	s_Hex* upper_hex = lev->get(lev_hex_i);
	s_2pt_i center = { Math::ftol(lev_center.x0), Math::ftol(lev_center.x1) };
	for (int i_lower = 0; i_lower < upper_hex->N; i_lower++) {
		s_Hex* lower_hex = (s_Hex*)upper_hex->nodes[i_lower];
		if (lower_hex != NULL) {
			m_render->RenderHex(lower_hex, center, lev_img);
		}
	}
	s_rgb hex_grid_col = { 0xfe, 0xdd, 0xff };
	m_render->setGridCol(hex_grid_col);
	m_render->RenderHex(upper_hex, center, lev_img, true, false);
	return ECODE_OK;
}
unsigned char TestHexEye_C::fillHexLevelWithColorRot(s_HexPlate* lev){
	s_ColWheel_Col col;
	col.shortest_rgb = m_shortest_rgb;
	col.Intensity = m_intensity;
	s_2pt Uhue;
	float hueAng = m_hue_start_rad;
	for (long i = 0; i < lev->N; i++) {
		hueAng = Math::Ang2PI(hueAng);
		Uhue.x0 = cosf(hueAng);
		Uhue.x1 = sinf(hueAng);
		col.hue = Uhue;
		float rgb[3];
		n_ColPlate::setRGBFromColWheel(m_colWheel, col, rgb);
		s_Hex* plate_hex = lev->get(i);
		for (int col_i = 0; col_i < 3; col_i++)
			plate_hex->rgb[col_i] = rgb[col_i];
		hueAng += m_hue_Drad;
	}
	return ECODE_OK;
}
unsigned char TestHexEye_C::setIJForLevelToRelative(s_HexPlate* lev) {
	for (long ii = 0; ii < lev->N; ii++) {
		s_Hex* cur_hex = lev->get(ii);
		cur_hex->i = Math::ftol(cur_hex->x);
		cur_hex->j = Math::ftol(cur_hex->y);
	}
	return ECODE_OK;
}
unsigned char TestHexEye_C::drawColorRotLevel(s_HexPlate* lev, s_2pt& lev_center, Img* lev_img) {
	unsigned char err = fillHexLevelWithColorRot(lev);
	if (Err(err))
		return err;
	if (Err(setIJForLevelToRelative(lev)))
		return ECODE_FAIL;
	s_2pt_i center = { Math::ftol(lev_center.x0), Math::ftol(lev_center.x1) };
	err = m_render->RenderHexPlate(lev, center, lev_img);
	return err;
}
unsigned char TestHexEye_C::drawWebLinksOnLevel(s_HexPlate* lev, s_2pt& lev_center, int web_link_i, Img* lev_img) {
	s_rgb rgb_start = { 0xaa, 0xff, 0xff };
	s_rgb rgb_end = { 0x11, 0x00, 0x00 };
	s_2pt_i center = { Math::ftol(lev_center.x0), Math::ftol(lev_center.x1) };
	for (long i_hex = 0; i_hex < lev->N; i_hex++) {
		s_Hex* chex = lev->get(i_hex);
		s_Hex* nhex= (s_Hex*)chex->web[web_link_i];
		if (nhex != NULL) {
			s_2pt_i ij_start = { chex->i, chex->j };
			s_2pt_i ij_end = { nhex->i, nhex->j };
			s_2pt_i ij_s = vecMath::add(ij_start, center);
			s_2pt_i ij_e = vecMath::add(ij_end, center);
			lev_img->DrawLineGrad(ij_s, ij_e, rgb_start, rgb_end);
		}
	}
	return ECODE_OK;
}