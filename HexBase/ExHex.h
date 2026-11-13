#pragma once
#ifndef EXHEX_H
#define EXHEX_H

#ifndef HEXSTACK_H
#include "HexStack.h"
#endif
#ifndef HEXRECT_H
#include "HexRect.h"
#endif
#ifndef HEXBASE_H
#include "HexBase.h"
#endif

class ExHex : public Base {
public:
	ExHex();
	~ExHex();

	unsigned char init(HexStack* stack, int numHanging);
	void release();

	unsigned char spawn(s_rtHexPlate* plate, s_2pt hexUs[] = NULL);
	void despawn(s_rtHexPlate* plate);

	inline long getNumCoreHexes() { return m_numNodes - m_numExHexes; }
	inline long getNumHexes() { return m_numNodes; }
	inline int getNumHanging() { return m_numHanging; }
	inline long getBaseSize() { return (m_Width > m_Height) ? m_Width : m_Height; }
	inline float getBaseSize_in_R() { return m_baseDim_in_R; }
	inline float getBottomR() { return m_stack->getBottomR(); }

protected:
	HexStack* m_stack;
	int m_numHanging;
	long m_numExHexes;
	long m_numNodes;
	float m_baseDim_in_R;
	long m_Width;
	long m_Height;

	float getSizeExBase(float hexstack_NumHexesLongDim);
	unsigned char copyBasePlateWeave(/*const*/ s_HexPlate* base, s_rtHexPlate* exPlate);
	unsigned char weaveExHexes(s_rtHexPlate* plate);
	unsigned char weaveRoundExHexes(s_rtHexPlate* plate);
};

#endif
