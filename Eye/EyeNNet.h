#pragma once
#ifndef EYENNET_H
#define EYENNET_H
#ifndef NNET_H
#include "../NNet/NNet.h"
#endif

#ifndef HEXSTACK_H
#include "../HexBase/HexStack.h"
#endif


class EyeNNet : public NNet {
public:
	EyeNNet();
	~EyeNNet();

	unsigned char init(HexStack* hexstack, Luna* luna);
	unsigned char setWB(s_Node_w weights[], s_Node_w biases[]);
	unsigned char spawn(/*const*/s_HexPlateLayer* lunaPlates, s_NNet* nnet);
	void despawn(s_NNet* nnet);

	int getLinkedBaseOs(const s_NNet* nnet, s_Node_w os[]);/*get the O values of the nodes that the hanging links are attached to*/
private:
	/*not owned*/
	HexStack* m_hexStack;
	Luna* m_luna;

	long* m_numNodesPerLayer;
	int  m_numLayers;
	int  m_numNodesBase;

	int m_len_weights;
	s_Node_w* m_weights;
	int m_len_biases;
	s_Node_w* m_biases;

	unsigned char initNumNodes();
};
#endif