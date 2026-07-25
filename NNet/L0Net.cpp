#include "L0Net.h"

L0Net::L0Net() {
	;
}
L0Net::~L0Net() {
	;
}
unsigned char L0Net::init(float r, int num_luna_plates, int num_L0_nets, int num_net_wbs) {
	m_genHexEye = new HexEye;
	if (Err(m_genHexEye->init(r, L0NET_c_numEyeLevels)))
		return ECODE_FAIL;
	m_netsGen = new CNets;
	/*setup for fully connected net with same level of nodes as the eye levels, except bottom is num eye nodes * num plates */
	int* numLevNodes = new int[L0NET_c_numEyeLevels];
	s_HexEye* struct_eye = m_genHexEye->getHexEyePtr();
	for (int i_lev = 0; i_lev < L0NET_c_numEyeLevels; i_lev++) {
		numLevNodes[i_lev] = struct_eye->get(i_lev)->N;
	}
	int max_eye_lev = L0NET_c_numEyeLevels - 1;
	numLevNodes[max_eye_lev] *= num_luna_plates;
	if (Err(m_netsGen->init(m_genHexEye, num_L0_nets, L0NET_c_numEyeLevels, numLevNodes,1)))
		return ECODE_FAIL;
	m_num_luna_plates = num_luna_plates;
	m_num_nets = num_L0_nets;
	m_num_net_wbs = m_netsGen->getTotalNumWs_perNet() + (m_netsGen->getTotalNumNodes_perNet() /*num bs is the nodes minus the 1 for the top node*/-1);
	if (num_net_wbs >= 0 && num_net_wbs != m_num_net_wbs) {
		release();
		return ECODE_ABORT;
	}
	m_refL0Nets = new s_CNets;
	if (Err(spawn(m_refL0Nets)))
		return ECODE_FAIL;
	return ECODE_OK;
}
void L0Net::release() {
	m_num_luna_plates = 0;
	m_num_nets = 0;
	m_num_net_wbs = 0;
	if (m_netsGen != NULL) {
		m_netsGen->release();
		delete m_netsGen;
	}
	m_netsGen = NULL;
	if (m_genHexEye != NULL) {
		m_genHexEye->release();
		delete m_genHexEye;
	}
	m_genHexEye = NULL;
}
unsigned char L0Net::spawn(s_CNets* L0Nets) {
	if (L0Nets == NULL)
		return ECODE_ABORT;
	if (L0Nets->eye != NULL)
		return ECODE_ABORT;
	L0Nets->eye = new s_HexEye;
	if (Err(m_genHexEye->spawn(L0Nets->eye)))
		return ECODE_FAIL;
	if (Err(m_netsGen->spawn(L0Nets)))
		return ECODE_FAIL;
	if (Err(weaveNets(L0Nets)))
		return ECODE_FAIL;
	return ECODE_OK;
}
unsigned char L0Net::spawn(float net_wbs[], s_CNets* L0Nets) {
	if (Err(spawn(L0Nets)))
		return ECODE_FAIL;
	sNet* genNet = m_netsGen->getNetGen();
	if (genNet == NULL)
		return ECODE_FAIL;
	for (int net_i = 0; net_i < m_num_nets; net_i++) {
		float* wbs = new float[m_num_net_wbs];
		get_wbs_line(net_i, wbs, net_wbs);
		s_Net* cur_net = L0Nets->net[net_i];
		if (cur_net == NULL)
			return ECODE_FAIL;
		genNet->importWBsChain(cur_net, wbs, m_num_net_wbs);
		delete[] wbs;
	}
	return ECODE_OK;
}
void L0Net::despawn(s_CNets* L0Nets) {
	if (L0Nets==NULL || m_netsGen == NULL || m_genHexEye == NULL)
		return;
	m_netsGen->despawn(L0Nets);
	m_genHexEye->despawn(L0Nets->eye);
	if (L0Nets->eye != NULL)
		delete L0Nets->eye;
	L0Nets->eye = NULL;
}
unsigned char L0Net::getNumNodesPerLevel_perNet(float levNodes[], int numLev) {
	int* numNodes = m_netsGen->getNumLevNodes_perNet();
	int gen_numLev = m_netsGen->getNumLevels_perNet();
	if (gen_numLev != numLev)
		return ECODE_ABORT;
	for (int i = 0; i < numLev; i++) {
		levNodes[i] = static_cast<float>(numNodes[i]);
	}
	return ECODE_OK;
}
unsigned char L0Net::weaveNets(s_CNets* L0Nets) {
	unsigned char err = ECODE_FAIL;
	s_HexEye* baseEye = L0Nets->getEye();
	for (int i_net = 0; i_net < L0Nets->N; i_net++) {
		err = connNetLinks(L0Nets->get(i_net), baseEye);
		if (Err(err))
			return err;
		err = defaultWeightsNet(L0Nets->get(i_net), baseEye);
		if (Err(err))
			return err;
	}
	return ECODE_OK;
}
unsigned char L0Net::connNetLinks(s_Net* net, s_HexEye* baseEye) {
	if (baseEye->N != net->N)
		return ECODE_FAIL;

	int i_lev = 0;
	/*connect the top to the next level down*/
	s_nPlate* nettop_plate = net->get(i_lev);
	s_Plate* eyetop_plate = baseEye->get(i_lev);
	s_nPlate* netlow_plate = net->get(i_lev + 1);
	int i_top_nd = 0;
	s_nNode* nettop_nd = nettop_plate->get(i_top_nd);
	s_Node* eyetop_nd = eyetop_plate->get(i_top_nd);
	for (int i_low = 0; i_low < nettop_nd->N; i_low++) {
		s_Node* eyelow_nd = eyetop_nd->nodes[i_low];
		int i_low_indx = eyelow_nd->thislink;
		s_nNode* netlow_nd = netlow_plate->get(i_low_indx);
		nettop_nd->nodes[i_low] = netlow_nd;
		((s_nNode*)nettop_nd->nodes[i_low])->hex = (s_Hex*)eyelow_nd;
	}
	/*connect the lower nodes to the base*/
	int i_float_lev = baseEye->N-2;
	if (i_float_lev < 0)
		return ECODE_FAIL;

	int i_base_offset = 0;
	nettop_plate = net->get(i_float_lev);
	s_Plate* eyelow_plate = baseEye->getBottom();
	netlow_plate = net->getBottom();
	int baseNd_N = eyelow_plate->N;
	int num_plates = (netlow_plate->N) / baseNd_N;
	for (i_top_nd = 0; i_top_nd < nettop_plate->N; i_top_nd++) {
		nettop_nd = nettop_plate->get(i_top_nd);
		for (int i_low = 0; i_low < eyelow_plate->N; i_low++) {
			s_Node* eyelow_nd = eyelow_plate->get(i_low);
			for (int i_plate = 0; i_plate < num_plates; i_plate++) {
				i_base_offset = i_plate * baseNd_N;
				int i_bot_indx = i_low + i_base_offset;
				s_nNode* netlow_nd = netlow_plate->get(i_bot_indx);
				nettop_nd->nodes[i_bot_indx] = netlow_nd;
				((s_nNode*)nettop_nd->nodes[i_bot_indx])->hex = (s_Hex*)eyelow_nd;
			}
		}
	}
	return ECODE_OK;
}

unsigned char L0Net::defaultWeightsNet(s_Net* net, s_HexEye* baseEye) {
	/*set the top weights*/
	int i_lev = 0;
	s_nPlate* nettop_plate = net->get(i_lev);
	int i_top_nd = 0;
	s_nNode* nettop_nd = nettop_plate->get(i_top_nd);
	nettop_nd->b = 0.f;
	for (int i_low = 0; i_low < nettop_nd->N; i_low++) {
		s_nNode* netlow_nd = nettop_nd->getHanging(i_low);
		netlow_nd->w[i_low] = 1.f;
	}
	i_lev = net->N - 2;
	if (i_lev < 0)
		return ECODE_FAIL;
	/*set the bottom weights*/
	nettop_plate = net->get(i_lev);
	s_Plate* eyetop_plate = baseEye->get(i_lev);
	s_nPlate* netlow_plate = net->get(i_lev + 1);
	for (i_top_nd = 0; i_top_nd < nettop_plate->N; i_top_nd++) {
		nettop_nd = nettop_plate->get(i_top_nd);
		s_Hex* eyetop_nd = nettop_nd->hex;
		for (int i_hanging = 0; i_hanging < nettop_nd->N; i_hanging++) 
			nettop_nd->w[i_hanging] = 0.f;
		nettop_nd->b = 0.f;
		for (int i_low=0; i_low < eyetop_nd->N; i_low++) {
			s_Hex* eyelow_nd = eyetop_nd->getHanging(i_low);
			int eyelow_indx = eyelow_nd->thislink;
			for (int i_search = 0; i_search < nettop_nd->N; i_search++) {
				int nd_eyelow_indx = nettop_nd->getHanging(i_search)->hex->thislink;
				if (eyelow_indx == nd_eyelow_indx)
					nettop_nd->w[i_search] = 1.f;
			}
		}
	}
	return ECODE_OK;
}
void L0Net::get_wbs_line(int line_i, float wbs[], const float net_wbs[]) {
	int start_i = line_i * m_num_net_wbs;
	for (int i = 0; i < m_num_net_wbs; i++) {
		int wbs_i = start_i + i;
		wbs[i] = net_wbs[wbs_i];
	}
}

bool n_L0Net::run(s_CNets* L0Nets, s_HexPlateLayer* lunaPlates, s_HexPlateLayer* L0Plates, long plate_i) {
	if (!n_CNets::runNNet(L0Nets, lunaPlates, plate_i))
		return false;
	for (int i_net = 0; i_net < L0Nets->N; i_net++) {
		float netVal = L0Nets->net[i_net]->o;
		s_HexPlate* L0_p = L0Plates->p[i_net];
		s_Hex* L0_hex = L0_p->get(plate_i);
		L0_hex->o = netVal;
	}
	return true;
}
