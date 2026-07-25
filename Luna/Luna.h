#pragma once
#ifndef LUNA_H
#define LUNA_H

#ifndef SNET_H
#include "../Nets/sNet.h"
#endif


#define LUNA_WSCALE 3.f//60.f for line finder//1.f

const int LUNA_NUM_LUNAS = 6;/* was 8; *//*6 rotated lunas 7th (index 6) is white, 8th (index 7) is black*/
const int LUNA_NUM_FOOTS = 7;/* number of hexes in the lower level of the luna */

class s_Luna : public s_CNets {
public:
	s_Luna():m_lunaEye(NULL) {	; }
	~s_Luna() { ; }

	unsigned char init(const s_Luna& other);/*makes this s_Luna a copy of the other s_Luna this s_Luna owns its own objects*/
	void release();
	inline unsigned char setOwnedEye(s_HexEye* heye) { if (m_lunaEye != NULL) return ECODE_FAIL; m_lunaEye = heye; eye = m_lunaEye;  return ECODE_OK; }/*transfers ownershsip of this object to the luna*/
	inline s_HexEye* getOwnedEye() { return m_lunaEye; }
	inline void releaseOwnedEye(); 
protected:
	s_HexEye* m_lunaEye;/*hex eye that is owned by this class, unowned eye points to this one*/
};
namespace n_Luna {
	bool run(s_Luna* lun, s_HexPlate* colPlate, s_HexPlateLayer* lunPlates, long plate_index);
}
class Luna : public Base {
public:
	Luna();
	~Luna();

	unsigned char init(float r);/*r is the dimension of the base hex for the luna pattern*/
	void release();

	unsigned char spawn(s_Luna* lun);/*initializes the s_Luna and fills it with the luna patterns, the luna owns its own eye*/
	void          despawn(s_Luna* lun);


	inline float get_r() { return m_EyeMaster->getBottomR(); }
	inline int getNumLuna() { return m_NetsMaster->getNumNets(); }
	inline int getNumWBs_perLunaNet(){ return m_NetsMaster->getTotalNumWBs_perNet(); }

private:
	/*owned modules used to initiate their respective structs*/
	CNets* m_NetsMaster;
	HexEye* m_EyeMaster;
	/*                                                       */


	unsigned char genLunaPatterns(s_Luna* lun);
	void genHalfLunaPattern(int lunRot, s_nNode* topNd);
};

#endif
