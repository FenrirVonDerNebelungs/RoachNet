#include "TrainEye.h"

TrainEye::TrainEye() : 
	m_tgaImg(NULL), 
	m_parseTxt_Key(NULL), 
	m_parseTxt_Sig(NULL), 
	m_parseTxt_Bak(NULL), 
	m_Imgs(NULL), 
	m_N_imgs(0), 
	m_eyeBaseImg(NULL), 
	m_hexImg(NULL), 
	m_Eye(NULL), 
	m_imgHexedPlate(NULL), 
	m_seye(NULL),
	m_imgDim(0), 
	m_numLunaXs(0),
	m_sigToBackgroundRatio(0.f),
	m_slideSigma(0.f),
	m_numSlideSmears(0),
	m_numRotSmears(0),
	m_numTotSmears(0),
	m_sigSmearDR(0.f), 
	m_sigSmearDAng(0.f),
	m_numTotSigSmears(0),
	m_numSigOut(0),
	m_numBakOut(0)
{
	n_ConvolHex::clear(m_eyeBaseConvolHexMaskVars);
	n_gaussianInt::clear(m_gaussDxyBak);
	n_gaussianInt::clear(m_gaussDxySig);
	n_gaussianInt::clear(m_gaussDAngSig);
}

TrainEye::~TrainEye() {
	;
}

unsigned char TrainEye::init(
	int N_imgs,
	int imgDim,
	int numLunaXs,
	float sigToBackgroundRatio,
	float slideSigma /*in R hex dist*/,
	int numRotSmears,
	int numTotSmears,
	float sigSmearDR,
	float sigSmearDAng,
	float RHex,
	float twisted_root_radius,
	float sigma_hexImg,
	int numCurvePatterns
) {
	m_N_imgs = N_imgs;
	m_imgDim = imgDim;
	m_numLunaXs = numLunaXs;
	m_sigToBackgroundRatio = sigToBackgroundRatio;
	m_tgaImg = new CTargaImage;
	m_tgaImg->Init();
	m_parseTxt_Key = new ParseTxt;
	m_parseTxt_Key->init();
	m_parseTxt_Sig = new ParseTxt;
	m_parseTxt_Sig->init();
	m_parseTxt_Bak = new ParseTxt;
	m_parseTxt_Bak->init();
	m_N_imgs = numCurvePatterns;
	m_Imgs = new Img * [m_N_imgs];
	for (int i = 0; i < m_N_imgs; i++) {
		m_Imgs[i] = new Img;
	}

	m_eyeBaseImg = new Img;
	m_hexImg = new HexImg;
	m_Eye = new Eye;

	m_eyeBaseImg->init(imgDim, imgDim, g_colorMode);
	m_hexImg->init(m_eyeBaseImg, RHex);
	m_eyeBaseConvolHexMaskVars = m_hexImg->getMask();
	m_Eye->init(m_hexImg, RHex, twisted_root_radius, sigma_hexImg, 1, 1, g_numStackLevels);

	m_imgHexedPlate = new s_rtHexPlate;
	m_hexImg->spawn(m_imgHexedPlate);
	m_seye = new s_Eye;
	m_Eye->spawn(m_seye);
	/*root the eye in the center of the img plate*/
	n_HexBase::root(m_seye->rootPlate, m_imgHexedPlate, m_Eye->getRootCenterIndex(), m_hexImg->getCenterIndex());

	m_imgDim = imgDim;
	m_numLunaXs = m_Eye->getNLinkedBaseOs();
	m_sigToBackgroundRatio = sigToBackgroundRatio;
	m_slideSigma = slideSigma * RHex;
	m_numSlideSmears = m_numSlideSmears;
	m_numRotSmears = m_numRotSmears;
	m_numTotSmears = m_numSlideSmears * m_numRotSmears;
	int imgHalfDim = imgDim / 2;
	n_gaussianInt::init(m_gaussDxyBak, m_slideSigma, (float)(imgHalfDim-1.f), g_numGaussN);
	m_sigSmearDR = sigSmearDR * RHex;
	m_sigSmearDAng = sigSmearDAng;
	float aprox_numSigSmears = sigToBackgroundRatio * (float)m_numTotSmears;
	m_numTotSigSmears = (int)ceilf(aprox_numSigSmears);
	n_gaussianInt::init(m_gaussDxySig, m_sigSmearDR, (float)(imgHalfDim - 1.f), g_numGaussN);
	n_gaussianInt::init(m_gaussDAngSig, m_sigSmearDAng, 0.f, g_numGaussN);
	m_numSigOut = (m_numTotSigSmears * m_N_imgs) * (m_N_imgs - 1);
	m_numBakOut = (m_numTotSmears * m_N_imgs) * m_N_imgs;
	return genKeyOutFile();
}
void TrainEye::release() {
	if (m_Eye != NULL) {
		m_Eye->despawn(m_seye);
	}
	if (m_seye != NULL) {
		delete m_seye;
		m_seye = NULL;
	}
	if (m_hexImg != NULL) {
		m_hexImg->despawn(m_imgHexedPlate);
	}
	if (m_imgHexedPlate != NULL) {
		delete m_imgHexedPlate;
		m_imgHexedPlate = NULL;
	}
	if (m_Eye != NULL) {
		m_Eye->release();
		delete m_Eye;
		m_Eye = NULL;
	}
	if (m_hexImg != NULL) {
		m_hexImg->release();
		delete m_hexImg;
		m_hexImg = NULL;
	}
	if (m_eyeBaseImg != NULL) {
		m_eyeBaseImg->release();
		delete m_eyeBaseImg;
		m_eyeBaseImg = NULL;
	}
	if (m_Imgs != NULL) {
		for (int i = 0; i < m_N_imgs; i++) {
			if (m_Imgs[i] != NULL) {
				m_Imgs[i]->release();
				delete m_Imgs[i];
				m_Imgs[i] = NULL;
			}
		}
		delete[] m_Imgs;
		m_Imgs = NULL;
	}
	m_N_imgs = 0;
	if (m_tgaImg != NULL) {
		m_tgaImg->Release();
		delete m_tgaImg;
		m_tgaImg = NULL;
	}
	if (m_parseTxt_Key != NULL) {
		m_parseTxt_Key->release();
		delete m_parseTxt_Key;
		m_parseTxt_Key = NULL;
	}
	if (m_parseTxt_Sig != NULL) {
		m_parseTxt_Sig->release();
		delete m_parseTxt_Sig;
		m_parseTxt_Sig = NULL;
	}
	if (m_parseTxt_Bak != NULL) {
		m_parseTxt_Bak->release();
		delete m_parseTxt_Bak;
		m_parseTxt_Bak = NULL;
	}
}

unsigned char TrainEye::run() {
	if (Err(readInSourceImgs())) {
		return ECODE_ABORT;
	}
	for (int i = 0; i < m_N_imgs; i++) {
		if (Err(runStamp(i)))
			return ECODE_FAIL;
	}
	return ECODE_OK;
}
unsigned char TrainEye::genKeyOutFile() {
	int numBaseOs_singleNode = m_Eye->getNSingleNodeBaseOs();
	std::string filePath = g_baseDir + "/" + g_keyOutFile + g_keySuffix;
	m_parseTxt_Key->setOutFile(filePath);
	s_datLine** keyOut = new s_datLine*[2];
	keyOut[0] = new s_datLine(2);
	keyOut[1] = new s_datLine(2);
	keyOut[0]->n = 2;
	keyOut[0]->v[0] = (float)numBaseOs_singleNode;
	keyOut[0]->v[1] = (float)m_numLunaXs;
	keyOut[1]->n = 2;
	keyOut[1]->v[0] = (float)m_numSigOut;
	keyOut[1]->v[1] = (float)m_numBakOut;
	unsigned char err = m_parseTxt_Key->writeCSV(keyOut, 2);
	delete keyOut[1];
	delete keyOut[0];
	delete[] keyOut;
	m_parseTxt_Key->release();
	return err;
}
unsigned char TrainEye::readInSourceImgs() {
	for (int i = 0; i < m_N_imgs; i++) {
		std::string filePath = constructFilePath(i);
		unsigned char isOpen = m_tgaImg->Open(filePath.c_str());
		if (Err(isOpen)) {
			std::cout << "\n could not open " << filePath << " \n";
			return ECODE_ABORT;
		}
		unsigned char* imgData = m_tgaImg->GetImage();
		long img_width = (long)m_tgaImg->GetWidth();
		long img_height = (long) m_tgaImg->GetHeight();
		long color_mode = (long)m_tgaImg->GetColorMode();
		if(Err(m_Imgs[i]->init(imgData, img_width, img_height, color_mode)))
			return ECODE_FAIL;
	}
	return ECODE_OK;
}
unsigned char TrainEye::runStamp(int stamp_num) {
	int totalBakLines = (m_N_imgs - 1) * m_numTotSmears;
	int totalSigLines = (m_N_imgs - 1) * m_numTotSigSmears;
	s_datLine** p_sigOut = new s_datLine * [totalSigLines];
	s_datLine** p_bakOut = new s_datLine * [totalBakLines];
	for (int i = 0; i < totalSigLines; i++) {
		p_sigOut[i] = new s_datLine(m_numLunaXs);
	}
	for (int i = 0; i < totalBakLines; i++) {
		p_bakOut[i] = new s_datLine(m_numLunaXs);
	}
	if (Err(getXsForStamp(stamp_num, p_sigOut, p_bakOut)))
		return ECODE_FAIL;
	std::string fnumber;
	n_ParseTxt::intToFixedLenStr(stamp_num, g_fnameNumberLen, fnumber);
	std::string foutPathSig = g_baseDir + "/" + g_sigOutFile + fnumber + g_outSuffix;
	std::string foutPathBak = g_baseDir + "/" + g_bakOutFile + fnumber + g_outSuffix;
	m_parseTxt_Sig->setOutFile(foutPathSig);
	m_parseTxt_Bak->setOutFile(foutPathBak);
	if (Err(m_parseTxt_Sig->writeCSV(p_sigOut, totalSigLines)))
		return ECODE_FAIL;
	if (Err(m_parseTxt_Bak->writeCSV(p_bakOut, totalBakLines)))
		return ECODE_FAIL;
	m_parseTxt_Sig->release();
	m_parseTxt_Bak->release();
	for (int i = 0; i < totalSigLines; i++) {
		if (p_sigOut[i] != NULL)
			delete p_sigOut[i];
	}
	for (int i = 0; i < totalBakLines; i++) {
		if (p_bakOut[i] != NULL)
			delete p_bakOut[i];
	}
	delete[] p_bakOut;
	delete[] p_sigOut;

	return ECODE_OK;
}
unsigned char TrainEye::getXsForStamp(int stamp_num, s_datLine* sigOut[], s_datLine* bakOut[]) {
	Img* bakImgs = new Img[m_numTotSmears];
	Img* sigImgs = new Img[m_numTotSigSmears];
	int numSigOut = 0;
	int numBakOut = 0;
	for (int stamp_i = 0; stamp_i < m_N_imgs; stamp_i++) {
		if (stamp_i != stamp_num) {/* (numTotSigSmears * N_imgs) * (N_imgs-1) */
			if (Err(genSigSeq(m_Imgs[stamp_num], m_numTotSigSmears, sigImgs)))
				return ECODE_FAIL;
			for (int sig_i = 0; sig_i < m_numTotSigSmears; sig_i++) {
				genLunaOut(sigImgs[sig_i], sigOut[numSigOut]);
			}
		}
		/* (numTotSmears * N_imgs) * (N_imgs)  */
		if (Err(genBakSeq(m_Imgs[stamp_i], m_numTotSmears, bakImgs)))
			return ECODE_FAIL;
		for (int bak_i = 0; bak_i < m_numTotSmears; bak_i++) {
			genLunaOut(bakImgs[bak_i], bakOut[numBakOut]);
		}
		
	}
	return ECODE_OK;
}
unsigned char TrainEye::genSigSeq(const Img* sigImg, const int N, Img imgs[]) {
	if (N < 1)
		return ECODE_FAIL;
	s_2pt* offsets = new s_2pt[N];
	s_2pt center = { 0.f, 0.f };
	float* dAngs = new float[N];
	for (int i = 0; i < N; i++) {
		offsets[i] = Math::randGaus2D(center, m_gaussDxySig);
		dAngs[i] = Math::randGausJitterAng(m_gaussDAngSig, 0.f);
		imgs[i].init(*sigImg);
	}
	for (int i = 0; i < N; i++) {
		imgs[i].rotate(dAngs[i]);
		imgs[i].translate(offsets[i]);
	}
	delete[] dAngs;
	delete[] offsets;
	return ECODE_OK;
}
unsigned char TrainEye::genBakSeq(const Img* bakImg, const int N, Img imgs[]) {
	if (N < 1)
		return ECODE_FAIL;
	s_2pt* offsets = new s_2pt[N];
	s_2pt center = { 0.f, 0.f };
	float* dAngs = new float[N];
	for (int i = 0; i < N; i++) {
		offsets[i] = Math::randGaus2D(center, m_gaussDxyBak);
		dAngs[i] = Math::randAng();
		imgs[i].init(*bakImg);
	}
	for (int i = 0; i < N; i++) {
		imgs[i].rotate(dAngs[i]);
		imgs[i].translate(offsets[i]);
	}
	delete[] dAngs;
	delete[] offsets;
	return ECODE_OK;
}
unsigned char TrainEye::genLunaOut(const Img& img, s_datLine* Xs) {
	n_HexImg::update(m_imgHexedPlate, &img, m_eyeBaseConvolHexMaskVars);
	n_Eye::run(m_seye);
	s_EyeCore* first_eye_core = m_seye->eyeCores[0];
	s_EyeNets first_nets_inst = first_eye_core->nets[0];
	s_NNet* first_nnet_inst = first_nets_inst.nets[0];
	s_Node_w* os = new s_Node_w[m_numLunaXs];
	int len_os = n_NNet::getLinkedBaseOs(first_nnet_inst, os);
	if (len_os != m_numLunaXs)
		return ECODE_FAIL;
	for (int i = 0; i < len_os; i++) {
		Xs->v[i] = os[i].w;
	}
	Xs->n = len_os;
	return ECODE_OK;
}
std::string TrainEye::constructFilePath(int fnum) {
	std::string filePath = g_baseDir + "/" + g_imgFile;
	std::string intStr;
	n_ParseTxt::intToFixedLenStr(fnum, g_fnameNumberLen, intStr);
	filePath += intStr;
	filePath += g_imgFileSuffix;
	return filePath;
}