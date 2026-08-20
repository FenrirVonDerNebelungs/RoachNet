#pragma once
#ifndef HEXIMG_H
#define HEXIMG_H
#ifndef CONVOLHEX_H
#include "ConvolHex.h"
#endif
#ifndef HEX_H
#include "../HexBase/Hex.h"
#endif
#ifndef HEXRECT_H
#include "../HexBase/HexRect.h"
#endif
/*class takes an image from a pointer that it does NOT OWN and uses it to first create a hex structure which is stored in 
  the hex base plate
  each pass is then used to update the rgb values of the hexes in the HexBasePlate from the current image values */
class HexImg : public rtHexRect {
public:
	HexImg();
	~HexImg();

	unsigned char init(
		Img* img,
		float r = 9.f,//7.f,//3.f,//5.f, //3.f,  7 for debug of eye
		float sigmaVsR = CONVOLHEX_sigmaVsR,
		float IMaskRVsR = CONVOLHEX_IMaskRVsR
	);
	void release();
	unsigned char spawn(s_rtHexPlate* plate);/*points to a plate object that exists but has NOT been intialized */
	void despawn(s_rtHexPlate* plate);

	inline s_ConvolHex getMask() { return m_Convol->getMask(); }
protected:
	/*not owned */
	Img* m_img;
	/* owned */
	ConvolHex* m_Convol;


	s_2pt_i m_corner_start_hex_center;/*starts from top left, min X max Y*/
	s_2pt m_corner_margin;
};

namespace n_HexImg {
	bool update(s_rtHexPlate* plate, const Img* img, const s_ConvolHex& MaskVars);/*image must have same dimensions as original*/
}
#endif