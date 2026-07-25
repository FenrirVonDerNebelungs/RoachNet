#include "TestColor_C.h"

TestColor_C::TestColor_C() :m_pix_between_imgs(0L), m_baseImg(NULL), m_render(NULL), m_genHexEye(NULL), m_genHexedImg(NULL), m_genColLayer(NULL), m_seye(NULL), m_colPlates(NULL),
m_cols(NULL), m_n_cols(0) {
	utilStruct::zero2pt_i(m_eye_center);
}
TestColor_C::~TestColor_C() {
	;
}
unsigned char TestColor_C::init(
	Img* baseImg,
	float r,
	int num_eye_lev,
	int num_cols,
	float Dhue,
	float DI,
	float DSat,
	float I_target,
	float Hue_targ_0,
	float Hue_targ_1,
	float Sat_target,
	float HueFadeV,
	float scale_factor_for_col_stepfunc,
	long pix_between_imgs
) {
	if (m_n_cols < 0)
		return ECODE_FAIL;
	m_n_cols = num_cols;
	m_n_cols += 1;
	if (baseImg == NULL)
		return ECODE_FAIL;
	m_baseImg = baseImg;
	m_cols = new s_ColWheel[m_n_cols];
	n_ColWheel::clear(m_cols[0]);
	n_ColWheel::setColWheelUnitVectors(m_cols[0]);
	m_cols[0].pixMax = TestColor_C_pixMax;
	m_cols[0].Dhue = Dhue;
	m_cols[0].DI = DI;
	m_cols[0].DSat = DSat;
	m_cols[0].I_target = I_target;
	m_cols[0].Hue_target = { Hue_targ_0, Hue_targ_1 };
	m_cols[0].Sat_target = Sat_target;
	m_cols[0].HueFadeV = HueFadeV;
	m_cols[0].finalScaleFactor = scale_factor_for_col_stepfunc;
	genColWheels();
	m_pix_between_imgs = pix_between_imgs;

	m_genHexEye = new HexEye;
	m_genHexedImg = new HexEyeImg;
	m_genColLayer = new Col;

	m_eye_center = m_baseImg->getCenter();
	s_2pt eye_center = { Math::ltof(m_eye_center.x0), Math::ltof(m_eye_center.x1) };
	m_genHexEye->init(r, num_eye_lev);
	m_genHexedImg->init(m_baseImg, eye_center, m_genHexEye);
	m_genColLayer->init(m_n_cols);/*only one color being run here*/
	for(int i_col=0; i_col <m_n_cols; i_col++)
		m_genColLayer->addCol(&m_cols[i_col]);/*add the color*/

	m_seye = new s_HexEye;
	m_colPlates = new s_ColPlateLayer;
	m_genHexEye->spawn(m_seye);
	m_genHexedImg->root(m_baseImg, *m_seye);
	m_genColLayer->spawn(m_seye->getBottom(), m_colPlates);
	
	m_render = new RenderBase;
	m_render->init(r,false);
	return ECODE_OK;
}
void TestColor_C::release() {
	if (m_render != NULL) {
		m_render->release();
		delete m_render;
	}
	m_render = NULL;
	if (m_genColLayer != NULL) {
		m_genColLayer->despawn(m_colPlates);
	}
	if (m_genHexEye != NULL) {
		m_genHexEye->despawn(m_seye);
	}
	if (m_colPlates != NULL)
		delete m_colPlates;
	m_colPlates = NULL;
	if (m_seye != NULL)
		delete m_seye;
	m_seye = NULL;
	if (m_genColLayer != NULL) {
		m_genColLayer->release();
		delete m_genColLayer;
	}
	m_genColLayer = NULL;
	if (m_genHexedImg != NULL) {
		m_genHexedImg->release();
		delete m_genHexedImg;
	}
	m_genHexedImg = NULL;
	if (m_genHexEye != NULL) {
		m_genHexEye->release();
		delete m_genHexEye;
	}
	m_genHexEye = NULL;
	if (m_cols != NULL)
		delete[] m_cols;
	m_baseImg = NULL;
}
unsigned char TestColor_C::genColWheels() {
	for (int i_col = 1; i_col < m_n_cols; i_col++) {
		n_ColWheel::copy(m_cols[i_col], m_cols[0]);
	}
	m_cols[0].DI = 0.5f;
	m_cols[0].I_target = 1.0f;
	m_cols[0].Sat_target = 0.0f;
	m_cols[0].HueFadeV = 2.f;/*don't consider hue for this*/

	float num_colored_cols = (float)m_n_cols;
	num_colored_cols -= 1.f;
	if (num_colored_cols > 0.5f) {
		float dAng = 2.f * PI /num_colored_cols;
		float Ang = 0.f;
		for (int i_col = 1; i_col < m_n_cols; i_col++) {
			m_cols[i_col].Hue_target = { cosf(Ang), sinf(Ang) };
			Ang += dAng;
		}
	}
	return ECODE_OK;
}
unsigned char TestColor_C::render(Img** imgs) {
	for (int i_img = 0; i_img < m_n_cols; i_img++) {
		imgs[i_img]->init(m_baseImg->getWidth(), m_baseImg->getHeight(), 3L);
		imgs[i_img]->clearToChar(0x00);
	}
	m_genHexedImg->run(m_baseImg, *m_seye);
	m_genColLayer->run(m_seye->getBottom(), m_colPlates);
	s_2pt_i zero_center = { 0L, 0L };/*the hexes already contain their correct coords in image space don't need an offset*/
	for (int i_img = 0; i_img < m_n_cols; i_img++) {
		fillColPlateWithColor(m_colPlates->get(i_img));
		m_render->RenderHexPlate(m_colPlates->get(i_img), zero_center, imgs[i_img]);
	}
	return ECODE_OK;
}
unsigned char TestColor_C::fillColPlateWithColor(s_ColPlate* colorPlate) {
	float rgb[3];
	if (!n_ColPlate::getRGBFromColWheel(colorPlate->Col, rgb))
		return ECODE_FAIL;
	for (int i_col = 0; i_col < 3; i_col++)
		rgb[i_col] *= RENDERBASE_pix_max;
	s_rgb rgbs;
	imgMath::FloatsToRGB(rgb, rgbs);

	for (long hex_i = 0; hex_i < colorPlate->N; hex_i++) {
		s_Hex* chex = colorPlate->get(hex_i);
		s_rgb rgb_int = imgMath::mulIntensity(rgbs, chex->o);
		float rgb_hex[3];
		imgMath::RGBToFloats(rgb_int, rgb_hex);
		for (int i_r = 0; i_r < 3; i_r++)
			chex->rgb[i_r] = rgb_hex[i_r];
	}
	return ECODE_OK;
}