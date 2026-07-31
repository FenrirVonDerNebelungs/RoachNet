#pragma once
#ifndef EXHEX_H
#define EXHEX_H

#ifndef HEXSTACK_H
#include "HexStack.h"
#endif
#ifndef HEXROOT_H
#include "HexRoot.h"
#endif

class ExHex : public Base {
public:
	ExHex();
	~ExHex();

	unsigned char init(HexStack* stack, int numHanging);
	void release();

	unsigned char spawn(s_HexPlate* plate);
	void despawn(s_HexPlate* plate);

protected:
	HexStack* m_stack;
	int m_numHanging;
	long m_numExHexes;
	long m_numNodes;

	unsigned char copyBasePlateWeave(/*const*/ s_HexPlate* base, s_HexPlate* exPlate);
	unsigned char weaveExHexes(s_HexPlate* plate);
	unsigned char weaveRoundExHexes(s_HexPlate* plate);
};

#endif
