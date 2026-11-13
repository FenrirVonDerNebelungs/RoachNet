#include "TestEyeRoot.h"

TestEyeRoot::TestEyeRoot() : m_loPlate(NULL), m_seye(NULL), m_eyeBaseImg(NULL), m_tgaImgOut(NULL), m_renderPlate(NULL), m_renderPlateHi(NULL) {
	;
}
TestEyeRoot::~TestEyeRoot() {
	;
}
unsigned char TestEyeRoot::init(
	s_rtHexPlate* loPlate,
	s_Eye* seye,
	Img* eyeBaseImg,
	CTargaImage* tgaImgOut
) {
	if (seye == NULL || eyeBaseImg == NULL || loPlate == NULL || tgaImgOut == NULL)
		return ECODE_FAIL;
	m_seye = seye;
	m_loPlate = loPlate;
	m_eyeBaseImg = eyeBaseImg;
	m_tgaImgOut = tgaImgOut;
	m_renderPlate = new RenderPlate;
	m_renderPlateHi = new RenderPlate;
	float base_r = m_loPlate->Rhex;
	m_renderPlate->init(base_r, 1.f);
	base_r = m_seye->rootPlate->Rhex;
	m_renderPlateHi->init(base_r, 1.f);

	s_rgb grid_col_base = { 0xFF, 0x00, 0x00 };
	m_renderPlate->setGridCol(grid_col_base);
	m_renderPlate->setGridOverlay(true);
	s_rgb grid_col_hi = { 0x00, 0xFF, 0x00 };
	m_renderPlateHi->setGridCol(grid_col_hi);
	m_renderPlateHi->setGridOverlay(true);
	return ECODE_OK;
}
void TestEyeRoot::release() {
	if (m_renderPlate != NULL) {
		m_renderPlate->release();
		delete m_renderPlate;
		m_renderPlate = NULL;
	}
	if (m_renderPlateHi != NULL) {
		m_renderPlateHi->release();
		delete m_renderPlateHi;
		m_renderPlateHi = NULL;
	}
	m_seye = NULL;
	m_loPlate = NULL;
	m_eyeBaseImg = NULL;
	m_tgaImgOut = NULL;
}
unsigned char TestEyeRoot::runWrite() {
	Img* hexed_img = new Img();
	if (Err(genHexedImg(hexed_img)))
		return ECODE_FAIL;
	if (Err(writeImage(hexed_img)))
		return ECODE_FAIL;
	hexed_img->release();
	delete hexed_img;
	return ECODE_OK;
}
unsigned char TestEyeRoot::genHexedImg(Img* hexed_img) {
	if (hexed_img == NULL)
		return ECODE_FAIL;
	if (hexed_img->init(m_eyeBaseImg->getWidth(), m_eyeBaseImg->getHeight(), m_eyeBaseImg->getColorMode()))
		return ECODE_FAIL;
	s_2pt_i center = { 0, 0 };
	m_renderPlate->RenderHexPlate(m_loPlate, center, hexed_img);
	for (long i = 0L; i < m_seye->rootPlate->N; i++){
		s_rtHex* hex = m_seye->rootPlate->get(i);
		s_rgb currgb = { 0, 0, 0 };
		imgMath::FloatsToRGB(hex->rgb, currgb);
		currgb = imgMath::mulIntensity(currgb, 0.8f);
		imgMath::RGBToFloats(currgb, hex->rgb);
	}
	m_renderPlateHi->RenderHexPlate(m_seye->rootPlate, center, hexed_img);

	return ECODE_OK;
}
unsigned char TestEyeRoot::writeImage(Img* hexed_img) {
	if (hexed_img == NULL)
		return ECODE_FAIL;
	m_tgaImgOut->Release();
	m_tgaImgOut->Init(hexed_img->getImg(), (unsigned short)hexed_img->getWidth(), (unsigned short)hexed_img->getHeight(), false, IMAGE_RGB);
	std::string filePath = g_baseDir + "TestEyeRoot.tga";
	m_tgaImgOut->Write(filePath.c_str());
	return ECODE_OK;
}