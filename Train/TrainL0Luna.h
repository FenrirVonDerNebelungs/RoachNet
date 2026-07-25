#pragma once
#ifndef TRAINL0LUNA_H
#define TRAINL0LUNA_H

#ifndef COLOR_H
#include "../HexedImg/Color.h"
#endif
#ifndef L0LAYER_H
#include "../NNet/L0Layer.h"
#endif

class TrainL0Luna : public base {
public:
	unsigned char init(
		long img_width, 
		long img_height,
		float r_scale /*the small hex dim*/
	);
	void release();
	unsigned char update(Img* baseImg);
	unsigned char run();
protected:
	/*not owned*/
	Img* m_Img;
	/*owned*/
	Img* m_dimImg;/*image with correct dim used to initialize various objects*/
	HexEye* m_genHexEye;/*generates the larger hex Eye */
	HexEyeImg* m_genEyeImg;
	Col* m_genColPlates;
	LunaLayer* m_genLuna;
	L0Layer* m_genL0;

	s_HexEye* m_eye;/*eye struct that will be rooted on image and then updated for each new image, this eye will be one layer larger than that of the L0 net
	                  which is being trained by this object*/
	s_ColPlateLayer* m_colPlates;/* color plates inherits from HexBasePlateLayer, should contain the one color plate that picks out the white*/
	s_HexPlateLayer* m_lunaPlates;/*plates each of which correspond to a result from a luna operating on the base layer*/
	s_HexPlateLayer* m_L0Plates;

	/*this section would normally be run per each color plate*/
	

	/*helpers to init*/
	unsigned char genWhiteColWheel(s_ColWheel* col);/*col exist but is not initalized*/
	unsigned char spawnDataObjs();
};


#endif