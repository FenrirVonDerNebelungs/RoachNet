#include "Luna.h"


unsigned char s_Luna::init(const s_Luna& other) {
	unsigned char err = s_CNets::init(other);
	if (err != ECODE_OK)
		return err;
	this->m_lunaEye = new s_HexEye;/*luna owns this type of eye */
	if (this->m_lunaEye == NULL)
		return ECODE_FAIL;
	err = this->m_lunaEye->init(*other.m_lunaEye);
	this->eye = m_lunaEye;
	for (int ii = 0; ii < N; ii++) {
		net[ii]->eye = this->eye;
	}
	return err;
}
void s_Luna::release() {
	s_CNets::release();
	if (m_lunaEye != NULL) {
		m_lunaEye->release();
		delete m_lunaEye;
	}
	m_lunaEye = NULL;
}
void s_Luna::releaseOwnedEye() {
	if (m_lunaEye != NULL) {
		m_lunaEye->release(); 
		delete m_lunaEye; 
	}
	m_lunaEye = NULL;
	eye = NULL;
}
bool n_Luna::run(s_Luna* lun, s_HexPlate* colPlate, s_HexPlateLayer* lunPlates, long plate_index) {
	bool locRootGood = n_CNets::runNNet((s_CNets*)lun, colPlate, plate_index);/*returns false it net can't root at index*/
	if (locRootGood) {
		for (int i_luna = 0; i_luna < lun->N; i_luna++) {
			s_Hex* lunPlate_hex = lunPlates->get(i_luna)->get(plate_index);
			lunPlate_hex->o = lun->net[i_luna]->o;
		}
	}
	return locRootGood;
}

Luna::Luna():m_NetsMaster(NULL), m_EyeMaster(NULL){ ; }
Luna::~Luna() { ; }

unsigned char Luna::init(float r) {
	m_NetsMaster = new CNets;
	if (m_NetsMaster == NULL)
		return ECODE_FAIL;
	m_EyeMaster = new HexEye;
	if (m_EyeMaster == NULL)
		return ECODE_FAIL;
	unsigned char err = ECODE_OK;
	err = m_EyeMaster->init(r, 2);
	if (Err(err))
		return err;
	err = m_NetsMaster->init(LUNA_NUM_LUNAS, m_EyeMaster, 1);
	return err;
}
void Luna::release() {
	if (m_NetsMaster != NULL) {
		m_NetsMaster->release();
		delete m_NetsMaster;
	}
	m_NetsMaster = NULL;
	if (m_EyeMaster != NULL) {
		m_EyeMaster->release();
		delete m_EyeMaster;
	}
	m_EyeMaster = NULL;
}
unsigned char Luna::spawn(s_Luna* lun) {
	if (lun == NULL)
		return ECODE_ABORT;
	s_HexEye* luna_hexEye = new s_HexEye;
	unsigned char err = m_EyeMaster->spawn(luna_hexEye);
	if (Err(err))
		return ECODE_FAIL;
	err = m_NetsMaster->spawn((s_CNets*)lun, luna_hexEye);
	if (Err(err))
		return ECODE_FAIL;
	err = lun->setOwnedEye(luna_hexEye);/*transfers ownership of this object to the luna*/
	if (Err(err))
		return ECODE_FAIL;

	/*finished setting up the structure */
	/* now fill the w's so that the luna generate the correct patterns*/
	return genLunaPatterns(lun);
}
void Luna::despawn(s_Luna* lun) {
	if (lun == NULL)
		return;
	m_NetsMaster->despawn((s_CNets*)lun);
	lun->releaseOwnedEye();
}


unsigned char Luna::genLunaPatterns(s_Luna* lun) {
	/*since this hexEye has only one level, the indexes of the nodes hanging from the top node should be the same as their indexes in the bottom plate*/
	for (int i_rot = 0; i_rot < 6; i_rot++) {
		genHalfLunaPattern(i_rot, lun->net[i_rot]->getTop()->get(0));
	}
	/*create full white/black patterns*/
	/* currently don't do
	float patWeight = LUNA_WSCALE / ((float)NUM_LUNA_FOOTS);
	s_nNode* top_white_moon = lun->net[6]->getTop()->get(0);
	s_nNode* top_black_moon = lun->net[7]->getTop()->get(0);
	for (int i_ft = 0; i_ft < NUM_LUNA_FOOTS; i_ft++) {
		top_white_moon->w[i_ft] = patWeight;
		top_black_moon->w[i_ft] = -patWeight;
	}
	top_black_moon->N = NUM_LUNA_FOOTS;
	top_white_moon->N = NUM_LUNA_FOOTS;
	*/

	/*fix the base that will root 1 down on the actual colored plate*/
	for (int i_net = 0; i_net < 6; i_net++) {
		s_Net* luna_pat = lun->get(i_net);
		s_nPlate* lunbott = luna_pat->getBottom();
		for (long i_nd = 0; i_nd < lunbott->N; i_nd++) {
			s_nNode* nd = lunbott->get(i_nd);
			nd->N = 1; /*just set to 1 since will aways be set to 1 when attached*/
			nd->w[0] = 1.f;
		}
	}
	return ECODE_OK;

}

void Luna::genHalfLunaPattern(int lunRot, s_nNode* topNd) {
	int arcLen = 3;
	float posFracWeight = 1.f / (3.5f);
	float negFracWeight = -0.5f;//-3.5f / 2.f;//-1.f / arcLen;
	for (int i = 0; i < LUNA_NUM_FOOTS; i++)
		topNd->w[i] = negFracWeight;
	for (int i = 0; i <= arcLen; i++) {
		int i_Nd = Math::loop(lunRot + i, 6);
		i_Nd++;/*center node is 0, arc nodes are 1->6*/
		topNd->w[i_Nd] = posFracWeight;
	}
	int i_line;
	i_line = 0;
	float lineWeight = 0.5f / 3.5f;
	topNd->w[i_line] = lineWeight;
	i_line = Math::loop(lunRot, 6);
	topNd->w[i_line+1] = lineWeight;
	i_line = Math::loop(lunRot + 3, 6);
	topNd->w[i_line+1] = lineWeight;

	for (int i = 0; i < LUNA_NUM_FOOTS; i++)
		topNd->w[i]= LUNA_WSCALE * (topNd->w[i]);
	topNd->N = LUNA_NUM_FOOTS;
}