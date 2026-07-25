#pragma once
#ifndef HEXEYE_H
#define HEXEYE_H

#ifndef STRUCTS_H
#include "../Base/Structs.h"
#endif
#ifndef IMG_H
#include "../Base/Img.h"
#endif

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

class s_HexEye {
public:
	s_HexEye();
	~s_HexEye();

	unsigned char init(int NumLev);
	unsigned char init(const s_HexEye& other);
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

class HexEye : public Base {
public:
	HexEye();
	~HexEye();

	unsigned char init(float r, Img* im);/*init an eye generator so that it is the largest eye that will fit in this image*/
	unsigned char init(float r, int NLevels);/*1 level = 1 hex, 2 levels a 1 hex and a 7 hex level, etc...*/
	void          release();

	unsigned char spawn(s_HexEye* neye);/*spawn and put the results in neye eye should be new object but not initalized*/
	void          despawn(s_HexEye* neye);/*cleans up eye*/

	inline int getNLevels() { return m_N_levels; }
	inline long getNHexes(int i) { return m_N_hexes[i]; }

	s_HexEye& getHexEye() { return m_refEye; }
	s_HexEye* getHexEyePtr() { return &m_refEye; }

	inline float getBottomR() { return m_r; }

	inline long getBaseWidth() { return m_baseWidth; }
	inline long getBaseHeight() { return m_baseHeight; }
protected:
	/* dimensions of eyes this HexEye will initiate */
	float m_r;
	float m_R;/*largest R*/
	int   m_N_levels;
	s_2pt m_hexU[6];
	long*  m_N_hexes;

	long m_baseWidth;
	long m_baseHeight;
	s_HexEye m_refEye;/*hex eye that is spawned at the end of init used as a reference for coordinates*/
	/*                                            */
	unsigned char genNumHexesPerLevel();
	/*                                            */
	unsigned char initEye(s_HexEye* neye);
	void          releaseEye(s_HexEye* neye);
	/* helpers to init Eye */
	long numHexInLevel(long N_prev, int N_level);
	float sizeOfLevel(int N_level);/*N_level starts at 1 here not the index of the level*/
	int largestN_level(long img_size);/*the top of the level stack has radius m_R see how many levels can be built up before the eye is too large
	                                    returns level_N as index of level where levels are counted starting at 0 for the 1 hex level*/
	/*                     */

	unsigned char genEye(s_HexEye* neye);
	/* helpers to genEye             */
	int collectNebIndexes(s_HexPlate* lev, int i_top, s_2pt_i neb[]);
	void genLowerPattern(s_Node* lev_nds[], float Rs, s_2pt& loc, s_2pt_i neb[], int num_neb, int& indx);
	/*** helpers to genEye helpers ***/
	int getNebLevIndex(s_2pt_i neb[], int num_neb, int& web_i);
	void weaveRound(s_Hex* nd);
	int foundInAr(s_2pt_i ar[], int n, int val);
	/*                               */
};
#endif
