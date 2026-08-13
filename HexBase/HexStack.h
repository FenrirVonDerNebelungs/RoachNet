#pragma once
#ifndef HEXSTACK_H
#define HEXSTACK_H

#ifndef HEX_H
#include "Hex.h"
#endif


class s_HexStack {
public:
	s_HexStack();
	~s_HexStack();

	unsigned char init(int NumLev);
	unsigned char init(const s_HexStack& other);
	void          release();

	inline s_HexPlate* get(int nLev) { return lev[nLev]; }
	inline s_HexPlate* getBottom() { return lev[N - 1]; }
	inline void set(int nLev, s_HexPlate* pLev) { lev[nLev] = pLev; }
	inline bool append(s_HexPlate* pLev) { if (N < N_mem) { lev[N] = pLev; N++; return true; } return false; }

	inline float r() { if (N > 0) { return lev[N - 1]->Rhex; } return 0.f; }
	inline float rs() { if (N > 0) { return lev[N - 1]->RShex; } return 0.f; }
	inline float R() { if (N > 0) { return lev[0]->Rhex; } return 0.f; }
	inline float RS() { if (N > 0) { return lev[0]->RShex; } return 0.f; }
	inline s_2pt* hexU() { if (N > 0) { return lev[0]->hexU; } return NULL; }

	s_HexPlate** lev;/*s_HexPlates here are owned by the s_HexEye*/
	int N;

	long width;
	long height;

	float w_col_center; /*weights that add up to 1 that are used to compute the colors of the higher levels*/
	float w_col_aux;
protected:
	int N_mem;
};

class HexStack : public Base {
public:
	HexStack();
	~HexStack();

	unsigned char init(float r, int NLevels);/*1 level = 1 hex, 2 levels a 1 hex and a 7 hex level, etc...*/
	void          release();

	unsigned char spawn(s_HexStack* neye);/*spawn and put the results in neye eye should be new object but not initalized*/
	void          despawn(s_HexStack* neye);/*cleans up eye*/

	inline int getNLevels() { return m_N_levels; }
	inline long getNHexes(int i) { return m_N_hexes[i]; }
	inline long getNHexesBottom() { return m_N_hexes[m_N_levels - 1]; }

	s_HexStack& getHexStack() { return m_refEye; }
	s_HexStack* getHexStackPtr() { return &m_refEye; }

	inline float getBottomR() { return m_r; }

	inline long getBaseWidth() { return m_baseWidth; }
	inline long getBaseHeight() { return m_baseHeight; }
	float getNumHexesLongDim(int N_level=-1);

	int getNumEdgeHexes(int i_level);
	inline int getNumEdgeHexesBottom() { return getNumEdgeHexes(m_N_levels - 1); }

	s_HexPlate* getBottomPlate() { return m_refEye.getBottom(); }
protected:
	/* dimensions of eyes this HexEye will initiate */
	float m_r;
	float m_R;/*largest R*/
	int   m_N_levels;
	s_2pt m_hexU[6];
	long* m_N_hexes;

	long m_baseWidth;
	long m_baseHeight;
	s_HexStack m_refEye;/*hex eye that is spawned at the end of init used as a reference for coordinates*/
	/*                                            */
	unsigned char genNumHexesPerLevel();
	/*                                            */
	unsigned char initStack(s_HexStack* neye);
	void          releaseStack(s_HexStack* neye);
	/* helpers to init Stack */
	long numHexInLevel(long N_prev, int N_level);
	float sizeOfLevel(int N_level);/*N_level starts at 0 */

	unsigned char genEye(s_HexStack* neye);
	/* helpers to genEye             */
	int collectNebIndexes(s_HexPlate* lev, int i_top, s_2pt_i neb[]);
	void genLowerPattern(s_Node* lev_nds[], float Rs, s_2pt& loc, s_2pt_i neb[], int num_neb, int& indx);
	/*** helpers to genEye helpers ***/
	int getNebLevIndex(s_2pt_i neb[], int num_neb, int& web_i);
	void weaveRound(s_Hex* nd);
	int foundInAr(s_2pt_i ar[], int n, int val);
	/*                               */
};

#define HEXEYE_MAXNEBINDXS 6 
#define HEXEYE_RDIFFTOL 0.0001f
#define HEXEYE_MAXIMGFITLEVEL 20
const float HEXEYE_W_RATIO_COL_SUB_CENTER = 1.5f; /*weight of center in color transfer is this faction vs the weight of the cols of the 6 outer nodes*/
const int HEXEYE_NUM_LOWER_NODES_PER_HEX = 7;

struct s_highConvKernVars {/*struct is used to fill colors in eye's higher levels*/
	float w_center;/*weight of center node*/
	float w_aux;/*weight of the 6 nodes surrounding the center 6*w_aux+w_center=1 */

	long hex_index;
	long num_Hex;/*number of top hexes*/
	s_Node** Hexes;/*top hexes*/
};



namespace n_HexEye{
	unsigned char imgRoot(s_HexEye* eye, s_HexPlate* pImg, long center_i);/*roots on img, the hexEye should be one level above the img*/
	bool check_imgRoot(s_HexEye* eye, s_HexPlate* pImg);/*checks if the geometry is correct for the hex eye to root onthe img*/
	unsigned char imgRootL2(s_HexEye* eye, s_HexPlate* pImg, long center_i);/*roots on the image plate, requires the eye to have only 2 levels*/
	bool check_imgRootL2(s_HexEye* eye, s_HexPlate* pImg);

	unsigned char updateCol(s_HexEye* eye);/*assumes the base of the eye has been filled with the correct colors, updates the colors in the higher levels*/

	unsigned char rootUnderEyeHex(s_HexEye* eye, const s_HexEye* overEye, const int lev_i, const int hex_i);
	/*helper to root Under Eye*/
	bool recursiveTraceEyeHexDown(s_Hex* tr_hex, s_Hex* over_tr_hex, int trace_level_N, long down_i_trace[]);

	/*helper to updateCol*/
	unsigned char runEyeLevel(s_HexEye* seye, int i_level);/*index of level starting at 0*/
	unsigned char runEyeLevelSingleThread(s_highConvKernVars IOVars);
	void convHexKernel(s_highConvKernVars IOVars);
}


#endif
