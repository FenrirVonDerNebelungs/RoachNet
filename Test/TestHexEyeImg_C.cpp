#include "TestHexEyeImg_C.h"
TestHexEyeImg_C::TestHexEyeImg_C():m_render(NULL),m_genHexedImg(NULL), m_genHexEye(NULL), m_seye(NULL), m_baseImg(NULL) {
	utilStruct::zero2pt_i(m_render_center);
	utilStruct::zero2pt_i(m_img_center);
}
TestHexEyeImg_C::~TestHexEyeImg_C() {
	;
}
unsigned char TestHexEyeImg_C::init(Img* baseImg, float r, int N_eye_levels) {
	if (baseImg == NULL)
		return ECODE_FAIL;
	m_baseImg = baseImg;
	m_render = new RenderBase;
	if (Err(m_render->init(r, false)))
		return ECODE_FAIL;
	m_genHexedImg = new HexEyeImg;
	m_genHexEye = new HexEye;
	if (Err(m_genHexEye->init(r, N_eye_levels)))
		return ECODE_FAIL;
	m_render_center.x0 = 0L;
	m_render_center.x1 = 0L;
	m_img_center = baseImg->getCenter();
	s_2pt img_centerf = { (float)m_img_center.x0, (float)m_img_center.x1 };
	m_genHexedImg->init(baseImg, img_centerf, m_genHexEye);
	/*now generate the spawned stuff*/
	m_seye = new s_HexEye;
	if (Err(m_genHexEye->spawn(m_seye)))
		return ECODE_FAIL;
	if (Err(m_genHexedImg->root(baseImg, *m_seye)))
		return ECODE_FAIL;
	return ECODE_OK;
}
void TestHexEyeImg_C::release() {
	if (m_genHexEye != NULL)
		m_genHexEye->despawn(m_seye);
	if (m_seye != NULL)
		delete m_seye;
	m_seye = NULL;
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
	if (m_render != NULL) {
		m_render->release();
		delete m_render;
	}
	m_render = NULL;
	m_baseImg = NULL;
}
unsigned char TestHexEyeImg_C::genHexedImg(Img* hexed_img, int level) {
	if (hexed_img == NULL || m_baseImg==NULL)
		return ECODE_FAIL;
	if (Err(hexed_img->init(*m_baseImg)))/*init by putting the orignal image into the image to return*/
		return ECODE_FAIL;
	if (Err(m_genHexedImg->run(m_baseImg, *m_seye)))
		return ECODE_FAIL;
	/*draw the hexed image on the original image*/
	if(level<0)
		return m_render->RenderHexPlate(m_seye->getBottom(), m_render_center, hexed_img);
	for (int i_eye_lev = (m_seye->N - 1); i_eye_lev >= level; i_eye_lev--) {
		float level_r = m_seye->get(i_eye_lev)->Rhex;
		m_render->resetR(level_r);
		if (Err(m_render->RenderHexPlate(m_seye->get(i_eye_lev), m_render_center, hexed_img)))
			return ECODE_FAIL;
	}
	return ECODE_OK;
}