#pragma once
#ifndef HEXRECT_H
#define HEXRECT_H

#ifndef HEXBASE_H
#include "HexBase.h"
#endif

namespace n_HexRect {
	float tangR(float r) { return r * sqrtf(3.f) / 2.f; }

	int minSpan(float minCovered_width, float r);
	int minSpanEven(float minCovered_width, float r) {
		int tryNumSpan = minSpan(minCovered_width, r); if (tryNumSpan % 2 != 0) tryNumSpan++; return tryNumSpan;
	}
	int minLines(float minCovered_height, float r);
	int minLinesOdd(float minCovered_height, float r) {
		int tryNumLines = minLines(minCovered_height, r); if (tryNumLines % 2 == 0) tryNumLines++; return tryNumLines;
	}
	int maxSpan(float max_width, float r);
	int maxLines(float max_height, float r);
}

class HexRect :public Base {
public:
	HexRect();
	~HexRect();

	virtual unsigned char init(int numHexSpanLong, int numHexLines, float r=3.f, int numHanging=1);

	virtual unsigned char spawn(s_HexPlate* plate);
	virtual void despawn(s_HexPlate* plate);

protected:

	int m_numSpan; /*long span*/
	int m_numInnerSpan;
	int m_numLines;
	int m_numHexes;
	int m_numHanging;

	float m_minWidth;
	float m_maxWidth;
	float m_minHeight;
	float m_maxHeight;

	unsigned char constructPlate(s_HexPlate* plate);

	unsigned char layDownSpan_and_Weave(s_HexPlate* plate, long span_start_i, long numSpan); /*lays down a single span of hexes and weaves them together*/ 
	unsigned char weaveLines(s_HexPlate* plate, long span1_start_i, long span2_start_i);/*weave top line (1) to bottom line (2) assume top is long and bottom is short*/
	unsigned char weaveLinesUpsidedown(s_HexPlate* plate, long span1_start_i, long span2_start_i);/*weave top line (1) to bottom line (2) assume top is short and bottom is long*/
};
class rtHexRect :public HexRect {
public:
	unsigned char init(int numHexSpanLong, int numHexLines, float r = 3.f, int numHanging = 1);

	unsigned char spawn(s_rtHexPlate* plate); 
	void despawn(s_rtHexPlate* plate);

	long getCenterIndex() { return m_centerIndex; }
	bool getExactCenterIndex() { return m_exactCenterIndex; }
protected:
	long m_centerIndex; /*either index of hex in center or index of hex one line off center*/
	bool m_exactCenterIndex; /*true if m_centerIndex is the exact center hex, false if it is one line off center*/
	unsigned char setCenterIndex();
};
#endif
	