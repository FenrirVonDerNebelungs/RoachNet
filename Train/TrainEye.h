#pragma once
#ifndef TRAINEYE_H
#define TRAINEYE_H

#ifndef EYE_H
#include "../Eye/Eye.h"
#endif
#ifndef HEXIMG_H
#include "../HexedImg/HexImg.h"
#endif

#ifndef CTARGAIMAGE_H
#include "../FileIO/CTargaImage.h"
#endif
#ifndef PARSETXT_H
#include "../FileIO/ParseTxt.h"
#endif

const int g_numStackLevels = 3;
const int g_numGaussN = 100;
const int g_fnameNumberLen = 5;
const int g_numStampsIn = 10;/*needs to be reset*/

const long g_colorMode = 3L;

const std::string g_baseDir = "../Dat";
const std::string g_keyFile = "keys";
const std::string g_masterFile = "masterKey";
const std::string g_keySuffix = ".txt";
const std::string g_imgFile = "imgf";
const std::string g_imgFileSuffix = ".tga";

const std::string g_sigOutFile = "sigout";
const std::string g_bakOutFile = "bakout";
const std::string g_outSuffix = ".txt";

class TrainEye : public Base {
public:
	unsigned char init(
		int N_imgs,
		int imgDim,
		int numLunaXs,
		float sigToBackgroundRatio,
		float slideSigma /*in R hex dist*/,
		int numRotSmears,
		int numTotSmears,
		float sigSmearDR,
		float sigSmearAng,
		float RHex,
		float twisted_root_radius,
		float sigma_hexImg,
		int numCurvePatterns /*same as N_imgs or number of stamps to train on */
		);
	void release();
	unsigned char run();
protected:
	CTargaImage* m_tgaImg;
	ParseTxt* m_parseTxt_Sig;
	ParseTxt* m_parseTxt_Bak;
	Img** m_Imgs;
	int m_N_imgs;

	Img* m_eyeBaseImg;
	HexImg* m_hexImg;
	Eye* m_Eye;

	s_rtHexPlate* m_eyeBaseImgHexedPlate;
	s_Eye* m_seye;

	int m_imgDim;
	int m_numLunaXs;
	float m_sigToBackgroundRatio;
	/*background extra gen*/
	float m_slideSigma; 
	int m_numSlideSmears;
	int m_numRotSmears;
	int m_numTotSmears;/*Slide smears * rot smears */
	s_gaussianInt m_gaussDxyBak;
	/*signal extra gen*/
	float m_sigSmearDR;
	float m_sigSmearDAng;
	int m_numTotSigSmears;
	s_gaussianInt m_gaussDxySig;
	s_gaussianInt m_gaussDAngSig;



	unsigned char readInSourceImgs();
	unsigned char runStamp(int stamp_num);

	unsigned char getXsForStamp(int stamp_num, s_datLine* sigOut[], s_datLine* bakOut[]);

	unsigned char genSigSeq(const Img* sigImg, const int N, Img imgs[]);
	unsigned char genBakSeq(const Img* bakImg, const int N, Img imgs[]);
	
	unsigned char genLunaOut(const Img& img, s_datLine* Xs);

	std::string constructFilePath(int fnum);
};
#endif