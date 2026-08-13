#include "EyeNets.h"

unsigned char s_EyeNets::init(int num_nets) {
	if (num_nets < 1)
		return ECODE_ABORT;
	nets = new s_NNet * [num_nets];
	for (int i = 0; i < num_nets; i++)
		nets[i] = NULL;
	N = num_nets;
}

unsigned char EyeNets::init(HexStack* hexstack, Luna* luna, int num_nets) {
	if (hexstack == NULL || luna == NULL || num_nets < 1)
		return ECODE_ABORT;
	m_hexStack = hexstack;
	m_luna = luna;
	m_numNets = 0;
	m_genEyeNNet = new EyeNNet * [m_numNets];
	for (int i = 0; i < m_numNets; i++)
		m_genEyeNNet[i] = NULL;
	for (int i = 0; i < m_numNets; i++) {
		m_genEyeNNet[i] = new EyeNNet;
		m_numNets++;
		if (Err(m_genEyeNNet[i]->init(hexstack, luna)))
			return ECODE_FAIL;
	}
	return ECODE_OK;
}
unsigned char EyeNets::setWB(int net_i, s_Node_w weights[], s_Node_w biases[]) {
	if (net_i<0 || net_i >= m_numNets)
		return ECODE_ABORT;
	return m_genEyeNNet[net_i]->setWB(weights, biases);
}
unsigned char EyeNets::spawn(s_HexPlateLayer* lunaPlates, s_EyeNets* eyenets) {
	if (lunaPlates == NULL || eyenets == NULL)
		return ECODE_ABORT;
	if (Err(eyenets->init(m_numNets)))
		return ECODE_ABORT;
	for (int i_net = 0; i_net < m_numNets; i_net++) {
		EyeNNet* genNet = m_genEyeNNet[i_net];
		eyenets->nets[i_net] = new s_NNet;
		if (Err(genNet->spawn(lunaPlates, eyenets->nets[i_net])))
			return ECODE_FAIL;
	}
}