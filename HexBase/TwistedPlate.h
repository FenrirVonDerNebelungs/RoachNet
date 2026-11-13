#pragma once
#ifndef TWISTEDPLATE_H
#define TWISTEDPLATE_H

#ifndef HEXIMG_H
#include "../HexedImg/HexImg.h"
#endif

#ifndef EXHEX_H
#include "ExHex.h"
#endif

const float TWISTEDPLATE_root_plate_buffer_factor = 1.3f;
const int TWISTEDPLATE_min_num_hanging = 7;

class TwistedPlate : public Base {
public:
	TwistedPlate();
	~TwistedPlate();

	unsigned char init(HexImg* hexImg, HexStack* stack, float sigma/*sigma in terms of top plate R*/, float twisted_root_radius_in_R);
	void release();

	unsigned char spawn(s_rtHexPlate* newplate, /*const*/ s_rtHexPlate* basePlate, float radang);
	void despawn(s_rtHexPlate* plate);
	
	inline float getSizeRootBase() {
		return m_sizeRootBase;
	}
	inline ExHex* getExHex() { return m_exHex; }
protected:
	/*not owned*/
	HexImg* m_hexImg;/*not owned, used to get Rhex for computing size of root base*/

	/*owned*/
	ExHex* m_exHex;/*used to generate the extended plates,*/

	int m_numHanging;
	float m_sigma;/*sigma in terms of top plate R*/
	float m_twistedRadius;
	float m_sizeRootBase;
	long m_Width;
	long m_Height;

	int getNumhangingBaseToRoot();

	void genTwistedUs(s_rtHexPlate* basePlate, float radang, s_2pt hexUs[]);
	bool rootTwisted(s_rtHexPlate* topPlate, /*const*/ s_rtHexPlate* basePlate, int numHanging, float sigma/*in actual dist*/);/*gaussian root*/
	bool reorderArraysByDist(float* dists, int* indices, int n);
};

namespace n_TwistedPlate{
		bool run(s_rtHexPlate* twistedPlate);
}
#endif
