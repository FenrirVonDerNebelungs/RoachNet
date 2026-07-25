#pragma once
#ifndef COLPLATE_H
#define COLPLATE_H
#ifndef HEXEYEIMG_H
#include "../HexedImg/HexEyeImg.h"
#endif

struct s_ColWheel_Col {
	float rgb[3];/*normalized by pixMax*/
	float shortest_rgb;/*normalized to pixMax*/
	float hue_Intensity;/*the ave of the two hue components after shortest is subtracted*/
	s_2pt hue;/*normalized to pixMax*/
	float huelen;
	float Intensity;/*the ave of all three normalized rgb*/
};

struct s_ColWheel {
	float pixMax;/* maximum value for the pixels usually 255 for 8 bits*/

	/*this block is reset by resetCol*/
	float Dhue; /*distance in cos (needs to be greater than 0) */
	float DI;   /* intensity */
	float DSat; /*  saturation */
	float HueFadeV; /*hue vector length at which the hue is considered to fade to a neutral return */
	float I_target;
	s_2pt Hue_target;/*rgb vector in colorwheel*/
	float Sat_target;/*sat defined as hue len/ intensity len*/

	float finalScaleFactor;

	/*red, green, blue vectors in colwheel space*/
	s_2pt Ur;/*Ur is in the unit x(0) direction*/
	s_2pt Ug;/*Ug is up (pos y (x1)) and backwards -x(0)*/
	s_2pt Ub;/*Ub is down (neg y)     and backwards -x(0)*/

	float DhueRes;/* 2 - m_Dhue also reset by resetCol*/
};

class s_ColPlate : public s_HexPlate {
public:
	s_ColPlate();
	~s_ColPlate();

	s_ColWheel Col;
};

class ColPlate : public Base {
public:
	ColPlate();
	~ColPlate();

	unsigned char init();
	void          release();

	unsigned char spawn(
		s_HexPlate* hexedImg,
		s_ColPlate* colPlate,
		const s_ColWheel& Col
	);
	void          despawn(s_ColPlate* colPlate);
	
	void resetCol(s_ColPlate* colPlate, const s_ColWheel& Col);
protected:
	unsigned char setDownLinks(s_HexPlate* hexedImg, s_ColPlate* colPlate);
	void setColWheelUnitVectors(s_ColWheel& col);
};
namespace n_ColWheel {
	void setColWheelUnitVectors(s_ColWheel& col);
	void clear(s_ColWheel& Col);
	void resetCol(s_ColWheel& col, const s_ColWheel& orig);/*resets only what changes when the col/sat/intensity val change but does not reset U vectors*/
	void copy(s_ColWheel& Col, const s_ColWheel& orig);

}
namespace n_ColPlate {
	bool run(s_HexPlate* hexedImg, s_ColPlate* colPlate, long plate_index);

	float findColDistances(const s_ColWheel& cw, float rgb[]);
	/*helpers to findColDistances*/
	s_ColWheel_Col findColWheelHue(const s_ColWheel& cw, float rgb[]); /* fills shortest_rgb, rgb, intensity and hue_intensity and huelen 
																		values are in 0->1 (255) 
																		hue is the vector from the addition of the two
																		cw unit vectors with the shortest_rgb taken out
																		 intensity is the ave after normalization of rgb: (r+g+b)/3*/
	float findHueDistance(const s_ColWheel& cw, s_2pt& hueV, float hueLen);
	float findIDistance(const s_ColWheel& cw, s_ColWheel_Col& wcol);
	float findSatDistance(const s_ColWheel& cw, s_ColWheel_Col& wcol);

	/*additional color manipulation*/
	bool setRGBFromColWheel(const s_ColWheel& cw, s_ColWheel_Col& wcol, float rgb[]);/*color wheel col, wcol must have hue direction set, 
																						intensity as sum
																						and shortest rgb all normalized to 1*/
	bool getRGBFromColWheel(const s_ColWheel& cw, float rgb[]);/*uses I_target, Hue_target, and Sat_target from the color wheel, the unit vectors must also be set*/
	bool getRGBFromHueVIntensity(const s_ColWheel& cw, const s_2pt& hueV, float Intensity, float rgb[]);/*uses getRGBColoredFromHueV and Intensity
																										to find full rgb components
																										cw is just used for the unit vectors*/
	bool getRGBColoredFromHueV(const s_ColWheel& cw, const s_2pt& hueV, float rgb[]);/*this is hue with length depending on intensity but normalized by pixmax
													it returns the rg, rb, or bg pair that contributed to the hue*/

	bool getColoredHueCompFromHue(const s_2pt& vh, const s_2pt& U1, const s_2pt& U2,  s_2pt& colhue);/*vh is the hue, U1 and U2 are the hue coord
																											 vectors from which this hue is constructed
																											 return the two components of the hue along
																											 the U1 and U2 that add up when multiplied
																											 by the U1 and U2 to create the hue
																											 this kinda dups a section of getRGBColoredFromHueV*/

}
#endif
