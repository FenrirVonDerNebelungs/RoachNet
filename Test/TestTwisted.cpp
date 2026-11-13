#include "TestTwisted.h"

TestTwisted::TestTwisted() :
	m_seye(NULL),
	m_tgaImgOut(NULL),
	m_renderPlateTwist(NULL),
	m_renderPlate(NULL),
	m_twisted_rot_i(0)
{
	m_twisted_shadow_i[0] = 0;
	m_twisted_shadow_i[1] = 1;
	m_twisted_shadow_i[2] = 2;
	m_twisted_shadow_i[3] = 3;
	m_twisted_shadow_i[4] = 4;
	m_twisted_shadow_i[5] = 5;
	m_twisted_shadow_i[6] = 6;
	m_twisted_shadow_i[7] = 7;
	m_twisted_shadow_i[8] = 8;
}

unsigned char TestTwisted::init(
	s_Eye* seye,
	CTargaImage* tgaImgOut,
	int twisted_i_rot
)
{
	m_seye = seye;
	m_tgaImgOut = tgaImgOut;
	m_twisted_rot_i = twisted_i_rot;
	if (m_twisted_rot_i > m_seye->N_Cores)
		m_twisted_rot_i = 0;
	float r_base = m_seye->rootPlate->Rhex;
	s_rtHexPlate* stwistedplate = m_seye->eyeCores[m_twisted_rot_i]->twistedPlate;
	float r_twisted = stwistedplate->Rhex;
	s_2pt* twistedUs = stwistedplate->hexU;
	m_renderPlateTwist = new RenderPlate;
	m_renderPlate = new RenderPlate;
	m_renderPlate->init(r_base, true);
	m_renderPlateTwist->init(r_twisted, true, 1.f, twistedUs);
	s_rgb grid_col = { 0xff, 0x00, 0x00 };
	m_renderPlateTwist->setGridCol(grid_col);
	m_renderPlateTwist->setGridOverlay(true);
	return ECODE_OK;
}
void TestTwisted::release() {
	if (m_renderPlateTwist != NULL) {
		m_renderPlateTwist->release();
		delete m_renderPlateTwist;
		m_renderPlateTwist = NULL;
	}
	if (m_renderPlate != NULL) {
		m_renderPlate->release();
		delete m_renderPlate;
		m_renderPlate = NULL;
	}
	m_seye = NULL;
	m_tgaImgOut = NULL;
}
unsigned char TestTwisted::runWrite() {
	Img* hexed_img = new Img();
	Img* shadow_imgs[g_numTwistedShadows] = { new Img(), new Img(), new Img(), new Img(), new Img(), new Img(), new Img(), new Img(), new Img() };
	if (Err(genTwistedHexImg(hexed_img)))
		return ECODE_FAIL;
	if (Err(genTwistedShadows(shadow_imgs)))
		return ECODE_FAIL;
	if (Err(writeImages(hexed_img, shadow_imgs)))
		return ECODE_FAIL;
	hexed_img->release();
	delete hexed_img;
	for (int i = 0; i < g_numTwistedShadows; i++) {
		shadow_imgs[i]->release();
		delete shadow_imgs[i];
	}
	return ECODE_OK;
}
unsigned char TestTwisted::genTwistedShadows(Img* hexed_imgs[]) {
	for (int i = 0; i < g_numTwistedShadows; i++) {
		Img* cur_img = hexed_imgs[i];
		genTwistedShadow(cur_img, i);
	}
	return ECODE_OK;
}
unsigned char TestTwisted::genTwistedShadow(Img* hexed_img, int twisted_hex_i) {
	s_rtHexPlate* base_plate = m_seye->rootPlate;
	hexed_img->init(base_plate->width, base_plate->height, 3L);
	s_EyeCore* selCore = m_seye->eyeCores[m_twisted_rot_i];
	s_rtHexPlate* twistedPlate = selCore->twistedPlate;
	int max_twisted_hex_i = twistedPlate->N-1;
	if (twisted_hex_i > max_twisted_hex_i)
		return ECODE_ABORT;
	return m_renderPlate->spawnHangingWebPlateImg(twistedPlate, base_plate, twisted_hex_i, hexed_img);
}
unsigned char TestTwisted::genTwistedHexImg(Img* hexed_img) {
	s_EyeCore* selCore = m_seye->eyeCores[m_twisted_rot_i];
	s_rtHexPlate* twistedPlate = selCore->twistedPlate;
	/*put the o into the rgb so that there is something to render*/
	for (long ii = 0; ii < twistedPlate->N; ii++) {
		s_Hex* cur_hex = twistedPlate->get(ii);
		float cur_o = cur_hex->o;
		for (int i_col = 0; i_col < 3; i_col++) {
			float scaledrgbval = cur_o * 0xFF;
			if (scaledrgbval > 0xFF)
				scaledrgbval = 0xFF;
			if (scaledrgbval < 0.f)
				scaledrgbval = 0.f;
			cur_hex->rgb[i_col] = scaledrgbval;
		}
	}
	return m_renderPlateTwist->spawnHexPlateImg(twistedPlate, hexed_img);
}

unsigned char TestTwisted::writeImages(Img* hexed_img, Img* shadow_imgs[]) {
	m_tgaImgOut->Release();
	m_tgaImgOut->Init(hexed_img->getImg(), (unsigned short)hexed_img->getWidth(), (unsigned short)hexed_img->getHeight(), false, IMAGE_RGB);
	m_tgaImgOut->Write("Dat/twisted_hex.tga");
	for (int i = 0; i < g_numTwistedShadows; i++) {
		char fname[64];
		sprintf_s(fname, "Dat/twisted_shadow_%d.tga", i);
		Img* cur_shadow_img = shadow_imgs[i];
		m_tgaImgOut->Release();
		m_tgaImgOut->Init(cur_shadow_img->getImg(), (unsigned short)cur_shadow_img->getWidth(), (unsigned short)cur_shadow_img->getHeight(), false, IMAGE_RGB);
		m_tgaImgOut->Write(fname);
	}
	return ECODE_OK;
}