#include "LunaLayer.h"

LunaLayer::LunaLayer() :m_genLuna(NULL), m_lunaNets(NULL) {
	;
}
LunaLayer::~LunaLayer() {
	;
}

unsigned char LunaLayer::init(float r) {
	m_genLuna = new Luna;
	if (Err(m_genLuna->init(r)))
		return ECODE_FAIL;
	m_lunaNets = new s_Luna;
	if (Err(m_genLuna->spawn(m_lunaNets)))
		return ECODE_FAIL;
	return ECODE_OK;
}
void LunaLayer::release() {
	if (m_genLuna != NULL) {
		if (m_lunaNets != NULL) {
			m_genLuna->despawn(m_lunaNets);
			delete m_lunaNets;
		}
		m_lunaNets = NULL;
		m_genLuna->release();
		delete m_genLuna;
	}
	m_genLuna = NULL;
}
unsigned char LunaLayer::spawn(s_HexPlate* colPlate, s_HexPlateLayer* lunaPlates) {
	if (m_genLuna == NULL || m_lunaNets == NULL)
		return ECODE_ABORT;
	int num_lunas = m_lunaNets->N;
	if (num_lunas < 1)
		return ECODE_ABORT;
	if (Err(lunaPlates->init(num_lunas)))
		return ECODE_FAIL;
	lunaPlates->N = 0;
	for (int ii=0; ii < num_lunas; ii++) {
		lunaPlates->p[ii] = new s_HexPlate;
		if (lunaPlates->p[ii] == NULL)
			return ECODE_FAIL;
		if (Err(lunaPlates->p[ii]->initFixDownTarget(colPlate)))
			return ECODE_FAIL;
		lunaPlates->N++;
	}
	return ECODE_OK;
}
void LunaLayer::despawn(s_HexPlateLayer* lunaPlates) {
	if (lunaPlates != NULL) {
		for (int ii = 0; ii < lunaPlates->N; ii++) {
			if (lunaPlates->p[ii] != NULL) {
				lunaPlates->p[ii]->release();
				delete lunaPlates->p[ii];
			}
			lunaPlates->p[ii] = NULL;
		}
		lunaPlates->N = 0;
		lunaPlates->release();
	}
}

unsigned char LunaLayer::run(s_HexPlate* colPlate, s_HexPlateLayer* lunaPlates) {
	long num_hex = colPlate->N;
	for (long i_hex = 0; i_hex < num_hex; i_hex++) {
		n_LunaLayer::run(this, colPlate, lunaPlates, i_hex);
	}
	return ECODE_OK;
}

bool n_LunaLayer::run(LunaLayer* genLuna, s_HexPlate* colPlate, s_HexPlateLayer* lunaPlates, long plate_i) {
	if (genLuna == NULL)
		return false;
	s_CNets* lunas = (s_CNets*)genLuna->getLunaNets();
	if (!n_CNets::runNNet(lunas, colPlate, plate_i))
		return false;
	for (int i_luna = 0; i_luna < lunas->N; i_luna++) {
		float luna_val = lunas->net[i_luna]->o;
		s_HexPlate* luna_plate = lunaPlates->get(i_luna);
		luna_plate->nodes[plate_i]->o = luna_val;
	}
	return true;
}