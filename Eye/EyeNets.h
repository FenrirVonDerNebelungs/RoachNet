#pragma once
#ifndef EYENETS_H
#define EYENETS_H

#ifndef EYENNET_H
#include "EyeNNet.h"
#endif

class s_EyeNets {
public:
	s_EyeNets();
	~s_EyeNets();
	unsigned char init(int num_nets);
	void release();

	s_NNet** nets;/*nets trained to look for particular edge shape*/
	int N;/*number of target nets*/
protected:
};

class EyeNets : public Base {
public:
	EyeNets();
	~EyeNets();

	unsigned char init(HexStack* hexstack, Luna* luna, int num_nets);
	unsigned char setWB(int net_i, s_Node_w weights[], s_Node_w biases[]);
	unsigned char spawn(/*const*/s_HexPlateLayer* lunaPlates, s_EyeNets* eyenets);
	void despawn(s_EyeNets* eyenets);

protected:
	/*not owned*/
	HexStack* m_hexStack;
	Luna* m_luna;

	/*owned*/
	EyeNNet** m_genEyeNNet;
	int m_numNets;

};

#endif