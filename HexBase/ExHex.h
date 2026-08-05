#pragma once
#ifndef EXHEX_H
#define EXHEX_H

#ifndef HEXSTACK_H
#include "HexStack.h"
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

	unsigned char spawn(s_rtHexPlate* plate);
	void despawn(s_rtHexPlate* plate);

protected:
	HexStack* m_stack;
	int m_numHanging;
	long m_numExHexes;
	long m_numNodes;

	unsigned char copyBasePlateWeave(/*const*/ s_HexPlate* base, s_rtHexPlate* exPlate);
	unsigned char weaveExHexes(s_rtHexPlate* plate);
	unsigned char weaveRoundExHexes(s_rtHexPlate* plate);
};

#endif
