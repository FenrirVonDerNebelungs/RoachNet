#pragma once
#ifndef L0LAYER_H
#define L0LAYER_h

#ifndef L0NET_H
#include "L0Net.h"
#endif

class L0Layer : public Base {
public:
	L0Layer();
	~L0Layer();

	unsigned char init(LunaLayer* genLunaLayer, float net_wbs_stack[], int len_net_wbs, int num_L0_nets);
	void release();

	unsigned char spawn(s_HexPlateLayer* lunaPlates, s_HexPlateLayer* L0Plates);/*L0Plates has been created but not initalized
																					the L0Plates 7 down links for each hex will point to wherever
																					the lunaPlates down links pointed, which is typically a color plate*/
	void despawn(s_HexPlateLayer* L0Plates);
	unsigned char run(s_HexPlateLayer* lunaPlates, s_HexPlateLayer* L0Plates);/*luna plates must all have same dim, and L0Plates should be the ones generated with spawn
																				currently this is not threaded */

	inline sNet* getNetGen() { return m_genL0Nets->getNetGen(); }
	inline s_CNets* getL0Nets() { return m_L0Nets; }
private:
	int m_num_luna;
	int m_num_L0_nets;
	int m_num_net_wbs;
	/*owned*/
	L0Net* m_genL0Nets;
	/*** owned data tokens that contain the net structures ***/
	s_CNets* m_L0Nets;
	/*init helpers*/

	unsigned char spawnPlateFromLunaLayer(s_HexPlateLayer* lunaPlates, s_HexPlate* p);
};

namespace n_L0Layer {
	inline bool run(L0Layer* genL0, s_HexPlateLayer* lunaPlates, s_HexPlateLayer* L0Plates, long plate_i);
}
#endif