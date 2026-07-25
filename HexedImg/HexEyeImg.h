#pragma once
#ifndef HEXEYEIMG_H
#define HEXEYEIMG_H

#ifndef HEXEYE_H
#include "HexEye.h"
#endif

#ifndef CONVOLHEX_H
#include "ConvolHex.h"
#endif

class HexEyeImg : public Base{
public:
	HexEyeImg();
	~HexEyeImg();

	unsigned char init(Img* baseImg, s_2pt& eye_center, HexEye* genHexEye, 
		float sigmaVsR = CONVOLHEX_sigmaVsR,
		float IMaskRVsR = CONVOLHEX_IMaskRVsR
	);/* pre step to filling the lowest hex plate of the eye with the convoluted colors from the image
																				image should have the correct dimensions, the same as those of the image used later
																				eye_center should be the center of the eye in
																				the image coordinates
																				heye should be created and initialized 
																				function fixes up heye so that the locations
																				of its nodes correspond to their proper locations
																				as centered on the eye_center coordinate in the image 
																				if the node is off the image the ij node coords are set to negative
																				uses the xy coord set in each node when the eye is generated to calculate
																				the original offset of the node from the center*/
	void release();

	unsigned char root(Img* baseImg, s_HexEye& heye);/*this transfers the coord calculated and put in the refEye at init into the s_HexEye
													   and checks that the image has the correct dimensions to match what was initialized
													   heye must be spawned by genHexEye*/
	unsigned char run(Img* baseImg, s_HexEye& hexe); /*this requires the heye to already be rooted
													   this is the unthreaded version
													   heye must already have root run on it*/

protected:
	/*not owned*/
	HexEye* m_genHexEye;
	/*owned*/
	float   m_imgWidth;
	float   m_imgHeight;
	ConvolHex* m_Convol;

	s_HexEye* m_refHexEye;

	unsigned char resetNodeToImgCoord(Img* baseImg, s_2pt& eye_center, s_Hex* hex_nd);/*center is in image coordinates*/
	unsigned char runSingleThread(Img* baseImg, s_HexEye& heye);
};

namespace n_HexEyeImg{
	/*helpers*/
	/*i, j less than 0 is checked in convCellKernel*/
	inline void fillStruct(Img& inimg, ConvolHex& genConvol, s_HexPlate& s_botPlate, s_convKernVars& IOVars);
}
#endif