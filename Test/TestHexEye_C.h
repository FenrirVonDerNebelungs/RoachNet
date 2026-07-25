#pragma once
#ifndef TESTHEXEYE_C_H
#define TESTHEXEYE_C_H

#ifndef RENDERBASE_H
#include "../Render/RenderBase.h"
#endif
#ifndef HEXEYE_H
#include "../HexedImg/HexEye.h"
#endif
/*this class tests the code of HexEye.h,
 the code should produce a s_HexEye with the correct shapes, layers and links */
class TestHexEye_C : public Base {
public:
	TestHexEye_C();
	~TestHexEye_C();

	unsigned char init(
		float r = 7.0f,
		int NLevels = 2,
		float intensity=0.9f,/*not exactly intensity more like fraction of max possible intensity*/
		float hue_start_rad = 0.f,
		float hue_Drad = PI / 4.f,
		float shortest_rgb=0.3f,
		long pix_between_lev_imgs=20);
	void release();
	unsigned char runTestMasks(Img** test_img);
	unsigned char runTestHex(Img* test_hex);
	unsigned char run(Img* retImg_levels/*created but uninit image that will contain the returned image*/, Img* retImg_web, Img* retImg_lower);
protected:
	/*owned*/
	HexEye* m_genEye;
	RenderBase* m_render;/*object used to help in rendering*/

	s_HexEye* m_seye;
	s_ColWheel m_colWheel;/*clear in constructer and setUnitVectors in init*/
	float m_intensity;
	float m_hue_start_rad;
	float m_hue_Drad;
	float m_shortest_rgb;
	long m_pix_between_lev_imgs;
	/*draws a level with different colors from seye*/
	unsigned char drawEyeLevel(s_HexEye* seye, int eye_lev, s_2pt& lev_center, Img* lev_img);
	/*draws the levels next to eachother*/
	unsigned char drawEyeLevels(s_HexEye* seye, Img* lev_img);/*lev_img is created but not initalized when passed, this function initalizes it*/

	/*draw the links between hexes on the same eye level*/
	unsigned char drawWebLinkForLevel(s_HexPlate* lev, s_2pt& lev_center, int web_link_i, Img* lev_img);
	/*draw all the links by redrawing the level, several pictures*/
	unsigned char drawAllWebLinksForLevel(s_HexPlate* lev, Img* lev_img);

	/*draw lower links, level hex in black outline overlapping the level below it with the below hexes highlighted*/
	unsigned char drawLevelAndLowerLinks(s_HexEye* sye, int eye_lev, Img* lev_img);/*lev_img is created but not initialized whn passed, this function initalizes it*/
	unsigned char drawUpperHexAndLowerLinksWithBackground(s_HexEye* seye, int eye_lev, long lev_hex_i, s_2pt& lev_center, Img* lev_img);
	unsigned char drawLevelHexAndLowerLinks(s_HexPlate* lev, long lev_hex_i, s_2pt& lev_center, Img* lev_img);

	/*helpers*/
	unsigned char fillHexLevelWithColorRot(s_HexPlate* lev);
	unsigned char setIJForLevelToRelative(s_HexPlate* lev);
	unsigned char drawColorRotLevel(s_HexPlate* lev, s_2pt& lev_center, Img* lev_img);
	unsigned char drawWebLinksOnLevel(s_HexPlate* lev, s_2pt& lev_center, int web_link_i, Img* lev_img);
};

#endif