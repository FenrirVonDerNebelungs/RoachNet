#include "Luna.h"

Luna::Luna():m_exHex(NULL){ ; }
Luna::~Luna() { ; }

unsigned char Luna::init(ExHex* exHex) {
	unsigned char err = ECODE_OK;
	if(exHex==NULL)
		return ECODE_ABORT;
	err = genLunaPatterns();
	if (Err(err))
		return err;
	m_exHex = exHex;
	return err;
}
void Luna::release() {
}
unsigned char Luna::spawn(/*const*/s_HexPlate* basePlate, s_HexPlateLayer* lunPlates) {
	if (lunPlates == NULL)
		return ECODE_ABORT;
	if(Err(lunPlates->init(LUNA_NUM_LUNAS)))
		return ECODE_FAIL;
	for (int i_luna = 0; i_luna < LUNA_NUM_LUNAS; i_luna++) {
		s_HexPlate* lunPlate = lunPlates->get(i_luna);
		unsigned char err = spawnPlate(basePlate, i_luna, lunPlate);
		if (Err(err))
			return err;
	}
	return ECODE_OK;
}
long Luna::getNumNodes() {
	return m_exHex->getNumHexes();
}
unsigned char Luna::spawnPlate(/*const*/s_HexPlate* basePlate, int lunRot, s_HexPlate* lunPlate) {
	if (lunPlate == NULL)
		return ECODE_ABORT;
	long numNodesLunaPlate = m_exHex->getNumCoreHexes();
	if(Err(lunPlate->init(numNodesLunaPlate, LUNA_NUM_FOOTS)))
		return ECODE_FAIL;
	n_HexBase::copyWeave(basePlate, lunPlate);
	s_Node& lunaPat = m_lunas[lunRot];
	for (long node_i = 0; node_i < lunPlate->N; node_i++) {
		s_Hex* lunPlate_hex = lunPlate->get(node_i);
		s_Hex* basePlate_hex = basePlate->get(node_i);
		lunPlate_hex->nodes[0]=basePlate_hex;/*the top node directly under the luna is the corresponding base plate node*/
		lunPlate_hex->w[0] = m_lunas[lunRot].w[0];
		lunPlate_hex->N++;
		for(int i_web=0; i_web<6; i_web++){
			s_Hex* basePlate_webHex = basePlate_hex->getWeb(i_web);
			lunPlate_hex->nodes[i_web + 1] = basePlate_webHex;
			lunPlate_hex->w[i_web + 1] = m_lunas[lunRot].w[i_web + 1];
			lunPlate_hex->N++;
		}
	}

	return ECODE_OK;
}


unsigned char Luna::genLunaPatterns() {
	/*since this hexEye has only one level, the indexes of the nodes hanging from the top node should be the same as their indexes in the bottom plate*/
	for (int i_rot = 0; i_rot < 6; i_rot++) {
		m_lunas[i_rot].init(LUNA_NUM_FOOTS);
		genHalfLunaPattern(i_rot, m_lunas[i_rot]);
	}
	return ECODE_OK;

}

void Luna::genHalfLunaPattern(int lunRot, s_Node& topNd) {
	int arcLen = 3;
	float posFracWeight = 1.f / (3.5f);
	float negFracWeight = -0.5f;//-3.5f / 2.f;//-1.f / arcLen;
	for (int i = 0; i < LUNA_NUM_FOOTS; i++)
		topNd.w[i] = negFracWeight;
	for (int i = 0; i <= arcLen; i++) {
		int i_Nd = Math::loop(lunRot + i, 6);
		i_Nd++;/*center node is 0, arc nodes are 1->6*/
		topNd.w[i_Nd] = posFracWeight;
	}
	int i_line;
	i_line = 0;
	float lineWeight = 0.5f / 3.5f;
	topNd.w[i_line] = lineWeight;
	i_line = Math::loop(lunRot, 6);
	topNd.w[i_line+1] = lineWeight;
	i_line = Math::loop(lunRot + 3, 6);
	topNd.w[i_line+1] = lineWeight;

	for (int i = 0; i < LUNA_NUM_FOOTS; i++)
		topNd.w[i]= LUNA_WSCALE * (topNd.w[i]);
	topNd.N = LUNA_NUM_FOOTS;
}