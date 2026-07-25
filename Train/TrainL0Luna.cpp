#include "TrainL0Luna.h"

unsigned char TrainL0Luna::init(long img_width, long img_height, float r_scale) {
	m_dimImg = new Img;
	m_dimImg->init(img_width, img_height, 3L);
	m_genHexEye = new HexEye;
	m_genEyeImg = new HexEyeImg;
	m_genHexEye->init(3);
	float half_width = ((float)img_width) / 2.f;
	float half_height = ((float)img_height / 2.f);
	s_2pt eye_center = (half_width, half_height);
	m_genEyeImg->init(m_dimImg, eye_center, m_genHexEye);

	m_genColPlates = new Col;
	m_genColPlates->init(1);
	s_ColWheel* whiteCol = new s_ColWheel;
	genWhiteColWheel(whiteCol);
	m_genColPlates->addCol(whiteCol);
	delete whiteCol;/*not needed after object takes it*/

	m_genLuna = new LunaLayer;
	m_genLuna->init(r_scale);
	m_genL0->init(m_genLuna,1)/*for training purposes only need one L0 net*/
	return spawnDataObjs();
}
unsigned char TrainL0Luna::spawnDataObjs() {
	m_eye = new s_HexEye;
	m_colPlates = new s_ColPlateLayer;
	m_lunaPlates = new s_HexPlateLayer;
	m_genHexEye->spawn(m_eye);
	m_genEyeImg->root(m_dimImg, *m_eye);
	s_HexPlate* eyeBotHexed = m_eye->getBottom();
	m_genColPlates->spawn(eyeBotHexed, m_colPlates);
	m_genLuna->spawn(m_colPlates->get(0), m_lunaPlates);
	m_genL0->spawn(m_lunaPlates, m_L0Plates);
}
unsigned char TrainL0Luna::update(Img* baseImg) {
	if (baseImg == NULL)
		return ECODE_ABORT;
	if (!m_dimImg->sameKind(*baseImg))
		return ECODE_ABORT;
	m_Img = baseImg;
	/*since the diming and the cur img have the same dim the rooting of the eye for the dim also works here*/
	return ECODE_OK;
}
unsigned char TrainL0Luna::run() {
	if(Err(m_genEyeImg->run(m_Img, *m_eye)))/*rooting stays same as for baseImg since all imgs have the same dimensions*/
		return ECODE_FAIL;
	if (Err(m_genColPlates->run(m_eye->getBottom(), m_colPlates)))
		return ECODE_FAIL;
	if (Err(m_genLuna->run(m_colPlates->get(0), m_lunaPlates)))
		return ECODE_FAIL;
	if (Err(m_genL0->run(m_lunaPlates, m_L0Plates)))
		return ECODE_FAIL;
	return ECODE_OK;
}