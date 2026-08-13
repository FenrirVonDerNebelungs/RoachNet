#pragma once
#ifndef EYECORE_H
#define EYECORE_H

#ifndef HEXRECT_H
#include "../HexBase/HexRect.h"
#endif
#ifndef TWISTEDPLATE_H
#include "../HexBase/TwistedPlate.h"
#endif
#ifndef EYENETS_H
#include "EyeNets.h"
#endif

class s_EyeCore {
public:
	s_EyeCore();
	~s_EyeCore();

	s_rtHexPlate* twistedPlate;/*twisted plate*/
	s_HexPlateLayer* lunaPlates;
	s_EyeNets* nets;/*nnets with the filters*/
protected:
};

class EyeCore : Base {
public:
	EyeCore();
	~EyeCore();
	unsigned char init(TwistedPlate* twistedplate, Luna* luna, EyeNets* genEyeNet);
	void release();
	unsigned char spawn(float angRad, /*const*/ s_rtHexPlate* basePlate, s_EyeCore* newEyeCore);
	void despawn(s_EyeCore* eyeCore);
protected:
	/*not owned*/
	TwistedPlate* m_twistedPlate;
	Luna* m_luna;
	EyeNets* m_genEyeNet;
};

#endif