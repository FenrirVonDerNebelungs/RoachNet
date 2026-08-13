#pragma once
#ifndef LUNA_H
#define LUNA_H

#ifndef EXHEX_H
#include "../HexBase/ExHex.h"
#endif
const int LUNA_NUM_LUNAS = 6;/* was 8; *//*6 rotated lunas 7th (index 6) is white, 8th (index 7) is black*/
const int LUNA_NUM_FOOTS = 7;/* number of hexes in the lower level of the luna */
const float LUNA_WSCALE = 3.f;//60.f for line finder//1.f

class Luna : public Base {
public:
	Luna();
	~Luna();

	unsigned char init(ExHex* exHex);
	void release();

	unsigned char spawn(/*const*/s_HexPlate* basePlate, s_HexPlateLayer* lunPlates);
	void          despawn(s_HexPlateLayer* lunPlates);

	long getNumNodes();/*get the number of nodes in a luna plate*/
	inline int getNumLunaPlates() { return LUNA_NUM_LUNAS; }

private:
	/*not owned*/ 
	ExHex* m_exHex;/*was used to generate the extended plates, assume has already been initialized*/

	s_Node m_lunas[LUNA_NUM_LUNAS];/*these are the 6 rotated lunas, the 7th is white and the 8th is black*/

	unsigned char spawnPlate( /*const*/s_HexPlate* basePlate, int lunRot, s_HexPlate* lunPlate);
	void          despawnPlate(s_HexPlate* lunPlate);


	unsigned char genLunaPatterns();
	void genHalfLunaPattern(int lunRot, s_Node& topNd);
};


#endif
