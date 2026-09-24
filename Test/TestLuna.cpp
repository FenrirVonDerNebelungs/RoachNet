#include "TestLuna_C.h"
TestLuna_C::TestLuna_C() :m_pix_between_imgs(0L), m_baseImg(NULL), m_render(NULL),
m_genHexEye(NULL), m_genHexedImg(NULL), m_genColLayer(NULL), m_genLunaLayer(NULL),
m_seye(NULL), m_colPlates(NULL), m_lunaPlates(NULL)
{
	utilStruct::zero2pt_i(m_eye_center);
	utilStruct::zero2pt(m_luna_hue_start);
	for (int i = 0; i < 6; i++)
		utilStruct::zeroRGB_f(m_luna_hues[i]);
	n_ColWheel::clear(m_sCol);
}
TestLuna_C::~TestLuna_C() {
	;
}

unsigned char TestLuna_C::init(Img* baseImg, float r, long pix_between_imgs, float luna_hue_ang, int num_eye_lev) {
	if (baseImg == NULL)
		return ECODE_FAIL;
	m_baseImg = baseImg;
	genWhiteColWheel();
	genLunaColHues(luna_hue_ang);
	m_pix_between_imgs = pix_between_imgs;


	m_genHexEye = new HexEye;
	m_genHexedImg = new HexEyeImg;
	m_genColLayer = new Col;
	m_genLunaLayer = new LunaLayer;


	m_eye_center = m_baseImg->getCenter();
	s_2pt eye_center = { Math::ltof(m_eye_center.x0), Math::ltof(m_eye_center.x1) };
	m_genHexEye->init(r, num_eye_lev);
	m_genHexedImg->init(m_baseImg, eye_center, m_genHexEye);
	m_genColLayer->init(1);/*only one color being run here*/
	m_genColLayer->addCol(&m_sCol);
	m_genLunaLayer->init(r);

	/*data objects*/
	m_seye = new s_HexEye;
	m_colPlates = new s_ColPlateLayer;
	m_lunaPlates = new s_HexPlateLayer;

	m_genHexEye->spawn(m_seye);
	m_genHexedImg->root(m_baseImg, *m_seye);
	m_genColLayer->spawn(m_seye->getBottom(), m_colPlates);
	m_genLunaLayer->spawn(m_colPlates->get(0), m_lunaPlates);


	m_render = new RenderBase;
	m_render->init(r, false);
	return ECODE_OK;
}
void TestLuna_C::release() {
	if (m_render != NULL) {
		m_render->release();
		delete m_render;
		m_render = NULL;
	}
	if (m_genLunaLayer != NULL) {
		m_genLunaLayer->despawn(m_lunaPlates);
	}
	if (m_genColLayer != NULL) {
		m_genColLayer->despawn(m_colPlates);
	}
	if (m_genHexEye != NULL)
		m_genHexEye->despawn(m_seye);
	if (m_lunaPlates != NULL)
		delete m_lunaPlates;
	m_lunaPlates = NULL;
	if (m_colPlates != NULL)
		delete m_colPlates;
	m_colPlates = NULL;
	if (m_seye != NULL)
		delete m_seye;
	m_seye = NULL;
	if (m_genLunaLayer != NULL) {
		m_genLunaLayer->release();
		delete m_genLunaLayer;
	}
	m_genLunaLayer = NULL;
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
	m_baseImg = NULL;
}
unsigned char TestLuna_C::render(Img* colPlateImg, Img* lunasImg, Img* lunaPlatesImg[]) {
	runLunaPlates();
	renderHexLunas(lunasImg);
	renderColPlate(colPlateImg);
	renderLunaPlates(lunaPlatesImg);
	return ECODE_OK;
}
unsigned char TestLuna_C::renderBase(Img* hexedImg) {
	return renderHexedBase(hexedImg);
}
unsigned char TestLuna_C::runLunaPlates() {
	m_genHexedImg->run(m_baseImg, *m_seye);
	m_genColLayer->run(m_seye->getBottom(), m_colPlates);
	m_genLunaLayer->run(m_colPlates->get(0), m_lunaPlates);
	/*bypass the col layer and just run on the hex plate*/
	//m_genLunaLayer->run(m_seye->getBottom(), m_lunaPlates);
	return ECODE_OK;
}

unsigned char TestLuna_C::renderHexLunas(Img* lunImg) {

	if (lunImg == NULL)
		return ECODE_FAIL;
	s_Luna* lunaNets = m_genLunaLayer->getLunaNets();
	int number_rendered = lunaNets->N;

	HexEye genRenderEye;
	genRenderEye.init(m_genHexEye->getBottomR(), 5);
	s_HexEye renderEye;
	genRenderEye.spawn(&renderEye);
	long rWidth = genRenderEye.getBaseWidth();
	long rHeight = genRenderEye.getBaseHeight();

	long repWidth = rWidth;
	long height = rHeight;
	height += 2L * m_pix_between_imgs;
	repWidth += m_pix_between_imgs;
	long width = m_pix_between_imgs;
	s_2pt_i* centers = new s_2pt_i[number_rendered];
	long center_half_h = height / 2L;
	long center_x = genRenderEye.getBaseWidth() / 2L + m_pix_between_imgs;
	for (int i_rot = 0; i_rot < number_rendered; i_rot++) {
		centers[i_rot] = { center_x, center_half_h };
		width += repWidth;
		center_x += repWidth;
	}

	lunImg->init(width, height, 3L);
	lunImg->clearToChar(0x00);

	n_RenderBase::setPlateIJ_toXY(renderEye.getBottom());
	n_CNets::rootOnPlate(lunaNets, renderEye.getBottom(), 0L);
	for (int i_rot = 0; i_rot < number_rendered; i_rot++) {
		s_Net* lun_net = lunaNets->net[i_rot];
		s_nPlate* net_top_plate = lun_net->getTop();
		s_nNode* top_net_nd = net_top_plate->get(0);
		s_rgb_f lun_col = { 1.f, 1.f, 1.f };
		if (i_rot < 6)
			lun_col = m_luna_hues[i_rot];
		for (int i_hex = 0; i_hex < top_net_nd->N; i_hex++) {
			float Intensity = top_net_nd->w[i_hex];
			float col_rgb[3];
			setColFromI(lun_col, Intensity, col_rgb);
			s_Hex* lowEyeBase_hex = top_net_nd->getHanging(i_hex)->hex;
			if (lowEyeBase_hex != NULL) {
				s_Hex* lowBase_hex = lowEyeBase_hex->getHanging(0);
				if (lowBase_hex != NULL)
					for (int i_col = 0; i_col < 3; i_col++)
						lowBase_hex->rgb[i_col] = col_rgb[i_col];
			}
		}
		m_render->RenderHexPlate(renderEye.getBottom(), centers[i_rot], lunImg);
	}

	delete[] centers;
	genRenderEye.despawn(&renderEye);
	genRenderEye.release();
	return ECODE_OK;
}
unsigned char TestLuna_C::renderHexedBase(Img* hexedImg) {
	if (Err(hexedImg->init(m_baseImg->getWidth(), m_baseImg->getHeight(), 3L)))
		return ECODE_FAIL;
	hexedImg->clearToChar(0x00);
	s_2pt_i zero_center = { 0L, 0L };
	return m_render->RenderHexPlate(m_seye->getBottom(), zero_center, hexedImg);
}
unsigned char TestLuna_C::renderColPlate(Img* colImg) {
	if(Err(colImg->init(m_baseImg->getWidth(), m_baseImg->getHeight(), 3L)))
		return ECODE_FAIL;
	colImg->clearToChar(0x00);
	s_2pt_i zero_center = { 0L, 0L };
	s_rgb col = { 0xff, 0xff, 0xff };
	n_RenderBase::setPlate_hex_o_toRGB(m_colPlates->get(0), col);
	return m_render->RenderHexPlate(m_colPlates->get(0), zero_center, colImg);
}
unsigned char TestLuna_C::renderLunaPlates(Img* lunPlates[]) {
	for (int i = 0; i < 6; i++) {
		int luna_i = i;//Math::loop(i+1, 6);
		s_HexPlate* lunaPlate = m_lunaPlates->get(i);
		if (Err(renderLunaPlate(luna_i, lunaPlate, lunPlates[i])))
			return ECODE_FAIL;
	}
	return ECODE_OK;
}
unsigned char TestLuna_C::renderLunaPlate(int luna_i, s_HexPlate* lunaPlate, Img* lunaPlateImg) {
	if (lunaPlateImg == NULL || m_render==NULL)
		return ECODE_FAIL;
	if (Err(lunaPlateImg->init(m_baseImg->getWidth(), m_baseImg->getHeight(), 3L)))
		return ECODE_FAIL;
	lunaPlateImg->clearToChar(0x00);
	s_2pt_i zero_center = { 0L, 0L };
	float col_rgb[3];
	setColFromI(m_luna_hues[luna_i], 1.f, col_rgb);
	s_rgb col = imgMath::convToRGB(col_rgb[0], col_rgb[1], col_rgb[2]);
	n_RenderBase::setPlate_hex_o_toRGB(lunaPlate, col);
	return m_render->RenderHexPlate(lunaPlate, zero_center, lunaPlateImg);//m_render->RenderHalfHexPlate(luna_i, lunaPlate, zero_center, lunaPlateImg);
}
unsigned char TestLuna_C::genWhiteColWheel() {
	n_ColWheel::clear(m_sCol);
	n_ColWheel::setColWheelUnitVectors(m_sCol);
	m_sCol.pixMax = 255.f;
	m_sCol.Dhue = 1.f;
	m_sCol.DI = 0.5f;
	m_sCol.DSat = 0.4f;
	m_sCol.I_target = 1.f;
	m_sCol.Hue_target = { 0.f, 0.f };
	m_sCol.Sat_target = 0.f;
	m_sCol.HueFadeV = 2.f;
	m_sCol.finalScaleFactor = 3.f;
	return ECODE_OK;
}
unsigned char TestLuna_C::genLunaColHues(float luna_hue_ang) {
	float dAng = 2.f * PI / 6.f;
	float Ang = luna_hue_ang;
	for (int i_col = 0; i_col < 6; i_col++) {
		s_2pt lun_hue = { cosf(Ang), sinf(Ang) };
		float rgb3[3];
		n_ColPlate::getRGBColoredFromHueV(m_sCol, lun_hue, rgb3);
		m_luna_hues[i_col].r = rgb3[0];
		m_luna_hues[i_col].g = rgb3[1];
		m_luna_hues[i_col].b = rgb3[2];
		Ang += dAng;
	}
	return ECODE_OK;
}
void TestLuna_C::setColFromI(const s_rgb_f& hue, float Intensity, float col_rgb[]) {
	col_rgb[0] = hue.r * RENDERBASE_pix_max * Intensity;
	col_rgb[1] = hue.g * RENDERBASE_pix_max * Intensity;
	col_rgb[2] = hue.b * RENDERBASE_pix_max * Intensity;
}