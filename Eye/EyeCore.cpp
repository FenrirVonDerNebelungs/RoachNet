#include "EyeCore.h"

s_EyeCore::s_EyeCore() : twistedPlate(NULL), lunaPlates(NULL), nets(NULL){
	;
}
s_EyeCore::~s_EyeCore() {
	;
}

unsigned char EyeCore::init(TwistedPlate* twistedplate, Luna* luna, EyeNets* genEyeNet) {
	if (twistedplate == NULL || luna == NULL || genEyeNet == NULL)
		return ECODE_ABORT;
	m_twistedPlate = twistedplate;
	m_luna = luna;
	m_genEyeNet = genEyeNet;
	return ECODE_OK;
}

unsigned char EyeCore::spawn(float angRad, s_rtHexPlate* basePlate, s_EyeCore* eyeCore) {
	if (angRad < 0.f)
		angRad = 0.f;
	if (angRad > 2.0f * PI)
		angRad = 0.f;
	if (eyeCore == NULL || basePlate==NULL)
		return ECODE_ABORT;
	eyeCore->twistedPlate = new s_rtHexPlate;
	if (Err(m_twistedPlate->spawn(eyeCore->twistedPlate, basePlate, angRad)))
		return ECODE_ABORT;
	eyeCore->lunaPlates = new s_HexPlateLayer;
	if (Err(m_luna->spawn(eyeCore->twistedPlate, eyeCore->lunaPlates)))
		return ECODE_ABORT;
	eyeCore->nets = new s_EyeNets;
	return m_genEyeNet->spawn(eyeCore->lunaPlates, eyeCore->nets);
}

bool n_EyeCore::run(s_EyeCore* eyeCore) {
	if (eyeCore == NULL)
		return false;
	if (!n_TwistedPlate::run(eyeCore->twistedPlate))
		return false;
	if (!n_Luna::run(eyeCore->lunaPlates))
		return false;
	if (!n_EyeNets::run(eyeCore->nets))
		return false;
	return true;
}