#include "TestLuna.h"

TestLuna::TestLuna() {
	m_seye = NULL;
	m_selCore = NULL;
	m_selTwistedPlate = NULL;
	m_lunaPlates = NULL;
	m_tgaImgOut = NULL;
	m_renderPlateLo = NULL;
	m_renderPlate = NULL;
}

unsigned char TestLuna::init(s_Eye* seye, CTargaImage* tgaImgOut) {
	m_seye = seye;
	m_selCore = m_seye->eyeCores[0];
	m_selTwistedPlate = m_selCore->twistedPlate;
	m_lunaPlates = m_selCore->lunaPlates;
	m_tgaImgOut = tgaImgOut;
	m_renderPlateLo = new RenderPlate;
	m_renderPlate = new RenderPlate;
	
	m_renderPlateLo->init(m_selTwistedPlate->Rhex);
	s_rgb grid_col = { 0xff, 0x00, 0x00 };
	m_renderPlateLo->setGridCol(grid_col);
	m_renderPlateLo->setGridOverlay(true);

	s_HexPlate* selLunaPlate = (m_lunaPlates->get(0));
	m_renderPlate->init(selLunaPlate->Rhex);
	s_rgb hanging_col = { 0x00, 0xff, 0x00 };
	m_renderPlate->setHangingCol(hanging_col);

	return ECODE_OK;
}
void TestLuna::release() {
	if (m_renderPlateLo != NULL) {
		m_renderPlateLo->release();
		delete m_renderPlateLo;
		m_renderPlateLo = NULL;
	}
	if (m_renderPlate != NULL) {
		m_renderPlate->release();
		delete m_renderPlate;
		m_renderPlate = NULL;
	}
	m_seye = NULL;
	m_selCore = NULL;
	m_selTwistedPlate = NULL;
	m_lunaPlates = NULL;
	m_tgaImgOut = NULL;
}

unsigned char TestLuna::genLunaPrints(Img* lunaPrints[]) {
	for (int i = 0; i < 6; i++) {
		Img* cur_img = lunaPrints[i];
		genLunaPrint(cur_img, i);
	}
	return ECODE_OK;
}
unsigned char TestLuna::genLunaPrint(Img* lunaPrint, int lunaPrint_i) {
	s_HexPlate* selLunaPlate = (m_lunaPlates->get(lunaPrint_i));
	m_renderPlateLo->spawnHexPlateImg(m_selTwistedPlate, lunaPrint);
	for (int i = 0; i < g_numTestHex; i++) {
		long test_hex_i = m_test_hex_i[i];
		m_renderPlate->renderHangingWeb(selLunaPlate, m_selTwistedPlate, test_hex_i, lunaPrint);
	}
	return ECODE_OK;
}
unsigned char TestLuna::genLunaPlates(Img* lunaPlates[]) {
	for (int i = 0; i < 6; i++) {
		Img* cur_img = lunaPlates[i];
		s_HexPlate* selLunaPlate = (m_lunaPlates->get(i));
		m_renderPlate->spawnHexPlateImg(selLunaPlate, cur_img);
	}
	return ECODE_OK;
}

unsigned char TestLuna::writeImages(Img* lunaPrints[], Img* lunaPlates[]) {
	for (int i = 0; i < 6; i++) {
		char fname[64];
		sprintf_s(fname, "luna_print_%d.tga", i);
		m_tgaImgOut->Release();
		m_tgaImgOut->Init(lunaPrints[i]->getImg(), (unsigned short)lunaPrints[i]->getWidth(), (unsigned short)lunaPrints[i]->getHeight(), false, IMAGE_RGB);
		m_tgaImgOut->Write(fname);
		sprintf_s(fname, "luna_plate_%d.tga", i);
		m_tgaImgOut->Release();
		m_tgaImgOut->Init(lunaPlates[i]->getImg(), (unsigned short)lunaPlates[i]->getWidth(), (unsigned short)lunaPlates[i]->getHeight(), false, IMAGE_RGB);
		m_tgaImgOut->Write(fname);
	}
	return ECODE_OK;
}

