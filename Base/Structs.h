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
	int N; /* prob 32 bit */

	float o;/*used as colset flag for hex nodes*/

protected:
	virtual void copy(const s_Node* other);
	int N_mem;/*lenght of nodes pointer array in memory*/
};

class s_Hex : public s_Node {
public:
	s_Hex();
	s_Hex(const s_Hex& other);
	~s_Hex();

	virtual unsigned char init(long plate_index, int numHanging=7);
	virtual unsigned char init(const s_Hex* other);
	virtual void          release();
	s_Hex& operator=(const s_Hex& other);
	inline void setRGB(float col_rgb[]) { rgb[0] = col_rgb[0]; rgb[1] = col_rgb[1]; rgb[2] = col_rgb[2]; }
	inline void setRGB(float col_r, float col_g, float col_b) { rgb[0] = col_r; rgb[1] = col_g; rgb[2] = col_b; }
	inline void setColSet() { o = 1.f; }
	inline bool colSet() { return (o > 0.5f); }
	inline s_Hex* getHanging(int indx) { return (s_Hex*)nodes[indx]; }

	long i;
	long j;

	s_Node* web[6];/*web will always be initialized to 6 pointers*/

	float rgb[3];
protected:
	void copy(const s_Hex* other);
};
class s_lunHex : public s_Hex {/*special hex that also contains enough info to run a fast simple net*/
public:
	s_lunHex() :w(NULL), col_i(-1) { ; }
	~s_lunHex() {;}

	unsigned char init(long plate_index);
	unsigned char init(const s_Hex* other);
	unsigned char init(const s_lunHex* other);
	void          release();

	float* w;
	int    col_i;
};
class s_nNode : public s_Node {/*nnet node*/
public:
	s_nNode();
	~s_nNode();

	unsigned char init(int nNodes);
	unsigned char init(const s_nNode* other);
	void          release();
	void          reset();
	s_nNode& operator=(const s_nNode& other);
	inline s_nNode* getHanging(int node_i) { return (s_nNode*)this->nodes[node_i]; }
	inline void setHanging(s_nNode* nd, int node_i) { this->nodes[node_i] = nd; }


	s_Hex* hex;/*hex node this NNet node  is attached to if such is linked*/

	float* w;/*w will have the length of nodes, N */
	float b;
private:
	void copy(const s_nNode* other);
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
class s_HexPlate : public s_Plate{
public:
	s_HexPlate();
	~s_HexPlate();

	unsigned char         init(long nNodes, int numHanging=7);/*s_Hex nodes are created by this*/
	unsigned char init(const s_HexPlate* other);/*down connections will still point to wherever the original pointed, the web is fixed however*/
	virtual unsigned char initFixDownTarget(const s_HexPlate* other);/*same as above but fixes the downlinks to the s_HexPlate "other" with each top pointed to 
	                                                                   7 down 0 in the middle and then the 6 around following the web direction*/
	void                  initRs(float inRhex);
	void                  release();/*assumes that the plate owns its subnodes if not NULL*/

	inline void set(long indx, s_Hex* nd) { this->nodes[indx] = (s_Node*)nd; }
	inline void set(long indx, s_Hex& nd) { this->set(indx,&nd); }
	void setWeb(long index, int web_i, long target_i);
	inline s_Hex* get(long indx) { return (s_Hex*)this->nodes[indx]; }
	inline s_Hex* getConst(long indx) const { return (s_Hex*)this->nodes[indx]; }
	inline s_Hex  getCopy(long indx) { return *((s_Hex*)this->nodes[indx]); }
	bool inHex(const long hexNode_i, const s_2pt& pt, const float padding = 0.f) const;

	long height;
	long width;
	float Rhex;
	float RShex;
	float Shex;
	s_2pt hexU[6];
protected:
	virtual void reset();
	void genHexU_0();
};
namespace n_HexPlate {
	void genHexU_0(s_2pt hexU[]);
	int rotateCLK(const s_Hex* hexNode, const int start_web_i);/*rotates cclk with y axis reversed*/
	int rotateCCLK(const s_Hex* hexNode, const int start_web_i);
	bool indexChainRoot(s_HexPlate* root, s_HexPlate* base, s_Hex* root_first_node, s_Hex* base_first_node, long hex_start_i, long hex_end_i);/*hex_start_i corresponds to the index of root_first_node, hex_end_i is one beyond what will be set*/
	int getWebDir(s_Hex* start_nd, s_Hex* end_nd, int& web_start_i);

	s_Hex* connLineStackedPlates(s_Hex* nd_hi, s_Hex* nd_lo, int next_web_i);/* puts the low nodes in a line along the direction
																			    determined by nex_web_i into nodes[0] of the hexes of the hi plate
																				returns last hi node at the end of the line of hi nodes */
	int turnCornerStackedPlates(s_Hex** nd_hi, s_Hex** nd_lo, int next_web_i, int fwd_web_i=0, int rev_web_i=3);/* turns around a corner for at the end of a
																						     line in a web linked hex mesh
																							 returns the new direction and fills
																							 the hi/low node ptrs with next pair of hexes*/
	long countNumHexesInLine(long start_i, int dir_web_i, s_HexPlate* o); /*counts the number of hexes in line moving in the web direction defined by dir_web_i from start_i including start_ in the number*/
	unsigned char pool2init(s_HexPlate* o, s_HexPlate* pool);/*creates a new hexPlate from the original, o, hexplate with the new plate reduced by a factor of 2
															     each of the new down links points to the 7 hexes on the o plate that the new plate is above*/
	//long xyToHexi(const s_2pt& xy);
}

/** classes below may be replaced or deleted later **/
class s_HexBasePlate : public s_HexPlate {
public:
	s_HexBasePlate();
	~s_HexBasePlate();

	unsigned char init(const s_HexBasePlate* other);

	unsigned char initRowStart(long rowN);
	void          releaseRowStart();

	long  N_wHex;
	long  N_hHex;


	/*for fast scan xy to plate loc for square plate configuration*/
	s_2pt* RowStart; /*xy position of the first hex in each row*/
	s_2pt_i* RowStart_is;/* index in the hex array of the row start, and number in row*/
	long   Row_N;
	float  Col_d;/*seperation between columns 2/3 * R */
	float  Row_d;/*seperation between rows 2*RS */
protected:
	void reset();
};
namespace n_HexBasePlate {
	unsigned char initHexBasePlate_from_HexPlate(s_HexBasePlate* p);
	unsigned char pool2init(s_HexBasePlate* o, s_HexBasePlate* pool);
}

class s_nPlate : public s_Plate {
public:
	s_nPlate() :num_hanging(0) {;}
	~s_nPlate() { ; }
	unsigned char init(long nNodes) { return s_Plate::init(nNodes); }
	unsigned char init(long nNodes, int nLowerNodes);/*Initializes the plate with nodes it owns */
	unsigned char init(const s_nPlate* other);
	unsigned char init(s_HexPlate* hex_plate, int nLowerNodes);/*initializes a plate with the dimensions of the hex plate
															  and with the nodes set to the hex plate nodes
															  nodes of this plate are owned by this plate */
	/*release is same as for s_Plate, since s_Plate releases and deletes non-null nodes*/

	unsigned char setHexes(s_HexPlate* hex_plate);/*requires hex plate to have the same dimensions as this plate,
												  sets all nNodes of this plate to the hex nodes of the hex plate*/
	inline void set(long indx, s_nNode* nd) { this->nodes[indx] = (s_Node*)nd; }
	inline void set(int indx, s_nNode* nd) { this->nodes[indx] = (s_Node*)nd; }
	inline s_nNode* get(long indx) { return (s_nNode*)this->nodes[indx]; }
	inline s_nNode* getConst(long indx) const { return (s_nNode*)this->nodes[indx]; }
	inline int getNumHanging() const{ return num_hanging; }
private:
	int num_hanging;
};

class s_HexPlateLayer {
public:
	s_HexPlateLayer();
	~s_HexPlateLayer();
	unsigned char init(int Nplates);/*just initializes the pointers for s_HexPlate and sets N_mem*/
	unsigned char init(const s_HexPlateLayer* pl);
	void          release();
	virtual inline s_HexPlate* get(int indx) { return p[indx]; }
	inline int getNmem() { return N_mem; }
	s_HexPlate** p;
	int N;
protected:
	int N_mem;
};
class s_HexBasePlateLayer : public s_HexPlateLayer {
public:
	s_HexBasePlateLayer() { ; }
	~s_HexBasePlateLayer() { ; }
	inline s_HexBasePlate* get(int indx) { return (s_HexBasePlate*)p[indx]; }
};
#endif