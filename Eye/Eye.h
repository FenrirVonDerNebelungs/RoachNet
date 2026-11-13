#pragma once
#ifndef EYE_H
#define EYE_H
#ifndef HEXIMG_H
#include "../HexedImg/HexImg.h"
#endif
#ifndef EYECORE_H
#include "EyeCore.h"
#endif


const float EYCORE_max_RGB_len_inv = 1.f / (255.f * sqrt(3.f));

class s_Eye {
public:
	s_Eye();
	~s_Eye();
	unsigned char init(int numCores);
	void release();
	s_HexStack* refHexStack;/*first hex stack in initial orientation*/
	s_rtHexPlate* rootPlate;/*owned by eye*/
	s_EyeCore** eyeCores;/*each attached to a twisted plate*/
	int N_Cores;
protected:
};

class Eye : Base {
public:
	Eye();
	~Eye();
	unsigned char init(
		HexImg* hexImg, 
		float r, 
		float twisted_root_radius, 
		float sigma/*in top plate r*/, 
		int numRot, 
		int numCurvePatterns, 
		int numStackLevels
	);
	unsigned char setWB(int net_i, s_Node_w weights[], s_Node_w biases[]);
	void release();
	unsigned char spawn(s_Eye* newEye);
	void despawn(s_Eye* eye);

	inline int getNumWeights() { return m_eyeNets->getNumWeights(); }
	inline int getNumBiases() { return m_eyeNets->getNumBiases(); }
	inline int getNLinkedBaseOs() { return m_eyeNets->getNLinkedBaseOs(); }
	inline int getNSingleNodeBaseOs() { return m_eyeNets->getNSingleNodeBaseOs(); }
	inline long getRootCenterIndex() { return m_hexRect->getCenterIndex(); }
protected:
	/*not owned*/
	HexImg* m_hexImg;
	/*owned*/
	HexStack* m_hexStack;
	rtHexRect* m_hexRect;
	TwistedPlate* m_twistedPlate;
	Luna* m_Luna;
	EyeNets* m_eyeNets;
	EyeCore* m_eyeCore;

	float m_R_base;
	float m_twisted_root_radius_in_R;/* radius per hex down from twisted plate onto root plate in R of upper plate*/
	int m_numRot;
	float m_dAng;
	float* m_Angs;

};
namespace n_Eye {
	bool run(s_Eye* eye); /*assumes Eye is already rooted*/
	float filter(float rgb[]);
}
#endif
