#include "TrainL0.h"

TrainL0::TrainL0() :m_stampDir(TRAINL0STAMP_BASEDIR), m_trainStamp(NULL), m_baseImg(NULL), m_r(0.f), m_imgDim(0.f), m_imgWH(0L), m_num_sig_stamps(0.f), m_color_mode(0L) {
	;
}
TrainL0::~TrainL0() {
	;
}
unsigned char TrainL0::init() {
	m_stampDir += '/';
	m_stampDir += TRAINL0STAMP_L0DIR;
	m_stampDir += '/';
	std::string keyDir = m_stampDir;
	keyDir += TRAINL0STAMP_MASTERKEYFILE;
	keyDir += TRAINL0STAMP_KEYFILESUF;
	ParseTxt readMaster;
	readMaster.init();
	readMaster.setInFile(keyDir);

	s_datLine fline_in[1];
	if (readMaster.readCSV(fline_in, 1)<1) {
		std::cout << "failed to read master key file: " << keyDir.c_str() << "   file contains r and img dim info and num stamps\n";
		return ECODE_FAIL;
	}
	m_r = fline_in[0].v[0];
	m_imgDim = fline_in[0].v[1];
	m_imgWH = static_cast<long>(fline_in[0].v[2]);
	m_num_sig_stamps = fline_in[0].v[3];
	if (m_num_sig_stamps > TRAINL0STAMP_max_train_stamps)
		m_num_sig_stamps = TRAINL0STAMP_max_train_stamps;
	m_color_mode = static_cast<long>(fline_in[0].v[4]);

	if (m_num_sig_stamps < 1) {
		std::cout << "Error: " << m_num_sig_stamps << " is not a valid number of stamps must be at least 1\n";
		return ECODE_ABORT;
	}
	if (m_color_mode != 3L && m_color_mode != 4L) {
		std::cout << "Error: " << m_color_mode << "  is not a valid color mode for image must be either 3 or 4\n";
		return ECODE_ABORT;
	}
	/*create a dummy image used to initialize*/
	m_baseImg = new Img;
	if (Err(m_baseImg->init(m_imgWH, m_imgWH, m_color_mode))) {
		std::cout << "Error could not initialize image with dim: " << m_imgWH<<"\n";
		return ECODE_ABORT;
	}
	m_trainStamp = new TrainL0Stamp;
	if (Err(m_trainStamp->init(m_baseImg, m_r))) {
		std::cout << "Error initializing stamp generator\n";
		if (m_baseImg != NULL)
			m_baseImg->release();
		return ECODE_ABORT;
	}
	return ECODE_OK;
}
void TrainL0::release() {
	if (m_trainStamp != NULL) {
		m_trainStamp->release();
		delete m_trainStamp;
	}
	m_trainStamp = NULL;
	if (m_baseImg != NULL) {
		m_baseImg->release();
		delete m_baseImg;
	}
	m_baseImg = NULL;
}
unsigned char TrainL0::run() {
	std::cout << "Running background\n";
	if (Err(runBakDir())) {
		release();
		return ECODE_FAIL;
	}
	std::cout << "Running Signal for "<<m_num_sig_stamps<<" sig stamps\n";
	for (int i = 0; i < m_num_sig_stamps; i++) {
		std::cout << "  stamp: " << i << "\n";
		if (Err(runStampDir(i))) {
			release();
			return ECODE_FAIL;
		}
	}
	return ECODE_OK;
}
unsigned char TrainL0::runBakDir() {
	std::string stampDir(m_stampDir);
	stampDir += TRAINL0STAMP_BAKDIR;
	stampDir += '/';
	return m_trainStamp->runDir(stampDir);
}
unsigned char TrainL0::runStampDir(int stampNum) {
	std::string stampDir(m_stampDir);
	stampDir += TRAINL0STAMP_SIGDIR;
	stampDir += '/';
	stampDir += TRAINL0STAMP_STAMPDIR;
	std::string stamp_tail = "";
	if (Err(n_ParseTxt::intToFixedLenStr(stampNum, TRAINL0STAMP_INTSTRLEN, stamp_tail))) {
		return ECODE_ABORT;
	}
	stampDir += stamp_tail;
	stampDir += '/';
	return m_trainStamp->runDir(stampDir);
}