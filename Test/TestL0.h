#pragma once
#ifndef TESTL0_H
#define TESTL0_H

#ifndef RENDERBASE_H
#include "../Render/RenderBase.h"
#endif
#ifndef COLOR_H
#include "../HexedImg/Color.h"
#endif
#ifndef LUNALAYER_H
#include "../Luna/LunaLayer.h"
#endif
#ifndef L0LAYER_H
#include "../NNet/L0Layer.h"
#endif
#ifndef TRAINL0_H
#include "../Train/TrainL0.h"
#endif

/*these directories should be created under the L0 directory prior to the program being run*/
#define TESTL0_TESTDIR "Test"
#define TESTL0_INDIR   "InDir"/*directory containing the images that this program will run over*/
#define TESTL0_IMGINFILE "Img"/*prefix for image in file*/
#define TESTL0_IMGFILESUF ".tga"/*sufix for image in file*/
#define TESTL0_OUTDIR "OutDir"
#define TESTL0_OUTSPACERL0 "s" /* out image files have the format "in image name" spacer "stamp number" IMGFILESUF*/
#define TESTL0_OUTSPACERLUNA "l"
#define TESTL0_OUTCOL "col"

#define TESTL0_MAXNUMINPUTIMGFILES 100

const char TESTL0_render_null_col = 0x00;
class TestL0 : public Base
{
public:
	TestL0();
	~TestL0();

	unsigned char init();
	void release();

	unsigned char run();/*netImg shuld be new but not init*/

private:
	float m_r;
	int m_len_wbs;
	int m_num_stamps;
	int m_num_rendered_plates;/*essentially a constant, the number of plates rendered L0 stamp set: 1 col plate + num_luna luna plates + num_stamps stamp plate*/

	std::string m_baseDir;
	/*data input objects*/
	std::string* m_inFileNames[TESTL0_MAXNUMINPUTIMGFILES];/*multiple images that can be run over; a set of L0 stamp images will be generated for each*/
	Img* m_baseImgs[TESTL0_MAXNUMINPUTIMGFILES];/*loaded input images*/
	int  m_num_baseImgs;/*number of input images*/
	s_2pt_i m_eye_center;
	/**** catched objects to make errhandle deletable  & other catched objects***/
	float* m_ids_stack;/*ids read out of the key file used later to id stamps in 2nd part of out image names*/
	float* m_wbs_stack;/* the trained w weights and b biases loaded from the pre-externally trained wbs key file*/
	/*** imgs are rewritten for each base img run ***/
	Img** m_renderedStampImgs;/* 1 img is hexed base after col, next imgs are luna plates, final set are L0 stamps, rewritten for each base img */
	/*owned*/
	CTargaImage* m_tga;
	RenderBase* m_render;

	/**generators**/
	/*** generators of data objects ***/
	HexEye* m_genHexEye;
	HexEyeImg* m_genHexedImg;
	Col* m_genColLayer;
	LunaLayer* m_genLunaLayer;
	L0Layer* m_genL0Layer;

	/** data objs **/

	s_HexEye* m_seye;
	s_ColPlateLayer* m_colPlates;
	s_HexPlateLayer* m_lunaPlates;
	s_HexPlateLayer* m_L0Plates;

	s_ColWheel m_sCol;


	/*init helper functions*/
	unsigned char genWhiteColWheel();

	unsigned char readInImgs();
	void releaseInImgs();
	unsigned char readInWBs(float ids_arr[], float wbs_arr[]);
	unsigned char initStampRenderImgs();/*uses m_num_stamps and dimensions of the first m_baseImgs*/
	void releaseStampRenderImgs();

	/*run*/
	unsigned char runAndWriteRenderForImg(int img_index);/*index in m_baseImgs and m_inFileNames arrays*/
	/**** helpers to runAndWriteRenderForImg ***/
	unsigned char runOnImg(Img* netImg);/*runs on base img*/
	unsigned char renderImgs();/* requies runOnImg to have been run render images to m_renderedStampImgs */
	/***** helpers to renderImg *****/
	unsigned char renderHexPlate(s_HexPlate* plt, Img* renderedimg);
	/********* helpers to renderHexPlate **********/
	unsigned char colHexPlate(s_HexPlate* plt);/*puts o into hex plate rgb */

	unsigned char writeRenderImg(const std::string& render_img_dir_base, const std::string& imglabel, Img* renderedImg);

	bool ErrHandle(unsigned char errc, const char* errmsg = "Error");
};

#endif