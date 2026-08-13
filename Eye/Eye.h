#pragma once
#ifndef EYE_H
#define EYE_H
#ifndef HEXIMG_H
#include "../HexedImg/HexImg.h"
#endif
#ifndef EYECORE_H
#include "EyeCore.h"
#endif

const float EYECORE_root_plate_buffer_factor = 1.3;

class s_Eye {
public:
	s_HexStack* m_refHexStack;/*first hex stack in initial orientation*/
	s_rtHexPlate* m_rootPlate;/*owned by eye*/
	s_EyeCore** m_eyeCores;/*each attached to a twisted plate*/
protected:
};

class Eye : Base {
public:
	Eye();
	~Eye();
	unsigned char init(HexImg* hexImg, float r, float twisted_root_radius, float sigma/*in top plate r*/, int numRot, int numStackLevels);
	unsigned char setWB(int net_i, s_Node_w weights[], s_Node_w biases[]);
	void release();
	unsigned char spawn(s_Eye* newEye);
	void despawn(s_Eye* eye);
protected:
	/*not owned*/
	HexImg* m_hexImg;
	/*owned*/
	HexStack* m_hexStack;
	rtHexRect* m_hexRect;
	ExHex* m_exHex;
	TwistedPlate* m_twistedPlate;
	Luna* m_Luna;
	EyeNets* m_eyeNets;
	EyeCore* m_eyeCore;

	float m_twisted_root_radius;/* radius per hex down from twisted plate onto root plate in R of upper plate*/

	float m_stack_maxDim;
	float m_stack_ex_maxDim;/*dim of stackBasePlate*/
	float m_eye_view_root_maxDim;/*dim of root plate*/
	int m_numHanging_stackBasePlate;/*num hanging from twisted plate*/

	/*init helper*/
	float getSizeExBase();
	float getSizeRootBase();
	int getNumhangingBaseToRoot();
};
#endif
