#pragma once
#ifndef SNET_H
#define SNET_H

#ifndef HEXEYE_H
#include "../HexedImg/HexEye.h"
#endif


//#define NUM_LUNA_EYE_LEVELS 2


 

class s_Net {
public:
	s_Net();
	~s_Net();

	unsigned char init(int nLev);/*initializes the array does not create the levels */
	virtual unsigned char init(const s_Net& other);/*assumes that if the lev pointer is non-null the copy should own the lev*/
	unsigned char newLevs();/*create the levels but don't yet init the lev plates*/
	void          delLevs();/*the level plates should already be released*/
	void          release();/*assumes that if lev ptr is not null it is owned*/

	inline s_nPlate* getTop() { return lev[0]; }
	inline s_nPlate* getBottom() { return lev[N - 1]; }
	inline s_nPlate* get(int i_lev) { return lev[i_lev]; }

	s_nPlate** lev;/*these may be owned*/
	int N;/*number of 'levels' or plates in this struct actually attached(or owned) */

	float o;/*output for this net */


protected:
	int N_mem;/*number of lev pointers*/

};
class s_HexBaseNet : public s_Net {
public:
	s_HexBaseNet();
	~s_HexBaseNet();

	virtual unsigned char init(const s_HexBaseNet& other);

	s_HexEye* eye;/*this is typically  not owned*/
	int N_Plates;/*number of plates the bottom of this sNet is attached to*/
};
class s_HexEyeNet : public s_HexBaseNet{
public:
	s_HexEyeNet();
	~s_HexEyeNet();

	unsigned char init(const s_HexEyeNet& other);

};

class s_EyeFNet : public s_HexBaseNet { /*fully connected net which will be connected at top and bottom to the eye*/
public:
	s_EyeFNet();
	~s_EyeFNet();

	unsigned char init(const s_EyeFNet& other);

};
namespace n_Net {
	void run(s_Net* net);/*requies net to be rooted*/
}
namespace n_HexBaseNet {
	inline long getPlateIndexFromRootedNetNode(s_nPlate* net_base, long net_nd_i);
	inline void rootNetNodeOnPlate(s_nPlate* net_base, long net_nd_i, s_HexPlate* plate, long rooted_plate_index);
}
namespace n_HexEyeNet {
	void rootNetFromRootedEye(s_HexBaseNet* net, s_HexPlate* plate);
}
namespace n_EyeFNet {
	void rootNetFromRootedEye(s_HexBaseNet* net, s_HexPlateLayer* plates);
	void rootNetFromRootedEye(s_HexBaseNet* net, s_HexPlate* plate, long node_offset_i, long num_plates);
}


class sNet : public Base { /* class that generates the s_Net structs 'structure' net */
public:
	sNet();
	~sNet();

	unsigned char init(int nLev, int numLevNodes[], int numHanging);
	void release();

	virtual unsigned char spawn(s_Net* sn);
	void          despawn(s_Net* sn);

	inline int getNumLevels() { return m_nLev; }
	inline int* getNumLevNodesPtr() { return m_numLevNodes; }

protected:

	int m_nLev;
	int* m_numLevNodes;/*num of nodes in each level + num of hanging nodes has length m_nLev+1*/
	int  m_numHanging;
	
};
class sHexEyeNet : public sNet {
public:
	sHexEyeNet();
	~sHexEyeNet();

	unsigned char init(HexEye* eye, int numBottomPlates_to_be_connected = 1);  /*initiates the sNet that matches the structure of a hexEye */
	unsigned char spawn(s_HexEye* eye, s_HexEyeNet* sn); /*this eye should have exactly the same structure as the eye used to initialize the sNet
													  the net should have the same structure as the eye, (web however is not currently connected)
													  Each node in the net links down to 7 nodes in the net layer beneath it (except for the bottom)
													  The hex of each node is the corresponding eye hex
													  For all levels: eye num hexes = num of net nodes
													  hanging nodes from lowest net level are set in the init */
protected:
	/*not owned*/
	HexEye* m_genHexEye;
	/*owned*/
	int  m_numBottomPlates_to_be_connected;

	unsigned char genNet(s_HexEyeNet* sn);
	unsigned char connTopNetToEye(s_HexEyeNet* sn, s_HexEye* eye);
	unsigned char connBotNetToEye(s_HexEyeNet* sn, s_HexEye* eye); /**/

};
class sEyeFNet : public sNet { /* generates fully connected nets, base for nnets*/
public:
	sEyeFNet();
	~sEyeFNet();

	unsigned char init(HexEye* eye, 
		int nLev/*total levels including top and bottom*/, 
		int numInnerLevNodes[]/*array of length nLev-2*/, 
		int numBottomPlates_to_be_connected=1);

	unsigned char spawn(s_HexEye* eye, s_EyeFNet* sn);/*spawns a fully connected net where again the structure is as set by m_nLev and m_numLevNodes
								                   for this net each node in the higher level links down by pointers to all the nodes in the next level down*/
	inline int getTotalNumWs() { return m_total_num_weights; }
	inline int getTotalNumNodes() { return m_total_num_nodes; }
	inline int getTotalNumBottomNodes() { return m_numLevNodes[m_nLev - 1]; }

	inline int getTotalNumWBs() { return m_total_num_weights + m_total_num_nodes; }
	int dumpWBsChain(s_Net* sn, float ws[]);/*dumps all the weights and bs into the ws array starting from
											   the first node on top layer and going down through the layers of the net*/
	inline int dumpWBsChainRef(float ws[]) { return dumpWBsChain(&m_refNet, ws); }
	bool importWBsChain(s_Net* sn, float wbs[], int len_wbs);/*imports the same as dumped, the chain has the order of all weights per node
																followed by the b for node then on to the next node
																this starts at level 0 and goes on*/
	int dumpOWeightLinksChain(s_Net* sn, float os[], int loc_lev[], int loc_i[]);/*each element in this chain corresponds at the index level to a w o's are duplicated and top o is not dumped
																	oloc_lev contains level of o, oloc_i contains plate index of node*/
	int dumpOsChain(s_Net* sn, float os[]);/*dumps the o's */
	int dumpOsChainLoc(s_Net* sn, s_2pt_i os[]);/*used for debuging dumps the loc relative to the hex eye if available, for not available 0,0*/
	inline int dumpOsChainLocRef(s_2pt_i os[]) { return dumpOsChainLoc(&m_refNet, os); }
	int dumpOsLinksChain(s_Net* sn, int loc_lev[], int loc_i[]);/*dumps just the o's in order but with the level info*/
	bool dumpWBsOChainMatch(s_Net* sn, int oloc_i[], int wblen);/*takes array of len WBs and for each w finds the index of the corresponding lower o in the o chain*/

protected:
	/*not owned*/
	HexEye* m_genHexEye;
	/*owned*/
	int m_numBottomPlates_to_be_connected;

	s_EyeFNet m_refNet;/*net that is created during init and can be used in functions to calculate various dimentions*/
	int   m_total_num_weights;/*total number of weights for all levels*/
	int   m_total_num_nodes;/*total number of nodes and also equivalently total number of offset constants for all levels*/

	unsigned char genNet(s_EyeFNet* sn);/*generates the fully connected net*/
	unsigned char connDownNet(s_Net* sn);/* fully connected links
											links each lev/plate to lower plate such that each high node links to all low nodes
	                                        lower nodes are in same order as their indexes on the plate*/

	unsigned char connTopNetToEye(s_EyeFNet* sn, s_HexEye* eye);
	unsigned char connBotNetToEye(s_EyeFNet* sn, s_HexEye* eye); /*connects the hanging nodes to the bottom plate nodes of the hex eye,
																   all connections are done in the same order
																  each numBottomPlates_.. block of hanging nodes in sequence is connected to the same
																  hex eye node. The start of the block index increases by numBottomPlates.. and the 
																  next hex eye node is selected for connection untill all 
																  (numBottomPlates...)* (num bottom hex eye nodes) hanging nodes are connected */

	unsigned char initSetNumWeightsAndNodes();
	unsigned char getNumWeightsAndNodesPerEyeLevel(int level, int& total_num_weights, int& total_num_nds);

};


#endif