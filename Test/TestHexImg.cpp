#include "TestHexImg.h"
TestHexImg::TestHexImg() :m_render(NULL), m_hexImg(NULL), m_imgHexedPlate(NULL), m_tgaImgOut(NULL) {
	utilStruct::zero2pt_i(m_img_center);
	utilStruct::zero2pt_i(m_render_center);
}
TestHexImg::~TestHexImg() {
	;
}
unsigned char TestHexImg::init(HexImg* hexImg, s_rtHexPlate* imgHexedPlate, CTargaImage* tgaImgOut) {
	if (hexImg == NULL || imgHexedPlate == NULL || tgaImgOut == NULL)
		return ECODE_FAIL;
	m_hexImg = hexImg;
	m_imgHexedPlate = imgHexedPlate;
	m_tgaImgOut = tgaImgOut;
	m_render = new RenderPlate;
	if (Err(m_render->init(hexImg->getRhex())))
		return ECODE_FAIL;
	m_render_center.x0 = 0L;
	m_render_center.x1 = 0L;
	m_img_center = { 0L, 0L };
	return ECODE_OK;
}
void TestHexImg::release() {
	if (m_render != NULL) {
		m_render->release();
		delete m_render;
		m_render = NULL;
	}
	m_hexImg = NULL;
	m_imgHexedPlate = NULL;
	m_tgaImgOut = NULL;
	utilStruct::zero2pt_i(m_img_center);
	utilStruct::zero2pt_i(m_render_center);
}
unsigned char TestHexImg::runWrite() {
	Img* hexed_img = new Img();
	Img* web_imgs[6] = { new Img(), new Img(), new Img(), new Img(), new Img(), new Img() };
	if (Err(genHexedImg(hexed_img)))
		return ECODE_FAIL;
	if (Err(genWebImgs(web_imgs)))
		return ECODE_FAIL;
	if (Err(writeImages(hexed_img, web_imgs)))
		return ECODE_FAIL;
	hexed_img->release();
	delete hexed_img;
	for (int i = 0; i < 6; i++) {
		web_imgs[i]->release();
		delete web_imgs[i];
	}
	return ECODE_OK;
}
unsigned char TestHexImg::genHexedImg(Img* hexed_img) {
	if (hexed_img == NULL)
		return ECODE_FAIL;
	if (Err(hexed_img->init(*m_hexImg->getImg())))/*init by putting the orignal image into the image to return*/
		return ECODE_FAIL;
	if (Err(m_render->RenderHexPlate(m_imgHexedPlate, m_render_center, hexed_img)))
		return ECODE_FAIL;
	return ECODE_OK;
}
unsigned char TestHexImg::genWebImgs(Img* web_img[]) {
	if (web_img == NULL)
		return ECODE_FAIL;
	if (Err(m_render->spawnWebPlateImg(m_imgHexedPlate, web_img)))
		return ECODE_FAIL;
	return ECODE_OK;
}
unsigned char TestHexImg::writeImages(Img* hexed_img, Img* web_imgs[]) {
	if (hexed_img == NULL || web_imgs == NULL)
		return ECODE_FAIL;
	m_tgaImgOut->Release();
	m_tgaImgOut->Init(hexed_img->getImg(), (unsigned short)hexed_img->getWidth(), (unsigned short)hexed_img->getHeight(), false, IMAGE_RGB);
	std::string hexedImgPath = g_baseDir + "hexedImg.tga";
	if (Err(m_tgaImgOut->Write(hexedImgPath.c_str())))
		return ECODE_FAIL;
	for (int i = 0; i < 6; i++) {
		m_tgaImgOut->Release();
		m_tgaImgOut->Init(web_imgs[i]->getImg(), (unsigned short)web_imgs[i]->getWidth(), (unsigned short)web_imgs[i]->getHeight(), false, IMAGE_RGB);
		std::string webImgPath = g_baseDir + "webImg_" + std::to_string(i) + ".tga";
		if (Err(m_tgaImgOut->Write(webImgPath.c_str())))
			return ECODE_FAIL;
	}
	return ECODE_OK;
}