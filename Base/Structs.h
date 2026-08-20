#pragma once
#ifndef STRUCTS_H
#define STRUCTS_H

#ifndef BASE_H
#include "Base.h"
#endif

class s_Node {/*prob 44 bytes*/
public:
	s_Node();
	s_Node(const s_Node& other);/*this is only designed to work if the nodes are NOT owned*/
	~s_Node();

	virtual unsigned char init(int nNodes);
	virtual unsigned char init(const s_Node* other);
	virtual void release();
	unsigned char genSubNodes();
	void          releaseSubNodes();
	virtual void reset();

	virtual s_Node& operator=(const s_Node& other);/*the node pointers from nodes are copied but not owned by the copy
										   the size of nodes in memory must be equal for 
										   the node pointers to be copied */
	inline int getNmem() { return N_mem; }
	inline virtual s_Node* getHanging(int node_i) { return this->nodes[node_i]; }
	inline virtual void setHanging(s_Node* nd, int node_i) { this->nodes[node_i] = nd; }

	float x; /* prob 64 bit*/
	float y;

	long thislink; /* prob 64 bit*/
	s_Node** nodes;/*lower nodes*//* prob 64 bit*/
	float* w;/*weights for lower nodes*/
	int N; /* prob 32 bit */

	float o;/*used as colset flag for hex nodes*/

protected:
	virtual void copy(const s_Node* other);
	int N_mem;/*lenght of nodes pointer array in memory*/
};

class s_Plate {
public:
	s_Plate();
	~s_Plate();

	virtual unsigned char init(long nNodes);
	virtual unsigned char init(const s_Plate* other);
	virtual void          release();/*assumes that the plate owns its subnodes if not NULL*/

	inline virtual void set(long indx, s_Node* nd) { this->nodes[indx] = nd; }
	inline virtual s_Node* get(long indx) { return this->nodes[indx]; }
	inline virtual s_Node* getConst(long indx) const { return this->nodes[indx]; }
	inline s_Node* getNd(long indx) { return this->nodes[indx]; }
	inline s_Node** getNodes() { return this->nodes; }

	s_Node** nodes;
	long    N;
protected:
	long N_mem;
	virtual void reset();
};
namespace n_Plate {
	unsigned char fixStackedPlateLinks(s_Plate* topP, s_Plate* botP);
}
#endif