#pragma once
#ifndef NNET_H
#define NNET_H

#ifndef LUNA_H
#include "../Luna/Luna.h"
#endif

struct s_Node_w {
	int lev_i;
	long nd_i;
	float w;
	long lo_i;
};

namespace n_Node_w {
	void zero(s_Node_w& ndw);
	void copy(s_Node_w& newNdw, const s_Node_w& orig);
}

class s_nNode : public s_Node {
public:
	s_nNode();
	s_nNode(const s_nNode& other);
	~s_nNode();

	unsigned char init(int nNodes);
	unsigned char init(const s_nNode* other);
	void release();

	s_nNode& operator=(const s_nNode& other);
	inline s_nNode* getHanging(int node_i) { return (s_nNode*)nodes[node_i]; }
	inline void setHanging(s_nNode* nd, int node_i) { this->nodes[node_i] = nd; }

	inline float getB() { return b; }
	inline void setB(float _b) { b = _b; }
protected:
	void copy(const s_nNode* other);
	float b;/*bias for this node*/
};

class s_nPlate : public s_Plate {
public:
	s_nPlate();
	~s_nPlate();

	unsigned char init(long nNodes);
	unsigned char init(const s_nPlate* other);
	void release();

	inline void set(long indx, s_nNode* nd) { this->nodes[indx] = (s_Node*)nd; }
	inline s_nNode* get(long indx) { return (s_nNode*)this->nodes[indx]; }
	inline s_nNode* getConst(long indx) const { return (s_nNode*)this->nodes[indx]; }
protected:
};

class s_NNet {
public:
	s_NNet();
	~s_NNet();
	unsigned char init(int _N_lev);
	void release();

	inline s_nPlate* getLev(int i_lev) { return lev[i_lev]; }
	inline const s_nPlate* getLevConst(int i_lev) const { return lev[i_lev]; }
	inline int getNLev() const { return N_lev; }
protected:
	s_nPlate** lev;
	int N_lev;
};


class NNet : public Base {
public:
	NNet();
	~NNet();

	unsigned char init(int N_lev, long nodesPerLev[], int N_base);
	void release();

	unsigned char spawn(s_NNet* nnet);
	void despawn(s_NNet* nnet);

	unsigned char set(s_NNet* nnet, int stream_len_w, int stream_len_bias, const s_Node_w weights[], const s_Node_w biases[]);
	int getWeights(s_Node_w weights[]);
	int getBiases(s_Node_w biases[]);
	int getNumWeights();/*get length of weights array */
	int getNumBiases();/*get length of biases array */

	inline int getNLev() { return m_N_lev; }
	inline int getNBase() { return m_N_base; }
	inline int getNLinkedBaseOs() { return m_nodesPerLev[m_N_lev - 1] * m_N_base; }


protected:
	int m_N_lev;
	long* m_nodesPerLev;
	int m_N_base;
	unsigned char weaveNet(s_NNet* nnet);
	bool weaveNodeToPlate(s_nNode* nd, /*const*/ s_nPlate* plate);

};

namespace n_NNet {
	bool run(s_NNet* net);
	float act_func(float nd_in);
}

#endif
