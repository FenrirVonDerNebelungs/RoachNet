#include "EyeNNet.h"

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
	return ECODE_OK;
}
unsigned char EyeNNet::setWB(s_Node_w weights[], s_Node_w biases[]) {
	for (int i = 0; i < m_len_weights; i++)
		n_Node_w::copy(m_weights[i], weights[i]);
	for (int i = 0; i < m_len_biases; i++)
		n_Node_w::copy(m_biases[i], biases[i]);
	return ECODE_OK;
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
		int i_lo = 0;
		for (int i_luna_plate = 0; i_luna_plate < lunaPlates->N; i_luna_plate++) {
			s_HexPlate* luna_plate = lunaPlates->get(i_luna_plate);
			for (long i_luna_nd = 0; i_luna_nd < luna_plate->N; i_luna_nd++) {
				nd->nodes[i_lo] = (s_Node*)luna_plate->getNd(i_luna_nd);
			}
		}
	}
	/*fill the inital weights and biases */
	unsigned char err = NNet::set(nnet, m_len_weights, m_len_biases, m_weights, m_biases);
	return err;
}

int EyeNNet::getLinkedBaseOs(const s_NNet* nnet, s_Node_w os[]) {
	if (nnet == NULL || nnet->getNLev() < 1)
		return -1;
	int num_stream = 0;
	int lev_i = m_N_lev - 1;
	const s_nPlate* net_plate = nnet->getLevConst(lev_i);
	for (long i_nd = 0; i_nd < net_plate->N; i_nd++) {
		const s_nNode* nd = net_plate->getConst(i_nd);
		for (int i_lo = 0; i_lo < nd->N; i_lo++) {
			const s_Node* lo_nd = nd->nodes[i_lo];
			float o = lo_nd->o;
			os[num_stream].lev_i = lev_i;
			os[num_stream].nd_i = i_nd;
			os[num_stream].w = o;
			os[num_stream].lo_i = i_lo;
			num_stream++;
		}
	}
	return num_stream;
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
	m_numLayers = numNNetLevels;
	m_numNodesPerLayer = new long[numNNetLevels] ;
	for (int i_lev = 0; i_lev < numNNetLevels; i_lev++) {
		int stack_index = i_lev;
		m_numNodesPerLayer[i_lev] = m_hexStack->getNHexes(stack_index);
	}
	return ECODE_OK;
}
