#pragma once
#ifndef L0NET_H
#define L0NET_H

#ifndef LUNALAYER_H
#include "../Luna/LunaLayer.h"
#endif

const int L0NET_c_numEyeLevels = 3;
class L0Net : public Base {
public:
	L0Net();
	~L0Net();

	unsigned char init(
		float r,/*scale r for the nets used since the HexEye records it*/
		int num_luna_plates,
		int num_L0_nets,
		int num_net_wbs=-1 /*if this is non-negative then checks it against the expected number of weights*/
	);
	void release();
	unsigned char spawn(float net_wbs[], s_CNets* L0Nets);/* net_wbs is an array of size m_num_net_wbs, by m_num_nets*/
	unsigned char spawn(s_CNets* L0Nets);/*spawns the nets but with default dummy values as the weights, can be used for training*/
	void despawn(s_CNets* L0Nets);

	inline int getTotalNumWBs() { return m_num_net_wbs; }
	inline int getTotalNumNodes_perNet() { return m_netsGen->getTotalNumNodes_perNet(); }
	inline int getTotalNumBottomOs_perNet() { return m_netsGen->getTotalNumBottomOs_perNet(); }
	inline int getNumLevels_perNet() { return m_netsGen->getNumLevels_perNet(); }
	unsigned char getNumNodesPerLevel_perNet(float levNodes[], int numLev/*check var*/);/*levNodes must be at least numLev in length*/

	inline CNets* getCNetsGen() { return m_netsGen; }
	inline sNet* getNetGen() { return m_netsGen->getNetGen(); }
private:
	int m_num_luna_plates;
	int m_num_net_wbs;/*total number of weight and offset(b) variables in a L0Net of type s_Net*/
	int m_num_nets;
	/*owned*/
	HexEye* m_genHexEye;
	CNets* m_netsGen;

	s_CNets* m_refL0Nets;

	unsigned char weaveNets(s_CNets* L0Nets);
	unsigned char connNetLinks(s_Net* snet, s_HexEye* baseEye);
	unsigned char defaultWeightsNet(s_Net* net, s_HexEye* baseEye);
	void get_wbs_line(int line_i, float wbs[], const float net_wbs[]);
};
namespace n_L0Net {
	bool run(s_CNets* L0Nets, s_HexPlateLayer* lunaPlates, s_HexPlateLayer* L0Plates, long plate_i);
}
#endif