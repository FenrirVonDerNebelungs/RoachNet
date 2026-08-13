#include "NNet.h"
void n_Node_w::zero(s_Node_w& ndw) {
	ndw.lev_i = -1;
	ndw.nd_i = -1;
	ndw.w = 0.f;
	ndw.lo_i = -1;
}
void n_Node_w::copy(s_Node_w& newNdw, const s_Node_w& orig) {
	newNdw.lev_i = orig.lev_i;
	newNdw.nd_i = orig.nd_i;
	newNdw.w = orig.w;
	newNdw.lo_i = orig.lo_i;
}

s_nNode::s_nNode() : b(0.f) {
	;
}
s_nNode::~s_nNode() {
	;
}
unsigned char s_nNode::init(int nNodes) {
	b = 0.f;
	return s_Node::init(nNodes);
}
unsigned char s_nNode::init(const s_nNode* other) {
	this->b = other->b;
	return s_Node::init((const s_Node*)other);
}
s_nNode& s_nNode::operator=(const s_nNode& other) {
	copy(&other);
}
void s_nNode::copy(const s_nNode* other) {
	s_Node::copy((const s_Node*)other);
	this->b = other->b;
}

s_nPlate::s_nPlate() {
	;
}
s_nPlate::~s_nPlate() {
	;
}
unsigned char s_nPlate::init(long nNodes) {
	return s_Plate::init(nNodes);
}
unsigned char s_nPlate::init(const s_nPlate* other) {
	return s_Plate::init((const s_Plate*)other);
}
void s_nPlate::release() {
	s_Plate::release();
}

s_NNet::s_NNet() : lev(NULL), N_lev(0){
	;
}
s_NNet::~s_NNet() {
	;
}
unsigned char s_NNet::init(int _N_lev) {
	if (_N_lev < 1)
		return ECODE_ABORT;
	N_lev = _N_lev;
	lev = new s_nPlate * [N_lev];
	for (int i_lev = 0; i_lev < N_lev; i_lev++) {
		lev[i_lev] = new s_nPlate;
	}
	return ECODE_OK;
}

NNet::NNet() : m_N_lev(0), m_nodesPerLev(NULL), m_N_base(0) {
	;
}
NNet::~NNet() {
	;
}
unsigned char NNet::init(int N_lev, long nodesPerLev[], int N_base) {
	if (m_N_lev < 1)
		return ECODE_ABORT;
	m_N_lev = N_lev;
	m_nodesPerLev = new int[m_N_lev];
	for (int i = 0; i < m_N_lev; i++)
		m_nodesPerLev[i] = nodesPerLev[i];
	m_N_base = N_base;
}
unsigned char NNet::spawn(s_NNet* nnet) {
	if (nnet == NULL)
		return ECODE_ABORT;
	if (m_N_base < 1)
		return ECODE_ABORT;
	if (Err(nnet->init(m_N_lev)))
		return ECODE_ABORT;
	for (int i_lev = 0; i_lev < m_N_lev; i_lev++) {
		s_nPlate* net_plate = nnet->getLev(i_lev);
		if (Err(net_plate->init(m_nodesPerLev[i_lev])))
			return ECODE_FAIL;
		for (long i_node = 0; i_node < m_nodesPerLev[i_lev]; i_node++) {
			net_plate->nodes[i_node] = new s_Node;
			net_plate->N++;
		}
	}
	for (int i_lev = 0; i_lev < (m_N_lev-1); i_lev++) {
		s_nPlate* net_plate = nnet->getLev(i_lev);
		for (long i_node = 0; i_node < m_nodesPerLev[i_lev]; i_node++) 
			if(Err(net_plate->nodes[i_node]->init(m_nodesPerLev[i_lev + 1])))
				return ECODE_FAIL;
	}
	s_nPlate* bot_net_plate = nnet->getLev(m_N_lev - 1);
	for (long i_node = 0; i_node < m_nodesPerLev[m_N_lev - 1]; i_node++)
		if (Err(bot_net_plate->nodes[i_node]->init(m_N_base)))
			return ECODE_FAIL;
	return weaveNet(nnet);
}

unsigned char NNet::set(s_NNet* nnet, int stream_len_w, int stream_len_bias, const s_Node_w weights[], const s_Node_w biases[]) {
	for (int i_stream = 0; i_stream < stream_len_w; i_stream++) {
		s_Node_w weight = weights[i_stream];	
		if (weight.lev_i >= nnet->getNLev())
			return ECODE_ABORT;
		s_nPlate* net_plate = nnet->getLev(weight.lev_i);
		if (weight.nd_i >= net_plate->N)
			return ECODE_ABORT;
		s_nNode* nd = net_plate->get(weight.nd_i);
		if (weight.lo_i >= nd->N)
			return ECODE_ABORT;
		nd->w[weight.lo_i] = weight.w;
	}
	for (int i_stream = 0; i_stream < stream_len_bias; i_stream++) {
		s_Node_w bias = biases[i_stream];
		if (bias.lev_i >= nnet->getNLev())
			return ECODE_ABORT;
		s_nPlate* net_plate = nnet->getLev(bias.lev_i);
		if (bias.nd_i >= net_plate->N)
			return ECODE_ABORT;
		s_nNode* nd = net_plate->get(bias.nd_i);
		nd->setB(bias.w);
	}
	return ECODE_OK;
}
int NNet::getWeights(s_Node_w weights[]) {
	if (m_N_lev < 1)
		return -1;
	int num_stream = 0;
	for (int i_lev = 0; i_lev < (m_N_lev-1); i_lev++) {
		for (long i_nd = 0; i_nd < m_nodesPerLev[i_lev]; i_nd++) {
			for (long i_lo = 0; i_lo < m_nodesPerLev[i_lev + 1]; i_lo++) {
				weights[num_stream].lev_i = i_lev;
				weights[num_stream].nd_i = i_nd;
				weights[num_stream].w = 0.f;
				weights[num_stream].lo_i = i_lo;
				num_stream++;
			}
		}
	}
	for (long i_nd = 0; i_nd < m_nodesPerLev[m_N_lev - 1]; i_nd++) {
		for (long i_lo = 0; i_lo < m_N_base; i_lo++) {
			weights[num_stream].lev_i = m_N_lev - 1;
			weights[num_stream].nd_i = i_nd;
			weights[num_stream].w = 0.f;
			weights[num_stream].lo_i = i_lo;
			num_stream++;
		}
	}
	return num_stream;
}
int NNet::getBiases(s_Node_w biases[]) {
	if (m_N_lev < 1)
		return -1;
	int num_stream = 0;
	for (int i_lev = 0; i_lev < m_N_lev; i_lev++) {
		for (long i_nd = 0; i_nd < m_nodesPerLev[i_lev]; i_nd++) {
			biases[num_stream].lev_i = i_lev;
			biases[num_stream].nd_i = i_nd;
			biases[num_stream].w = 0.f;
			biases[num_stream].lo_i = -1;
			num_stream++;
		}
	}

	return num_stream;
}
int NNet::getNumWeights() {
	int total_num = 0;
	for (int i_lev = 0; i_lev < (m_N_lev - 1); i_lev++) {
		total_num+=m_nodesPerLev[i_lev] * m_nodesPerLev[i_lev + 1];
	}
	total_num += m_nodesPerLev[m_N_lev - 1] * m_N_base;
	return total_num;
}
int NNet::getNumBiases() {
	int total_num;
	for (int i_lev = 0; i_lev < m_N_lev; i_lev++) {
		total_num += m_nodesPerLev[i_lev];
	}
	return total_num;
}

unsigned char NNet::weaveNet(s_NNet* nnet) {
	for (int i_lev = 0; i_lev < (m_N_lev - 1); i_lev++) {
		s_nPlate* net_plate = nnet->getLev(i_lev);
		for (long i_node = 0; i_node < net_plate->N; i_node++)
			if (!weaveNodeToPlate(net_plate->get(i_node), nnet->getLev(i_lev + 1)))
				return ECODE_FAIL;
	}
}
bool NNet::weaveNodeToPlate(s_nNode* nd, s_nPlate* plate) {
	for (long i_lo = 0; i_lo < plate->N; i_lo++) {
		nd->nodes[i_lo] = plate->getNd(i_lo);
	}
	nd->N = plate->N;
}
bool n_NNet::run(s_NNet* nnet) {
	int N_lev_max = nnet->getNLev()-1;
	for (int i_lev = N_lev_max; i_lev >= 0; i_lev--) {
		s_nPlate* lev = nnet->getLev(i_lev);
		for (long i_nd = 0; i_nd < lev->N; i_nd++) {
			s_nNode* nd = lev->get(i_nd);
			float weighted_sum = 0.f;
			for (int i_hang = 0; i_hang < nd->N; i_hang++) {
				s_Node* lo_nd = nd->nodes[i_hang];
				/*assume these are non null for speed*/
				float outval = lo_nd->o;
				float weighted_outval = outval * nd->w[i_hang];
				weighted_sum += weighted_outval;
			}
			float nd_in = weighted_sum + nd->getB();
			nd->o = act_func(nd_in);
		}
	}
}
float n_NNet::act_func(float nd_in) {
	return tanh(nd_in);
}