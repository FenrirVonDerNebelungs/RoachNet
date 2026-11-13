#include "ExHex.h"
ExHex::ExHex() :m_stack(NULL), m_numHanging(0), m_numExHexes(0L), m_numNodes(0L), m_baseDim_in_R(0.f), m_Width(0L), m_Height(0L) {
	;
}
ExHex::~ExHex() {
	;
}
unsigned char ExHex::init(HexStack* stack, int numHanging) {
	if (stack == NULL)
		return ECODE_FAIL;
	m_stack = stack;
	m_numHanging = numHanging;
	long numStackBaseHexes = stack->getNHexesBottom();
	long numStackBaseEdgeHexes = stack->getNumEdgeHexesBottom();
	/*each line hex has 2 half hexes or 1 hex
	* end hex has 2 half hexes that it shares or 1 shared hex
	* end hex also has 1 end hex that is not shared
	*/
	m_numExHexes = numStackBaseEdgeHexes + 6;
	m_numNodes = numStackBaseHexes + m_numExHexes;
	float hexStack_NumHexesLongDim = m_stack->getNumHexesLongDim();
	float sizeExBase_in_R = getSizeExBase(hexStack_NumHexesLongDim);
	m_baseDim_in_R = sizeExBase_in_R;
	float sizeExBase = sizeExBase_in_R * m_stack->getBottomR();
	m_Width = (long)ceilf(sizeExBase);
	m_Height = m_Width;
	return ECODE_OK;
}
void ExHex::release() {
	m_stack = NULL;
	m_numHanging = 0;
	m_numExHexes = 0L;
	m_numNodes = 0L;
}
unsigned char ExHex::spawn(s_rtHexPlate* plate, s_2pt hexUs[]) {
	if (plate == NULL)
		return ECODE_ABORT;
	float r_base = m_stack->getBottomR();
	unsigned char err = plate->init(m_numNodes, hexUs, r_base, m_numHanging);
	if (Err(err))
		return err;
	s_HexPlate* stackBasePlate = m_stack->getBottomPlate();
	err = copyBasePlateWeave(stackBasePlate, plate);
	if (Err(err))
		return err;
	err = weaveExHexes(plate);
	if (Err(err))
		return err;
	err = weaveRoundExHexes(plate);
	if (Err(err))
		return err;
	if(!n_HexBase::computeVecHexDistances(plate))
		return ECODE_FAIL;
	plate->width = m_Width;
	plate->height = m_Height;
	return ECODE_OK;
}
void ExHex::despawn(s_rtHexPlate* plate) {
	if (plate != NULL)
		plate->release();
}
float ExHex::getSizeExBase(float hexStack_NumHexesLongDim) {
	float numHexes_stack_base_radius = hexStack_NumHexesLongDim / 2.f;
	float numHexes_ex_stack_base = numHexes_stack_base_radius + 1.f;
	float long_side = 2.f * n_HexRect::tangR(1.f) * numHexes_ex_stack_base;
	const float short_side = 1.f / 2.f;
	float base_long_dim_in_R = 2.f * sqrtf(short_side * short_side + long_side * long_side);
	return base_long_dim_in_R;
}
unsigned char ExHex::copyBasePlateWeave(/*const*/ s_HexPlate* base, s_rtHexPlate* exPlate) {
	if (base == NULL || exPlate == NULL)
		return ECODE_ABORT;
	if (base->N > exPlate->N)
		return ECODE_ABORT;
	for (long ii = 0; ii < base->N; ii++) {
		s_Hex* baseHex = base->get(ii);
		s_Hex* exHex = (s_Hex*)exPlate->get(ii);
		if (baseHex == NULL || exHex == NULL)
			return ECODE_FAIL;
		for(int i_web =0; i_web<6; i_web++){
			s_Node* linkedBaseHex = baseHex->web[i_web];
			if (linkedBaseHex != NULL) {
				long linkedBaseIndex = linkedBaseHex->thislink;
				s_Hex* linkedExHex = (s_Hex*)exPlate->get(linkedBaseIndex);
				exHex->web[i_web] = (s_Node*)linkedExHex;
				//int rev_web_i = n_HexBase::revWeb_i(i_web);
				//linkedExHex->web[rev_web_i] = (s_Node*)exHex;
			}
		}

	}
	return ECODE_OK;
}
unsigned char ExHex::weaveExHexes(s_rtHexPlate* plate) {
	if (plate == NULL)
		return ECODE_ABORT;
	/*weave the ex hexes around the base hexes*/
	long numBaseHexes = m_numNodes - m_numExHexes;
	long ex_hex_i = numBaseHexes;
	for (long base_hex_i = 0; base_hex_i < numBaseHexes; base_hex_i++) {
		s_Hex* baseHex = (s_Hex*)plate->get(base_hex_i);

		for(int i_web=0; i_web<6; i_web++) {
			s_Hex* linkedHex = baseHex->getWeb(i_web);
			if (linkedHex == NULL) {
				/*find which neighbor is linked*/
				s_Hex* nebHex_1 = baseHex->getWeb(n_HexBase::Web_i(i_web - 1));
				s_Hex* nebHex1 = baseHex->getWeb(n_HexBase::Web_i(i_web + 1));
				int i_web_posPre = i_web;
				s_Hex* candidateDupHex = NULL;
				if (nebHex_1 != NULL) {
					i_web_posPre += 1;
					candidateDupHex = nebHex_1->getWeb(n_HexBase::Web_i(i_web_posPre));
					n_HexBase::weaveSharedHexToNeighbors(candidateDupHex, nebHex_1, baseHex);
				}
				if (nebHex1 != NULL && candidateDupHex==NULL) {
					i_web_posPre -= 1;
					candidateDupHex = nebHex1->getWeb(n_HexBase::Web_i(i_web_posPre));
					n_HexBase::weaveSharedHexToNeighbors(candidateDupHex, nebHex1, baseHex);
				}
				/*this is an unlinked edge hex, link to the ex hext*/
				if (candidateDupHex==NULL) {
					s_Hex* exHex = (s_Hex*)plate->get(ex_hex_i);
					n_HexBase::stichHexes(baseHex, exHex, i_web);
					ex_hex_i++;
				}
			}
		}
	}

	return ECODE_OK;
}
unsigned char ExHex::weaveRoundExHexes(s_rtHexPlate* plate) {
	if (plate == NULL)
		return ECODE_ABORT;
	/*weave the ex hexes around the base hexes*/
	long numBaseHexes = m_numNodes - m_numExHexes;
	for(long ex_hex_i = numBaseHexes; ex_hex_i < m_numNodes; ex_hex_i++) {
		s_Hex* exHex = (s_Hex*)plate->get(ex_hex_i);
		if (exHex == NULL)
			return ECODE_FAIL;
		//bool swept_into_interior = false;
		for(int i_web=0; i_web<6; i_web++) {
			s_Hex* linkedHex = exHex->getWeb(i_web);
			n_HexBase::stichTogetherWebHexes(linkedHex);
			/*
			if(linkedHex != NULL)
				swept_into_interior = true;
			if (linkedHex == NULL && swept_into_interior) {
				s_Hex* commonHex = exHex->getWeb(n_HexBase::Web_i(i_web - 1));
				s_Hex* nebHex = exHex->getWeb(n_HexBase::Web_i(i_web + 1));
				n_HexBase::weaveSharedHexToNeighbors(commonHex, exHex, nebHex);
			}
			*/
		}
	}
	return ECODE_OK;
}