#include "sNet.h"
s_Net::s_Net():lev(NULL), N(0), o(0.f), N_mem(0) {
	;
}
s_Net::~s_Net() {
	;
}
unsigned char s_Net::init(int nLev) {
	if (nLev < 1)
		return ECODE_ABORT;
	lev = new s_nPlate * [nLev];
	if (lev == NULL)
		return ECODE_FAIL;
	for (int i_lev = 0; i_lev < nLev; i_lev++)
		lev[i_lev] = NULL;
	N_mem = nLev;
	N = 0;
	return ECODE_OK;
}
unsigned char s_Net::init(const s_Net& other) {
	unsigned char err = init(other.N_mem);
	if (err != ECODE_OK)
		return err;
	this->N = other.N;
	for (int ii = 0; ii < N; ii++) {
		s_nPlate* other_lev = other.lev[ii];
		if (other_lev != NULL) {
			this->lev[ii] = new s_nPlate;
			if ((this->lev[ii]) == NULL)
				return ECODE_FAIL;
			err = this->lev[ii]->init(other_lev);
			if (err != ECODE_OK)
				return err;
		}
	}
	/* fix intra-plate links */
	for (int ii = 0; ii < (N - 1); ii++) {
		s_Plate* topLev = this->lev[ii];
		s_Plate* lowLev = this->lev[ii + 1];
		n_Plate::fixStackedPlateLinks(topLev, lowLev);
	}
	return ECODE_OK;
}
unsigned char s_Net::newLevs() {
	for (int i_lev = 0; i_lev < N_mem; i_lev++) {
		if (lev[i_lev] != NULL)
			return ECODE_FAIL;
		lev[i_lev] = new s_nPlate;
		N++;
	}
	return ECODE_OK;
}
void s_Net::delLevs() {
	if (lev == NULL)
		return;
	for (int i_lev = 0; i_lev < N_mem; i_lev++) {
		if (lev[i_lev] != NULL)
			delete lev[i_lev];
		lev[i_lev] = NULL;
	}
	N = 0;
}
void s_Net::release() {
	if (lev != NULL) {
		for (int ii = 0; ii < N_mem; ii++) {
			if (lev[ii] != NULL) {
				lev[ii]->release();
				delete lev[ii];
			}
			lev[ii] = NULL;
		}
		delete[] lev;
	}
	lev = NULL;
	N_mem = 0;
	N = 0;
}
s_HexBaseNet::s_HexBaseNet() :eye(NULL), N_Plates(0) {
	;
}
s_HexBaseNet::~s_HexBaseNet() {
	;
}
unsigned char s_HexBaseNet::init(const s_HexBaseNet& other) {
	const s_HexBaseNet* pother= &other;
	if(ECODE_OK!=s_Net::init(*pother))
		return ECODE_FAIL;
	this->eye = other.eye;
	this->N_Plates = other.N_Plates;
	return ECODE_OK;
}
s_HexEyeNet::s_HexEyeNet() {
	;
}
s_HexEyeNet::~s_HexEyeNet() {
	;
}
unsigned char s_HexEyeNet::init(const s_HexEyeNet& other) {
	const s_HexEyeNet* pother = &other;
	return s_HexBaseNet::init(*pother);
}
s_EyeFNet::s_EyeFNet() {
	;
}
s_EyeFNet::~s_EyeFNet() {
	;
}
unsigned char s_EyeFNet::init(const s_EyeFNet& other) {
	const s_EyeFNet* pother = &other;
	return s_HexBaseNet::init(*pother);
}


void n_Net::run(s_Net* net) {
	/*run base don't need to run the o's through the weights and stepfunction*/
	s_nPlate* net_bottom = net->getBottom();
	for (long nd_i = 0; nd_i < net_bottom->N; nd_i++) {
		s_nNode* net_nd = net_bottom->get(nd_i);
		float X = net_nd->nodes[0]->o;
		net_nd->o = X;
	}
	/*assume that net has at least two levels*/
	for (int lev_i = net->N - 2; lev_i >= 0; lev_i--) {
		s_nPlate* net_lev = net->lev[lev_i];
		for (long nd_i = 0; nd_i < net_lev->N; nd_i++) {
			s_nNode* net_node = net_lev->get(nd_i);
			float node_sum = 0.f;
			for (int w_i = 0; w_i < net_node->N; w_i++) {
				float X = net_node->nodes[w_i]->o;
				float w = net_node->w[w_i];
				node_sum += w * X;
			}
			node_sum += net_node->b;
			float node_o = Math::StepFuncSym(node_sum);
			net_node->o = node_o;
		}
	}
	net->o = net->getTop()->get(0)->o;
}
inline long n_HexBaseNet::getPlateIndexFromRootedNetNode(s_nPlate* net_base, long net_nd_i) {
	s_nNode* net_nd = net_base->get(net_nd_i);
	s_Hex* eye_hex = net_nd->hex;
	s_Hex* rooted_plate_hex = eye_hex->getHanging(0);
	long rooted_plate_indx = rooted_plate_hex->thislink;
	return rooted_plate_indx;
}
inline void n_HexBaseNet::rootNetNodeOnPlate(s_nPlate* net_base, long net_nd_i, s_HexPlate* plate, long rooted_plate_index) {
	s_nNode* net_nd = net_base->get(net_nd_i);
	s_Node* plate_Nd = plate->getNd(rooted_plate_index);
	net_nd->nodes[0] = plate_Nd;
}
void n_HexEyeNet::rootNetFromRootedEye(s_HexBaseNet* net, s_HexPlate* plate) {
	/*assume s_hexeye tower is rooted such that its bottom layer s_hex es link down by hanging to the plate below*/
	s_nPlate* net_base = net->getBottom();
	long num_net_nodes = net_base->N;
	for (long net_nd_i = 0; net_nd_i < num_net_nodes; net_nd_i++) {
		long rooted_plate_indx = n_HexBaseNet::getPlateIndexFromRootedNetNode(net_base, net_nd_i);
		n_HexBaseNet::rootNetNodeOnPlate(net_base, net_nd_i, plate, rooted_plate_indx);
	}
}
void n_EyeFNet::rootNetFromRootedEye(s_HexBaseNet* net, s_HexPlateLayer* plates) {
	long num_plates = plates->N;
	for (long i_plate = 0; i_plate < num_plates; i_plate++) {
		s_HexPlate* plate = plates->get(i_plate);
		rootNetFromRootedEye(net, plate, i_plate, num_plates);
	}
}
void n_EyeFNet::rootNetFromRootedEye(s_HexBaseNet* net, s_HexPlate* plate, long node_offset_i/*essentially the plate index*/, long num_plates) {
	/*assume s_hexeye tower is rooted such that its bottom layer s_hex es link down by hanging to the plate below*/
	s_nPlate* net_base = net->getBottom();
	/*long num_net_nodes = net_base->N must equal the max node_offset_i+1 sent * number of eye hexes */
	long num_eye_base_hexes = net->eye->getBottom()->N;
	for (long base_nd_i = 0; base_nd_i < num_eye_base_hexes; base_nd_i++) {
		long net_nd_i = base_nd_i*num_plates + node_offset_i;
		long rooted_plate_indx = n_HexBaseNet::getPlateIndexFromRootedNetNode(net_base, net_nd_i);
		n_HexBaseNet::rootNetNodeOnPlate(net_base, net_nd_i, plate, rooted_plate_indx);
	}
}




sNet::sNet() :m_genHexEye(NULL), m_total_num_weights(0), m_total_num_nodes(0), m_net_type(CNN), m_nLev(0), m_numLevNodes(NULL), m_numHanging(0), m_numPlates(0) { ; }
sNet::~sNet() { ; }
unsigned char sNet::init(int nLev, int numLevNodes[], int numHanging) {
	if (nLev < 1 || numHanging<1) {
		return ECODE_ABORT;
	}
	if (numLevNodes == NULL) {
		return ECODE_FAIL;
	}
	for (int i = 0; i < nLev; i++)
		if (numLevNodes[i] < 1)
			return ECODE_ABORT;

	m_nLev = nLev;
	m_numLevNodes = new int[m_nLev + 1];
	for (int i = 0; i < nLev; i++)
		m_numLevNodes[i] = numLevNodes[i];
	m_numLevNodes[nLev] = numHanging;
	m_numHanging = numHanging;
}
unsigned char sEyeFNet::init(HexEye* eye, int nLev, int numInnerLevNodes[], int numBottomPlates_to_be_connected) {
	if (nLev < 3)
		return ECODE_ABORT;
	if (eye == NULL)
		return ECODE_ABORT;
	m_genHexEye = eye;
	int number_of_eye_levels = m_genHexEye->getNLevels();
	if (number_of_eye_levels < 2)
		return ECODE_ABORT;

	int number_of_bottom_eye_hexes = (int)m_genHexEye->getNHexes(number_of_eye_levels - 1);
	/*for neural net which this class supports each bottom hex will be connected to all hexes for each plate
	   and to all hexes in all plates */
	int numBottomLevNodes = number_of_bottom_eye_hexes*numBottomPlates_to_be_connected;
	int inner_nLev = nLev - 2;
	int* numLevNodes = new int[nLev];
	numLevNodes[0] = 1;
	for (int i_inLev = 0; i_inLev < inner_nLev; i_inLev++)
		numLevNodes[1 + i_inLev] = numInnerLevNodes[i_inLev];
	numLevNodes[nLev - 1] = numBottomLevNodes;
	unsigned char errc = sNet::init(nLev, numLevNodes, 1/*numHanging*/);
	delete[] numLevNodes;
	if (Err(errc)) {
		return ECODE_ABORT;
	}


	if (Err(spawn(eye->getHexEyePtr(), &m_refNet)))
		return ECODE_FAIL;
	initSetNumWeightsAndNodes();
	return ECODE_OK;
}
unsigned char sHexEyeNet::init(HexEye* eye, int numBottomPlates_to_be_connected) {
	if (eye == NULL)
		return ECODE_ABORT;
	m_genHexEye = eye;
	int nLev = eye->getNLevels();
	int* numLevNodes = new int[nLev];
	for (int i = 0; i < nLev; i++)
		numLevNodes[i] = eye->getNHexes(i);
	/*num hanging will be the number of bottom plates to be connected*/
	if (Err(sNet::init(nLev, numLevNodes, numBottomPlates_to_be_connected)))
		return ECODE_ABORT;
	delete[] numLevNodes;

	m_numHanging = numBottomPlates_to_be_connected;/*numHanging has been set by init*/
	m_numBottomPlates_to_be_connected = numBottomPlates_to_be_connected;

	return ECODE_OK;
}


void sNet::release() {
	m_refNet.release();
	m_numHanging = 0;
	if (m_numLevNodes != NULL)
		delete[] m_numLevNodes;
	m_numLevNodes = NULL;
	m_nLev = 0;
	m_genHexEye = NULL;
}

unsigned char sHexEyeNet::genNet(s_HexEyeNet* sn) {
	if(Err(sNet::spawn(sn)))
		return ECODE_FAIL;
	unsigned char err = ECODE_FAIL;
	for (int ii = 0; ii < (m_nLev-1); ii++) {
		err = sn->lev[ii]->init((long)m_numLevNodes[ii], HEXEYE_NUM_LOWER_NODES_PER_HEX);
		if (Err(err)) {
			despawn(sn);
			return err;
		}
	}
	err = sn->lev[m_nLev - 1]->init((long)m_numLevNodes[m_nLev - 1], m_numHanging);
	sn->N_Plates = m_numBottomPlates_to_be_connected;
	return ECODE_OK;
}
unsigned char sEyeFNet::genNet(s_EyeFNet* sn) {
	if (Err(sNet::spawn(sn)))
		return ECODE_FAIL;
	for(int ii=0; ii<m_nLev; ii++){
		unsigned char err = sn->lev[ii]->init((long)m_numLevNodes[ii], m_numLevNodes[ii + 1]);
		if (Err(err)) {
			despawn(sn);
			return err;
		}
		sn->N++;
	}
	sn->N_Plates = m_numBottomPlates_to_be_connected;
	return connDownNet(sn);
}
unsigned char sEyeFNet::spawn(s_HexEye* eye, s_EyeFNet* sn) {
	if (eye == NULL || sn == NULL || m_genHexEye == NULL)
		return ECODE_ABORT;
	if (Err(genNet(sn)))
		return ECODE_FAIL;
	/*connect bottom plate to eye*/
	if (Err(connBotNetToEye(sn, eye)))
		return ECODE_FAIL;
	/*connect top plate/node to eye */
	if (Err(connTopNetToEye(sn, eye)))
		return ECODE_FAIL;
	return ECODE_OK;
}
unsigned char sNet::spawn(s_Net* sn) {
	if (sn == NULL)
		return ECODE_ABORT;
	if (Err(sn->init(m_nLev)))
		return ECODE_FAIL;
	if (Err(sn->newLevs()))
		return ECODE_FAIL;
	return ECODE_OK;
}
unsigned char sHexEyeNet::spawn(s_HexEye* eye, s_HexEyeNet* sn) {
	if (sn == NULL || eye == NULL || m_genHexEye == NULL)
		return ECODE_ABORT;
	if (m_genHexEye->getNLevels() != eye->N || m_genHexEye->getNLevels() != m_nLev)
		return ECODE_ABORT;

	unsigned char err = genNet(sn);
	if (Err(err))
		return err;
	/*connect bottom plate to eye*/
	if (Err(connBotNetToEye(sn, eye)))
		return ECODE_FAIL;
	/*connect top plate/node to eye */
	if (Err(connTopNetToEye(sn, eye)))
		return ECODE_FAIL;
	/*connect the nodes of the nnet so that its structure is the same as the hex eye*/
	for (int ii = 0; ii < (sn->N - 1); ii++) {
		s_nPlate* netplate = sn->lev[ii];
		s_HexPlate* hexplate = eye->get(ii);
		if (netplate->N != hexplate->N)
			return ECODE_FAIL;
		s_nPlate* netplate_lo = sn->lev[ii + 1];
		s_HexPlate* hexplate_lo = eye->get(ii + 1);
		if (netplate_lo->N != hexplate_lo->N)
			return ECODE_FAIL;
		for (long i_netNode = 0; i_netNode < netplate->N; i_netNode++) {
			s_nNode* netNode = netplate->get(i_netNode);
			s_Hex* eyeNode = hexplate->get(i_netNode);
			for (int i_hanging = 0; i_hanging < 7; i_hanging++) {
				s_Hex* eyeNode_lo = eyeNode->getHanging(i_hanging);
				long lo_plate_index = eyeNode_lo->thislink;
				s_nNode* netNode_lo = netplate_lo->get(lo_plate_index);
				netNode->setHanging(netNode_lo, i_hanging);
				netNode->N++;
			}
			netNode->hex = eyeNode;
		}
	}

	return ECODE_OK;
}
void sNet::despawn(s_Net* sn) {
	if (sn == NULL)
		return;
	sn->eye = NULL;
	for (int ii = 0; ii < m_nLev; ii++) {
		if (sn->lev[ii] != NULL) {
			sn->lev[ii]->release();
		}
	}
	sn->delLevs();
	sn->release();
} 

int sEyeFNet::dumpWBsChain(s_Net* sn, float wbs[]) {
	if (sn == NULL || wbs==NULL)
		return -1;
	if (sn->N < 2)
		return 0;
	int len_wbs = 0;
	for (int i_lev = 0; i_lev < sn->N-1; i_lev++) {
		s_nPlate* np = sn->lev[i_lev];
		if (np == NULL)
			return len_wbs;
		for (int i_nd = 0; i_nd < np->N; i_nd++) {
			s_nNode* nd = np->get(i_nd);
			if (nd != NULL) {
				for (int i_hang = 0; i_hang < nd->N; i_hang++) {
					float w = nd->w[i_hang];
					wbs[len_wbs] = w;
					len_wbs++;
				}
				wbs[len_wbs] = nd->b;
				len_wbs++;
			}
		}
	}
	return len_wbs;
}

bool sEyeFNet::importWBsChain(s_Net* sn, float wbs[], int len_wbs) {
	if (sn == NULL || wbs == NULL)
		return false;
	if (sn->N < 2)
		return (len_wbs < 1) ? true : false;
	if (len_wbs < 2)
		return false;
	int i_wbs = 0;
	for (int i_lev = 0; i_lev < sn->N-1; i_lev++) {
		s_nPlate* np = sn->lev[i_lev];
		if (np == NULL)
			return false;
		for (int i_nd = 0; i_nd < np->N; i_nd++) {
			s_nNode* nd= np->get(i_nd);
			if (nd != NULL) {
				for (int i_hang = 0; i_hang < nd->N; i_hang++) {
					nd->w[i_hang] = wbs[i_wbs];
					i_wbs++;
					if (i_wbs >= len_wbs)
						return false;
				}
				nd->b = wbs[i_wbs];
				i_wbs++;
				if (i_wbs >= len_wbs)
					return false;
			}
		}
	}
	return true;
}
int sEyeFNet::dumpOWeightLinksChain(s_Net* sn, float os[], int loc_lev[], int loc_i[]) {
	if (sn == NULL || os == NULL || loc_lev==NULL || loc_i==NULL)
		return -1;
	if (sn->N < 2)
		return 0;
	int len_os = 0;
	for (int i_lev = 0; i_lev < sn->N - 1; i_lev++) {
		s_nPlate* np = sn->lev[i_lev];
		if (np == NULL)
			return len_os;
		for (int i_nd = 0; i_nd < np->N; i_nd++) {
			s_nNode* nd = np->get(i_nd);
			if (nd != NULL) {
				for (int i_hang = 0; i_hang < nd->N; i_hang++) {
					float lo_o = nd->getHanging(i_hang)->o;
					os[len_os] = lo_o;
					loc_lev[len_os] = i_lev;
					loc_i[len_os] = (int)nd->getHanging(i_hang)->thislink;
					len_os++;
				}
			}
			os[len_os] = -1.f;
			loc_lev[len_os] = -1;
			loc_i[len_os] = -1;
			len_os++;
		}
	}
	return len_os;
}
int sEyeFNet::dumpOsChain(s_Net* sn, float os[]) {
	if (sn == NULL || os == NULL)
		return -1;
	int len_os=0;
	for (int i_lev = 0; i_lev < sn->N; i_lev++) {
		s_nPlate* np = sn->lev[i_lev];
		if (np == NULL)
			return len_os;
		for (int i_nd = 0; i_nd < np->N; i_nd++) {
			s_nNode* nd = np->get(i_nd);
			os[len_os] = nd->o;
			len_os++;
		}
	}
	return len_os;
}
int sEyeFNet::dumpOsChainLoc(s_Net* sn, s_2pt_i os[]) {
	if (sn == NULL || os == NULL)
		return -1;
	if (sn->eye == NULL)
		return -2;
	int len_os = 0;
	for (int i_lev = 0; i_lev < sn->N; i_lev++) {
		s_nPlate* np = sn->lev[i_lev];
		if (np == NULL)
			return len_os;
		for (int i_nd = 0; i_nd < np->N; i_nd++) {
			s_nNode* nd = np->get(i_nd);
			s_2pt_i nd_loc = { 0L, 0L };
			if (nd->hex != NULL) {
				nd_loc.x0 = Math::ftol(nd->hex->x);
				nd_loc.x1 = Math::ftol(nd->hex->y);
			}
			os[len_os] = nd_loc;
			len_os++;
		}
	}
	return len_os;
}
int sEyeFNet::dumpOsLinksChain(s_Net* sn, int loc_lev[], int loc_i[]) {
	if (sn == NULL || loc_lev == NULL || loc_i == NULL)
		return -1;
	int len_os = 0;
	for (int i_lev = 0; i_lev < sn->N; i_lev++) {
		s_nPlate* np = sn->lev[i_lev];
		if (np == NULL)
			return len_os;
		for (int i_nd = 0; i_nd < np->N; i_nd++) {
			s_nNode* nd = np->get(i_nd);
			loc_lev[len_os] = i_lev;
			loc_i[len_os] = nd->thislink;
			len_os++;
		}
	}
	return len_os;
}
bool sEyeFNet::dumpWBsOChainMatch(s_Net* sn, int oloc_i[], int wblen) {
	int max_array_len = getTotalNumWBs();
	if (wblen != max_array_len)
		return false;
	for (int i = 0; i < wblen; i++)
		oloc_i[i] = -1;

	float* os = new float[max_array_len];
	int* w_loc_lev = new int[max_array_len];
	int* w_loc_i = new int[max_array_len];
	int WBs_len = dumpOWeightLinksChain(sn, os, w_loc_lev, w_loc_i);
	if (WBs_len != wblen)
		return false;

	int* o_loc_lev = new int[max_array_len];
	int* o_loc_i = new int[max_array_len];
	int Os_len = dumpOsLinksChain(sn, o_loc_lev, o_loc_i);
	if (Os_len > wblen)
		return false;
	for (int i = 0; i < wblen; i++) {
		int lev_index = w_loc_lev[i];
		int nd_index = w_loc_i[i];
		for (int i_Os = 0; i_Os < Os_len; i_Os++) {
			if (o_loc_lev[i_Os] == lev_index)
				if (o_loc_i[i_Os] == nd_index)
					oloc_i[i_Os] = i;
		}
	}
	return true;
}

unsigned char sHexEyeNet::connBotNetToEye(s_HexEyeNet* sn, s_HexEye* eye) {
	if (eye->getBottom()->N != sn->getBottom()->N)
		return ECODE_ABORT;
	for (int ii = 0; ii < eye->getBottom()->N; ii++) {
		s_nNode* net_node = sn->getBottom()->get(ii);
		s_Hex* eye_node = eye->getBottom()->get(ii);
		net_node->hex = eye_node;
	}
	return ECODE_OK;
}
unsigned char sHexEyeNet::connTopNetToEye(s_HexEyeNet* sn, s_HexEye* eye) {
	if (sn->lev[0] == NULL || eye->lev[0] == NULL)
		return ECODE_FAIL;
	s_nNode* net_top_node = sn->lev[0]->get(0);
	s_Hex* eye_top_node = eye->lev[0]->get(0);
	net_top_node->hex = eye_top_node;
	sn->eye = eye;
	return ECODE_OK;
}
unsigned char sEyeFNet::connTopNetToEye(s_EyeFNet* sn, s_HexEye* eye) {
	if (sn->lev[0] == NULL || eye->lev[0] == NULL)
		return ECODE_FAIL;
	s_nNode* net_top_node = sn->lev[0]->get(0);
	s_Hex* eye_top_node = eye->lev[0]->get(0);
	net_top_node->hex = eye_top_node;
	sn->eye = eye;
	return ECODE_OK;
}
unsigned char sEyeFNet::connDownNet(s_Net* sn) {
	int num_net_lev = sn->N;
	if (num_net_lev < 1)
		return ECODE_ABORT;
	for (int ii = 0; ii < (num_net_lev - 1); ii++) {
		int num_low_lev_nodes = (int)sn->lev[ii+1]->N;
		int num_lev_nodes = (int)sn->lev[ii]->N;
		for (int i_nd = 0; i_nd < num_lev_nodes; i_nd++) {
			s_nNode* nd = sn->lev[ii]->get(i_nd);
			int num_hanging = nd->getNmem();
			if (num_hanging != num_low_lev_nodes)
				return ECODE_FAIL;
			for (int i_hanging = 0; i_hanging < num_hanging; i_hanging++) {
				s_Node* hanging_nd = sn->lev[ii + 1]->getNd(i_hanging);
				nd->nodes[i_hanging] = hanging_nd;
			}
			nd->N = num_hanging;
		}
	}
	return ECODE_OK;
}

unsigned char sEyeFNet::connBotNetToEye(s_EyeFNet* sn, s_HexEye* eye) {
	/*eye->getBottom()->N * m_numBottomPlates_to_be_connected = sn->getBottom()->N */
	int i_cnn = 0;
	for (int i_bot_hex = 0; i_bot_hex < eye->getBottom()->N; i_bot_hex++) { /*loop over the bottom eye plate hexes*/
		s_Hex* eye_node = eye->getBottom()->get(i_bot_hex);
		for (int i_plate = 0; i_plate < m_numBottomPlates_to_be_connected; i_plate++) { 
			s_nNode* net_node = sn->getBottom()->get(i_cnn);
			net_node->hex = eye_node;
			i_cnn++;
		}
	}
	return ECODE_OK;
}
unsigned char sEyeFNet::initSetNumWeightsAndNodes() {
	int lev_num_weights = 0;
	int lev_num_nds = 0;
	if (m_refNet.N <= 0)
		return ECODE_ABORT;
	m_total_num_weights = 0;
	m_total_num_nodes = 0;
	for (int i_lev = 0; i_lev < m_refNet.N-1; i_lev++) {
		if (Err(getNumWeightsAndNodesPerEyeLevel(i_lev, lev_num_weights, lev_num_nds)))
			return ECODE_FAIL;
		m_total_num_weights += lev_num_weights;
		m_total_num_nodes += lev_num_nds;
	}
	if (Err(getNumWeightsAndNodesPerEyeLevel(m_refNet.N - 1, lev_num_weights, lev_num_nds)))
		return ECODE_FAIL;
	/*bottom level weights don't count*/
	m_total_num_nodes += lev_num_nds;
	return ECODE_OK;
}

unsigned char sEyeFNet::getNumWeightsAndNodesPerEyeLevel(int level, int& total_num_weights, int& total_num_nds) {
	if (level<0 || level>=m_refNet.N)
		return ECODE_ABORT;
	s_nPlate* levPlate = m_refNet.lev[level];
	if (levPlate == NULL)
		return ECODE_ABORT;
	total_num_weights = 0;
	total_num_nds = 0;
	for (long i_node = 0; i_node < levPlate->N; i_node++) {
		s_nNode* nd = levPlate->get(i_node);
		int num_weights = nd->N;/*num_weights is the same as N which is the number of the lower nodes*/
		total_num_weights += num_weights;
		total_num_nds += 1;
	}
	return ECODE_OK;
}
