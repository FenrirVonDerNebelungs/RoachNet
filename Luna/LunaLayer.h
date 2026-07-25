#pragma once
#ifndef LUNALAYER_H
#define LUNALAYER_H

#ifndef LUNA_H
#include "Luna.h"
#endif


class LunaLayer : public Base{
public:
	LunaLayer();
	~LunaLayer();

	unsigned char init(float r);
	void          release();
	unsigned char spawn(s_HexPlate* colPlate, s_HexPlateLayer* lunaPlates);/*colPlate must have the correct dim for the luna plates and the lunaPlates 
																		     will be linked to the colPlate instance
																			the lunaPlates should be created but not initialized */
	void despawn(s_HexPlateLayer* lunaPlates);
	unsigned char run(s_HexPlate* colPlate, s_HexPlateLayer* lunaPlates);/*right now unthreaded */
	inline s_Luna* getLunaNets() { return m_lunaNets; }
	inline Luna* getGenLuna() { return m_genLuna; }

	inline float get_r() { return m_genLuna->get_r(); }
	inline int getNumLuna() { return m_genLuna->getNumLuna(); }
private:
	/*owned*/
	Luna* m_genLuna;
	s_Luna* m_lunaNets;
	
};

namespace n_LunaLayer {
	bool run(LunaLayer* genLuna, s_HexPlate* colPlate, s_HexPlateLayer* lunaPlates, long plate_i);/**/
}

#endif
