#pragma once
#ifndef HEXBASE_H
#define HEXBASE_H

#ifndef HEX_H
#include "Hex.h"
#endif

namespace n_HexBase {
	/*helpers*/
	int revWeb_i(int web_i);/*returns the web index that is opposite to the input web index*/
	int Web_i(int web_i);
	int Web_dir(s_Hex* hex1, s_Hex* hex2);/*returns the web index of hex1 that points to hex2, returns -1 if not found*/
	int rotateCLK(const s_Hex* hexNode, const int start_web_i);/*rotates cclk with y axis reversed*/
	int rotateCCLK(const s_Hex* hexNode, const int start_web_i);
	bool stichHexes(s_Hex* hex1, s_Hex* hex2, int web_i1);/*stiches two hexes together at the web_i index of hex1 pointing to hex2*/
	int stichTogetherWebHexes(s_Hex* centerhex); /*stitches together all hexes connected by the centerhex if they haven't been stiched yet*/
	bool weaveSharedHexToNeighbors(s_Hex* commonHex, /*const*/ s_Hex* hex1, /*const*/ s_Hex* hex2); /*weaves a shared hex to both its neighbors accross the neighbor boarder*/
	bool weaveSharedHexNeighbors(/*const*/ s_Hex* commonHexNode, s_Hex* hex1, s_Hex* hex2);/*weaves the two hexes together if they both share the same neighbor node*/
	long findEndOfLine(const s_HexPlate* plate, const long start_hex_i, const int web_i); /* go along direction defined by web_i until last index on plate */

	bool computeVecHexDistances(s_rtHexPlate* plate, int center_index = 0);
	s_2pt Loc(const s_rtHex* hex, const s_2pt hexU[], const float r);/*assumes vec distances have been filled*/

	bool rootTwisted(s_rtHexPlate* topPlate, /*const*/ s_rtHexPlate* basePlate, float radang, float sigma);/*gaussian root*/
}

#endif
