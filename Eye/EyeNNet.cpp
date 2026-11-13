#include "EyeNNet.h"

EyeNNet::EyeNNet() {
	m_hexStack = NULL;
	m_luna = NULL;
	m_numLayers = 0;
	m_numNodesPerLayer = NULL;
	m_numNodesBase = 0;
	m_len_weights = 0;
	m_weights = NULL;
	m_len_biases = 0;
	m_biases = NULL;
}
EyeNNet::~EyeNNet() {
	;
}
unsigned char EyeNNet::init(HexStack* hexstack, Luna* luna) {
	m_hexStack = hexstack;
	m_luna = luna;
	unsigned char err = initNumNodes();
	if (Err(err))
		return err;
	err = NNet::init(m_numLayers, m_numNodesPerLayer, m_numNodesBase);
	if (Err(err))
		return err;
	m_len_weights = NNet::getNumWeights();
	m_len_biases = NNet::getNumBiases();
	if (m_len_weights < 1 || m_len_biases < 1)
		return ECODE_FAIL;
	m_weights = new s_Node_w[m_len_weights];
	m_biases = new s_Node_w[m_len_biases];
	for (int i = 0; i < m_len_weights; i++)
		n_Node_w::zero(m_weights[i]);
	for (int i = 0; i < m_len_biases; i++)
		n_Node_w::zero(m_biases[i]);
	/* set default weights and biases */
	err = initDefaultNetWeights();
	if (Err(err))
		return err;
	return ECODE_OK;
}
unsigned char EyeNNet::setWB(s_Node_w weights[], s_Node_w biases[]) {
	for (int i = 0; i < m_len_weights; i++)
		n_Node_w::copy(m_weights[i], weights[i]);
	for (int i = 0; i < m_len_biases; i++)
		n_Node_w::copy(m_biases[i], biases[i]);
	return ECODE_OK;
}
void EyeNNet::release() {
	if (m_weights != NULL) {
		delete[] m_weights;
		m_weights = NULL;
	}
	if (m_biases != NULL) {
		delete[] m_biases;
		m_biases = NULL;
	}
	if (m_numNodesPerLayer != NULL) {
		delete[] m_numNodesPerLayer;
		m_numNodesPerLayer = NULL;
	}
	NNet::release();
	m_hexStack = NULL;
	m_luna = NULL;
	m_numLayers = 0;
	m_numNodesBase = 0;
	m_len_weights = 0;
	m_len_biases = 0;
}
unsigned char EyeNNet::spawn(s_HexPlateLayer* lunaPlates, s_NNet* nnet) {
	if (lunaPlates == NULL)
		return ECODE_ABORT;
	if(Err(NNet::spawn(nnet)))
		return ECODE_ABORT;
	if (m_N_lev < 1)
		return ECODE_ABORT;
	int lev_i = m_N_lev - 1;
	s_nPlate* net_plate = nnet->getLev(lev_i);
	for (long i_nd = 0; i_nd < net_plate->N; i_nd++) {
		s_nNode* nd = net_plate->get(i_nd);
		nd->N = 0;
		for (int i_luna_plate = 0; i_luna_plate < lunaPlates->N; i_luna_plate++) {
			s_HexPlate* luna_plate = lunaPlates->get(i_luna_plate);
			for (long i_luna_nd = 0; i_luna_nd < luna_plate->N; i_luna_nd++) {
				nd->nodes[nd->N] = (s_Node*)luna_plate->getNd(i_luna_nd);
				nd->N++;
			}
		}
	}
	
	/*fill the inital weights and biases */
	unsigned char err = NNet::set(nnet, m_len_weights, m_len_biases, m_weights, m_biases);
	if (Err(err))
		return err;
	return ECODE_OK;
}
void EyeNNet::despawn(s_NNet* nnet) {
	NNet::despawn(nnet);
}
int EyeNNet::getLinkedBaseOs(const s_NNet* nnet, s_Node_w os[]) {
	return n_NNet::getLinkedBaseOs(nnet, os);
}


unsigned char EyeNNet::initNumNodes() {
	if (m_hexStack == NULL || m_luna == NULL)
		return ECODE_ABORT;
	if (m_hexStack->getNLevels() < 2)
		return ECODE_FAIL;

	int numNodesPerLunaPlate = m_luna->getNumNodes();
	int numLunaPlates = m_luna->getNumLunaPlates();
	int numBaseNodes = numLunaPlates * numNodesPerLunaPlate;
	m_numNodesBase = numBaseNodes;

	int numNNetLevels = m_hexStack->getNLevels();
	m_numLayers = numNNetLevels /* hang off of level with 7 */ - 1 ;
	m_numNodesPerLayer = new long[m_numLayers] ;
	for (int i_lev = 0; i_lev < m_numLayers; i_lev++) {
		int stack_index = i_lev;
		m_numNodesPerLayer[i_lev] = m_hexStack->getNHexes(stack_index);
	}
	return ECODE_OK;
}
unsigned char EyeNNet::initDefaultNetWeights() {
	if (m_weights == NULL || m_biases == NULL)
		return ECODE_FAIL;
	if (m_numNodesPerLayer == NULL || m_numLayers < 1)
		return ECODE_FAIL;

	int i_w = 0;
	int i_b = 0;
	for (int i_lev = 0; i_lev < (m_numLayers - 1); i_lev++) {
		for (long i_nd = 0; i_nd < m_numNodesPerLayer[i_lev]; i_nd++) {
			m_biases[i_b].lev_i = i_lev;
			m_biases[i_b].nd_i = i_nd;
			m_biases[i_b].w = EYENNET_DEFBIAS;
			m_biases[i_b].lo_i = -1;
			i_b++;
			for (long i_lo = 0; i_lo < m_numNodesPerLayer[i_lev + 1]; i_lo++) {
				m_weights[i_w].lev_i = i_lev;
				m_weights[i_w].nd_i = i_nd;
				m_weights[i_w].w = EYENNET_DEFWEIGHT;
				m_weights[i_w].lo_i = i_lo;
				i_w++;
			}
		}
	}
	int base_lev_i = m_numLayers - 1;
	for(long i_nd=0; i_nd< m_numNodesPerLayer[base_lev_i]; i_nd++){
		m_biases[i_b].lev_i = base_lev_i;
		m_biases[i_b].nd_i = i_nd;
		m_biases[i_b].w = EYENNET_DEFBIAS;
		m_biases[i_b].lo_i = -1;
		i_b++;
		for (int i_base= 0; i_base < m_numNodesBase; i_base++) {
			m_weights[i_w].lev_i = base_lev_i;
			m_weights[i_w].nd_i = i_nd;
			m_weights[i_w].w = EYENNET_DEFWEIGHT;
			m_weights[i_w].lo_i = i_base;
			i_w++;
		}
	}
	return ECODE_OK;
}