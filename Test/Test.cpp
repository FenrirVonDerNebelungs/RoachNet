#include "Test.h"

Test::Test() : 
	m_imgRHex(0.f),
	m_RHex(0.f),
	m_twisted_root_radius(0.f),
	m_sigma_hexImg(0.f),
	m_tgaImg(NULL), 
	m_tgaImgOut(NULL), 
	m_eyeBaseImg(NULL), 
	m_hexImg(NULL), 
	m_Eye(NULL), 
	m_imgHexedPlate(NULL), 
	m_seye(NULL),
	m_testHexImg(NULL),
	m_testEyeRoot(NULL),
	m_testTwisted(NULL)
{
	n_ConvolHex::clear(m_eyeBaseConvolHexMaskVars);
}
Test::~Test() {
	;
}

unsigned char Test::init( 
	float imgRHex,
	float RHex,
	float twisted_root_radius,
	float sigma_hexImg
) {
	m_imgRHex = imgRHex;
	m_RHex = RHex;
	m_twisted_root_radius = twisted_root_radius;
	m_sigma_hexImg = sigma_hexImg;

	m_tgaImg = new CTargaImage();
	m_tgaImg->Init();
	m_tgaImgOut = new CTargaImage();
	m_tgaImgOut->Init();

	m_eyeBaseImg = new Img();
	m_hexImg = new HexImg();
	m_Eye = new Eye();

	m_imgHexedPlate = new s_rtHexPlate();
	m_seye = new s_Eye();

	m_testHexImg = new TestHexImg();
	m_testEyeRoot = new TestEyeRoot();
	m_testTwisted = new TestTwisted();

	return 0;
}
unsigned char Test::runWrite() {
	if (Err(readInSourceImg()))
		return ECODE_FAIL;
	if (Err(runEye()))
		return ECODE_FAIL;

	//m_testHexImg->init(m_hexImg, m_imgHexedPlate, m_tgaImgOut);
	//m_testHexImg->runWrite();
    //m_testHexImg->release();
	//m_testEyeRoot->init(m_imgHexedPlate, m_seye, m_eyeBaseImg, m_tgaImgOut);
	//m_testEyeRoot->runWrite();
	//m_testEyeRoot->release();

	m_testTwisted->init(m_seye, m_tgaImgOut,1);
	m_testTwisted->runWrite();
	m_testTwisted->release();

	clearSource();
	return ECODE_OK;
}
void Test::release() {
	if (m_testHexImg != NULL) {
		m_testHexImg->release();
		delete m_testHexImg;
		m_testHexImg = NULL;
	}
	if (m_testEyeRoot != NULL) {
		m_testEyeRoot->release();
		delete m_testEyeRoot;
		m_testEyeRoot = NULL;
	}
	if (m_seye != NULL) {
		m_seye->release();
		delete m_seye;
		m_seye = NULL;
	}
	if (m_imgHexedPlate != NULL) {
		m_imgHexedPlate->release();
		delete m_imgHexedPlate;
		m_imgHexedPlate = NULL;
	}
	if (m_Eye != NULL) {
		m_Eye->release();
		delete m_Eye;
		m_Eye = NULL;
	}
	if (m_hexImg != NULL) {
		m_hexImg->release();
		delete m_hexImg;
		m_hexImg = NULL;
	}
	if (m_eyeBaseImg != NULL) {
		m_eyeBaseImg->release();
		delete m_eyeBaseImg;
		m_eyeBaseImg = NULL;
	}
	if (m_tgaImgOut != NULL) {
		delete m_tgaImgOut;
		m_tgaImgOut = NULL;
	}
	if (m_tgaImg != NULL) {
		delete m_tgaImg;
		m_tgaImg = NULL;
	}

	n_ConvolHex::clear(m_eyeBaseConvolHexMaskVars);
}

unsigned char Test::readInSourceImg() {
	std::string filePath = "Dat/inimgf.tga";
	unsigned char isOpen = m_tgaImg->Open(filePath.c_str());
	if (Err(isOpen)) {
		std::cout << "\n could not open " << filePath << " \n";
		return ECODE_ABORT;
	}

	unsigned char* imgData = m_tgaImg->GetImage();
	long img_width = (long)m_tgaImg->GetWidth();
	long img_height = (long)m_tgaImg->GetHeight();
	long color_mode = (long)m_tgaImg->GetColorMode();
	if (Err(m_eyeBaseImg->init(imgData, img_width, img_height, color_mode)))
		return ECODE_FAIL;
	m_hexImg->init(m_eyeBaseImg, m_imgRHex);
	m_eyeBaseConvolHexMaskVars = m_hexImg->getMask();
	m_Eye->init(m_hexImg, m_RHex, m_twisted_root_radius, m_sigma_hexImg, g_numTwistedRot, 1, g_numStackLevels);
	m_hexImg->spawn(m_imgHexedPlate);
	m_Eye->spawn(m_seye);
	n_HexBase::root(m_seye->rootPlate, m_imgHexedPlate, m_Eye->getRootCenterIndex(), m_hexImg->getCenterIndex());
	//_CrtDumpMemoryLeaks();
	return ECODE_OK;
}
void Test::clearSource() {
	m_Eye->despawn(m_seye);
	m_hexImg->despawn(m_imgHexedPlate);
	m_Eye->release();
	m_hexImg->release();
	m_eyeBaseImg->release();
	m_tgaImg->Close();
}
unsigned char Test::runEye() {
	n_HexImg::update(m_imgHexedPlate, m_eyeBaseImg, m_eyeBaseConvolHexMaskVars);
	n_Eye::run(m_seye);
	return ECODE_OK;
}