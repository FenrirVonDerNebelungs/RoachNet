#pragma once
#ifndef HEX_H
#define HEX_H

#ifndef STRUCTS_H
#include "../Base/Structs.h"
#endif

class s_Hex : public s_Node {
public:
	s_Hex();
	s_Hex(const s_Hex& other);
	~s_Hex();

	virtual unsigned char init(long plate_index, int numHanging = 7);
	virtual unsigned char init(const s_Hex* other);
	virtual void          release();
	s_Hex& operator=(const s_Hex& other);
	inline void setRGB(float col_rgb[]) { rgb[0] = col_rgb[0]; rgb[1] = col_rgb[1]; rgb[2] = col_rgb[2]; }
	inline void setRGB(float col_r, float col_g, float col_b) { rgb[0] = col_r; rgb[1] = col_g; rgb[2] = col_b; }
	inline void setColSet() { o = 1.f; }
	inline bool colSet() { return (o > 0.5f); }
	virtual inline s_Hex* getHanging(int indx) { return (s_Hex*)nodes[indx]; }
	virtual inline s_Hex* getWeb(int indx) { return (s_Hex*)web[indx]; }
	virtual inline void setWeb(s_Hex* nd, int indx) { web[indx] = (s_Node*)nd; }
	long i;
	long j;

	s_Node* web[6];/*web will always be initialized to 6 pointers*/

	float rgb[3];
protected:
	void copy(const s_Hex* other);
};
class s_rtHex :public s_Hex {
public:
	s_rtHex();
	s_rtHex(const s_rtHex& other);
	~s_rtHex();

	virtual unsigned char init(long plate_index, int numHanging = 7);
	virtual unsigned char init(const s_rtHex* other);

	s_rtHex& operator=(const s_rtHex& other);
	inline s_rtHex* getHanging(int indx) { return (s_rtHex*)nodes[indx]; }
	inline s_rtHex* getWeb(int indx) { return (s_rtHex*)web[indx]; }
	inline void setWeb(s_rtHex* nd, int indx) { web[indx] = (s_Node*)nd; }

	inline int getDv(int web_i_dir) const { return dv[web_i_dir]; }
	inline void setDv(int web_i_dir, int _dv) {dv[web_i_dir] = _dv;}
	inline void setLocSetFlag(bool newflagval) { loc_set = newflagval; }
	inline bool getLocSetFlag() const { return loc_set; }
protected:

	int dv[3];
	bool loc_set;
};
class s_lunHex : public s_Hex {/*special hex that also contains enough info to run a fast simple net*/
public:
	s_lunHex() :w(NULL), col_i(-1) { ; }
	~s_lunHex() { ; }

	unsigned char init(long plate_index);
	unsigned char init(const s_Hex* other);
	unsigned char init(const s_lunHex* other);
	void          release();

	float* w;
	int    col_i;
};

class s_HexPlate : public s_Plate {
public:
	s_HexPlate();
	~s_HexPlate();

	virtual unsigned char init(long nNodes, int numHanging = 7);/*s_Hex nodes are created by this*/
	virtual unsigned char init(const s_HexPlate* other);/*down connections will still point to wherever the original pointed, the web is fixed however*/
	virtual unsigned char initFixDownTarget(const s_HexPlate* other);/*same as above but fixes the downlinks to the s_HexPlate "other" with each top pointed to
																	   7 down 0 in the middle and then the 6 around following the web direction*/
	void                  initRs(float inRhex);
	void                  release();/*assumes that the plate owns its subnodes if not NULL*/

	virtual inline void set(long indx, s_Hex* nd) { this->nodes[indx] = (s_Node*)nd; }
	virtual inline void set(long indx, s_Hex& nd) { this->set(indx, &nd); }
	void setWeb(long index, int web_i, long target_i);
	virtual inline s_Hex* get(long indx) { return (s_Hex*)this->nodes[indx]; }
	virtual inline s_Hex* getConst(long indx) const { return (s_Hex*)this->nodes[indx]; }
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
class s_rtHexPlate : public s_HexPlate {
public:
	s_rtHexPlate();
	~s_rtHexPlate();

	unsigned char init(long nNodes, int numHanging = 7);/*s_Hex nodes are created by this*/
	unsigned char init(const s_rtHexPlate* other);/*down connections will still point to wherever the original pointed, the web is fixed however*/

	inline void set(long indx, s_rtHex* nd) { this->nodes[indx] = (s_Node*)nd; }
	inline void set(long indx, s_rtHex& nd) { this->set(indx, &nd); }
	void setWeb(long index, int web_i, long target_i);
	inline s_rtHex* get(long indx) { return (s_rtHex*)this->nodes[indx]; }
	inline s_rtHex* getConst(long indx) const { return (s_rtHex*)this->nodes[indx]; }

};

namespace n_HexBase {
	void genHexU_0(s_2pt hexU[]);
}
#endif