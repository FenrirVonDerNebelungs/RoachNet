#include "Test.h"

unsigned char Test::init() {
	m_tgaImg = new CTargaImage();
	m_tgaImg->Init();
	m_tgaImgOut = new CTargaImage();
	m_tgaImgOut->Init();

	m_eyeBaseImg = new Img();
	m_hexImg = new HexImg();
	m_Eye = new Eye();
	m_imgHexedPlate = new s_rtHexPlate();
	m_seye = new s_Eye();
	return 1;
}