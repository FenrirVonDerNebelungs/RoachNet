#include "EyeCore.h"

s_EyeCore::s_EyeCore() : twistedPlate(NULL), lunaPlates(NULL), nets(NULL){
	;
}
s_EyeCore::~s_EyeCore() {
	;
}

EyeCore::EyeCore() : m_twistedPlate(NULL), m_luna(NULL), m_genEyeNet(NULL) {
	;
}
EyeCore::~EyeCore() {
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
void EyeCore::release() {
	m_twistedPlate = NULL;
	m_luna = NULL;
	m_genEyeNet = NULL;
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
void EyeCore::despawn(s_EyeCore* eyeCore) {
	if (eyeCore == NULL)
		return;
	if (eyeCore->nets != NULL) {
		m_genEyeNet->despawn(eyeCore->nets);
		delete eyeCore->nets;
		eyeCore->nets = NULL;
	}
	if (eyeCore->lunaPlates != NULL) {
		m_luna->despawn(eyeCore->lunaPlates);
		delete eyeCore->lunaPlates;
		eyeCore->lunaPlates = NULL;
	}
	if (eyeCore->twistedPlate != NULL) {
		m_twistedPlate->despawn(eyeCore->twistedPlate);
		delete eyeCore->twistedPlate;
		eyeCore->twistedPlate = NULL;
	}
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