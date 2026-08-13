#pragma once
#ifndef TWISTEDPLATE_H
#define TWISTEDPLATE_H

#ifndef EXHEX_H
#include "ExHex.h"
#endif

class TwistedPlate : public Base {
public:
	unsigned char init(ExHex* exHex, float sigma/*sigma in terms of top plate R*/);
	void release();

	unsigned char spawn(s_rtHexPlate* newplate, /*const*/ s_rtHexPlate* basePlate, float radang);
	void despawn(s_rtHexPlate* plate);
	

protected:
	/*not owned*/
	ExHex* m_exHex;/*used to generate the extended plates, assume has already been initialized*/

	int m_numHanging;
	float m_sigma;/*sigma in terms of top plate R*/


	bool rootTwisted(s_rtHexPlate* topPlate, /*const*/ s_rtHexPlate* basePlate, float radang, int numHanging, float sigma/*in actual dist*/);/*gaussian root*/
	bool reorderArraysByDist(float* dists, int* indices, int n);
};
#endif
