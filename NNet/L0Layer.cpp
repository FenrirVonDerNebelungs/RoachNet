#include "L0Layer.h"

L0Layer::L0Layer() :m_num_luna(0), m_num_L0_nets(0), m_num_net_wbs(0),
m_genL0Nets(NULL), m_L0Nets(NULL){
	;
}
L0Layer::~L0Layer() {
	;
}

unsigned char L0Layer::init(LunaLayer* genLunaLayer, float net_wbs_stack[], int len_net_wbs, int num_L0_nets){
	if (net_wbs_stack == NULL)
		return ECODE_ABORT;
	if (genLunaLayer == NULL || m_num_L0_nets < 1)
		return ECODE_ABORT;
	m_num_luna = genLunaLayer->getNumLuna();
	float r_scale = genLunaLayer->get_r();
	m_num_net_wbs = len_net_wbs;
	m_num_L0_nets = num_L0_nets;
	m_genL0Nets = new L0Net;
	unsigned char err = ECODE_FAIL;
	err = m_genL0Nets->init(r_scale, m_num_luna, m_num_L0_nets, m_num_net_wbs);
	if (Err(err)) {
		release();
		return ECODE_FAIL;
	}
	m_L0Nets = new s_CNets;
	if (Err(m_genL0Nets->spawn(net_wbs_stack, m_L0Nets))) {
		release();
		return ECODE_FAIL;
	}
		
	return err;
}
void L0Layer::release() {
	if (m_genL0Nets != NULL) {
		m_genL0Nets->despawn(m_L0Nets);
	}
	if (m_L0Nets != NULL)
		delete m_L0Nets;
	m_L0Nets = NULL;
	if (m_genL0Nets != NULL) {
		m_genL0Nets->release();
		delete m_genL0Nets;
	}
	m_genL0Nets = NULL;
	m_num_L0_nets = 0;
	m_num_net_wbs = 0;
	m_num_L0_nets = 0;
}

unsigned char L0Layer::spawn(s_HexPlateLayer* lunaPlates, s_HexPlateLayer* L0Plates) {
	/*check some stuff*/
	if (lunaPlates == NULL || L0Plates == NULL || lunaPlates->N<1)
		return ECODE_ABORT;
	s_HexPlate* dim_plate = lunaPlates->p[0];
	for (int i_lunap = 1; i_lunap < lunaPlates->N; i_lunap++) {
		long dim_N = dim_plate->N;
		long cur_N = lunaPlates->p[i_lunap]->N;
		if (dim_N != cur_N)
			return ECODE_ABORT;
	}

	/*now generate the plates*/
	unsigned char err = ECODE_FAIL;
	err = L0Plates->init(m_num_L0_nets);
	if (Err(err))
		return ECODE_FAIL;
	L0Plates->N = 0;
	for (int i_p = 0; i_p < m_num_L0_nets; i_p++) {
		L0Plates->p[i_p] = new s_HexPlate;
		err = spawnPlateFromLunaLayer(lunaPlates, L0Plates->p[i_p]);
		if (Err(err))
			return ECODE_FAIL;
		L0Plates->N++;
	}
	return ECODE_OK;
}
void L0Layer::despawn(s_HexPlateLayer* L0Plates) {
	if (L0Plates == NULL)
		return;
	for (int i_p = 0; i_p < L0Plates->N; i_p++) {
		if (L0Plates->p[i_p] != NULL) {
			L0Plates->p[i_p]->release();
			delete L0Plates->p[i_p];
			L0Plates->p[i_p] = NULL;
		}
	}
	L0Plates->release();
}
unsigned char L0Layer::run(s_HexPlateLayer* lunaPlates, s_HexPlateLayer* L0Plates) {
	long num_hex = lunaPlates->get(0)->N;
	for (long i_hex = 0; i_hex < num_hex; i_hex++) {
		n_L0Layer::run(this, lunaPlates, L0Plates, i_hex);
	}
	return ECODE_OK;
}


unsigned char L0Layer::spawnPlateFromLunaLayer(s_HexPlateLayer* lunaPlates, s_HexPlate* p) {
	if (p==NULL || lunaPlates->N < 1)
		return ECODE_ABORT;
	s_HexPlate* dim_plate = lunaPlates->p[0];
	return p->init(dim_plate);
}

bool n_L0Layer::run(L0Layer* genL0, s_HexPlateLayer* lunaPlates, s_HexPlateLayer* L0Plates, long plate_i) {
	return n_L0Net::run(genL0->getL0Nets(), lunaPlates, L0Plates, plate_i);
}