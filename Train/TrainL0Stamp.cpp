#include "TrainL0Stamp.h"
TrainL0Stamp::TrainL0Stamp() : m_baseImg(NULL), m_tga(NULL), m_net_lines(NULL), m_net_os_scratch(NULL),
m_genHexEye(NULL), m_genHexedImg(NULL), m_genColLayer(NULL), m_genLunaLayer(NULL), m_genL0Net(NULL),
m_seye(NULL), m_colPlates(NULL), m_lunaPlates(NULL), m_L0Nets(NULL){
	utilStruct::zero2pt_i(m_eye_center);
	n_ColWheel::clear(m_sCol);
}
TrainL0Stamp::~TrainL0Stamp() {
	;
}
unsigned char TrainL0Stamp::init(Img* baseImg, float r) {
	if (baseImg == NULL)
		return ECODE_FAIL;

	m_tga = new CTargaImage;
	m_tga->Init();

	m_baseImg = baseImg;
	genWhiteColWheel();
	m_genHexEye = new HexEye;
	m_genHexedImg = new HexEyeImg;
	m_genColLayer = new Col;
	m_genLunaLayer = new LunaLayer;
	m_genL0Net = new L0Net;
	

	m_eye_center = m_baseImg->getCenter();
	s_2pt eye_center = { Math::ltof(m_eye_center.x0), Math::ltof(m_eye_center.x1) };
	m_genHexEye->init(r, TRAINL0STAMP_num_img_eye_lev);
	m_genHexedImg->init(m_baseImg, eye_center, m_genHexEye);
	m_genColLayer->init(1);
	m_genColLayer->addCol(&m_sCol);
	m_genLunaLayer->init(r);
	int num_luna_plates = m_genLunaLayer->getNumLuna();
	int num_L0_nets = 1;
	m_genL0Net->init(r, num_luna_plates, num_L0_nets);

	/*data objects*/
	m_seye = new s_HexEye;
	m_colPlates = new s_ColPlateLayer;
	m_lunaPlates = new s_HexPlateLayer;
	m_L0Nets = new s_CNets;

	m_genHexEye->spawn(m_seye);
	m_genHexedImg->root(m_baseImg, *m_seye);
	m_genColLayer->spawn(m_seye->getBottom(), m_colPlates);
	m_genLunaLayer->spawn(m_colPlates->get(0), m_lunaPlates);

	m_genL0Net->spawn(m_L0Nets);

	return ECODE_OK;
}
void TrainL0Stamp::release() {
	if (m_genL0Net != NULL)
		m_genL0Net->despawn(m_L0Nets);
	if (m_genLunaLayer != NULL)
		m_genLunaLayer->despawn(m_lunaPlates);
	if (m_genColLayer != NULL)
		m_genColLayer->despawn(m_colPlates);
	if (m_genHexEye != NULL)
		m_genHexEye->despawn(m_seye);
	if (m_L0Nets != NULL)
		delete m_L0Nets;
	m_L0Nets = NULL;
	if (m_lunaPlates != NULL)
		delete m_lunaPlates;
	m_lunaPlates = NULL;
	if (m_colPlates != NULL)
		delete m_colPlates;
	m_colPlates = NULL;

}
int TrainL0Stamp::getOs_len() {
	return m_genL0Net->getTotalNumNodes_perNet();
}
int TrainL0Stamp::getNumBottomOs() {
	return m_genL0Net->getTotalNumBottomOs_perNet();
}
int TrainL0Stamp::getNumLevels() {
	return m_genL0Net->getNumLevels_perNet();
}
unsigned char TrainL0Stamp::getNumNodesPerLevel(float levNodes[], int numLev) {
	return m_genL0Net->getNumNodesPerLevel_perNet(levNodes, numLev);
}
unsigned char TrainL0Stamp::genWhiteColWheel() {
	n_ColWheel::clear(m_sCol);
	n_ColWheel::setColWheelUnitVectors(m_sCol);
	m_sCol.pixMax = 255.f;
	m_sCol.Dhue = 1.f;
	m_sCol.DI = 0.5f;
	m_sCol.DSat = 0.4f;
	m_sCol.I_target = 1.f;
	m_sCol.Hue_target = { 0.f, 0.f };
	m_sCol.Sat_target = 0.f;
	m_sCol.HueFadeV = 2.f;
	m_sCol.finalScaleFactor = 3.f;
	return ECODE_OK;
}
unsigned char TrainL0Stamp::runImg(Img* baseImg, float net_os[]) {
	if (baseImg == NULL)
		return ECODE_ABORT;
	if (baseImg->getWidth() != m_baseImg->getWidth() || baseImg->getHeight() != m_baseImg->getHeight())
		return ECODE_ABORT;
	m_genHexedImg->run(baseImg, *m_seye);
	m_genColLayer->run(m_seye->getBottom(), m_colPlates);
	m_genLunaLayer->run(m_colPlates->get(0), m_lunaPlates);

	n_CNets::runNNet(m_L0Nets, m_lunaPlates, 0);/*plate_i is 0 for center*/
	s_Net* L0Net = m_L0Nets->get(0);
	int len_out = m_genL0Net->getNetGen()->dumpOsChain(L0Net, net_os);
	return ECODE_OK;
}

unsigned char TrainL0Stamp::runDir(std::string& stampDir){
	/*path to master key in file*/
	std::string masterKey_dir = stampDir;
	masterKey_dir += TRAINL0STAMP_MASTERKEYFILE;
	masterKey_dir += TRAINL0STAMP_KEYFILESUF;

	/*start of path to img infile*/
	std::string imgPre_dir = stampDir;
	imgPre_dir += TRAINL0STAMP_IMGFILE;

	/*path to out file*/
	std::string Ofile_dir = stampDir;
	Ofile_dir += TRAINL0STAMP_L0BASEOFILE;
	Ofile_dir += TRAINL0STAMP_L0BASEOFILESUF;

	/*getting from the master key the start of the img stamp nums to the final img stamp num*/
	ParseTxt Tparser;
	Tparser.init(masterKey_dir, Ofile_dir);
	s_datLine* fline_in = new s_datLine[1];
	Tparser.readCSV(fline_in, 1);
	int istart_num = (int)roundf(fline_in[0].v[1]);
	int iend_num = (int)roundf(fline_in[0].v[2]);
	delete [] fline_in;
	
	/*variables that will be used in dumping output*/
	int num_Os = getOs_len();
	m_net_os_scratch = new float[num_Os];
	int num_net_dumps = iend_num - istart_num + 1;
	int numWBs = m_genL0Net->getNetGen()->getTotalNumWBs();
	if (num_net_dumps < 1 || num_Os >= PARSETXT_MAXAR || numWBs>PARSETXT_MAXAR) {/*equal since id is also written*/
		ErrHandle(ECODE_FAIL, "error reading start and end numbers from key for img files");
		return ECODE_FAIL;
	}
	m_net_lines = new s_datLine[num_net_dumps];
	/*loop over image files*/
	int num_lines_out = 0;
	for (int id = istart_num; id <= iend_num; id++) {
		std::string imgpath = imgPre_dir;
		std::string imgsuf = "";
		if (Err(n_ParseTxt::intToFixedLenStr(id, TRAINL0STAMP_INTSTRLEN, imgsuf)))
			return ECODE_FAIL;
		imgpath += imgsuf;
		imgpath += TRAINL0STAMP_IMGFILESUF;
		if (!ErrHandle(m_tga->Open(imgpath.c_str()), "failed to open file"))
			return ECODE_ABORT;
		Img* baseImg = new Img;
		if (!ErrHandle(baseImg->init(m_tga->GetImage(), m_tga->GetWidth(), m_tga->GetHeight(), m_tga->GetColorMode()), "unknown error")) {
			delete baseImg;
			return ECODE_FAIL;
		}
		unsigned char errcode = ECODE_OK;
		if (!ErrHandle(runImg(baseImg, m_net_os_scratch), "runImg failed may be wrong image dimensions")) {
			errcode = ECODE_FAIL;
		}
		if (baseImg != NULL) {
			baseImg->release();
			delete baseImg;
		}
		m_tga->Close();
		m_net_lines[num_lines_out].v[0] = (float)id;
		for (int i_os = 0; i_os < num_Os; i_os++)
			m_net_lines[num_lines_out].v[i_os+1] = m_net_os_scratch[i_os];
		m_net_lines[num_lines_out].n = num_Os+1;
		num_lines_out++;
	}

	/*the last time the network was run the nets should have been rooted and run on a plate
	get the locations of the hexes from the base the last time this was done*/
	/*writing key that tells where the bottom hexes are located*/
	s_2pt_i oslochold[PARSETXT_MAXAR];
	int numOsLoc = 0;
	if (m_L0Nets->get(0) != NULL) {
		numOsLoc = m_genL0Net->getNetGen()->dumpOsChainLoc(m_L0Nets->get(0), oslochold);
	}
	/* end of saving bottom loc in oslochold*/

	if (!ErrHandle(Tparser.writeCSV(m_net_lines, num_lines_out), "error trying to write to out file")) {
		return ECODE_FAIL;
	}
	if (m_net_os_scratch != NULL) {
		delete [] m_net_os_scratch;
		m_net_os_scratch = NULL;
	}
	if (m_net_lines != NULL) {
		delete [] m_net_lines;
		m_net_lines = NULL;
	}
	/*second parser to write the key*/
	s_datLine key_arr[4];
	key_arr[0].v[0] = (float)getNumLevels();
	float levhold[PARSETXT_MAXAR];
	float wbshold[PARSETXT_MAXAR];

	getNumNodesPerLevel(levhold, (float)key_arr[0].v[0]);
	for (int i_lev = 0; i_lev < key_arr[0].v[0]; i_lev++)
		key_arr[0].v[i_lev + 1] = levhold[i_lev];
	key_arr[0].n = int(key_arr[0].v[0]);
	m_genL0Net->getNetGen()->dumpWBsChainRef(wbshold);
	for (int i_wb = 0; i_wb < numWBs; i_wb++)
		key_arr[1].v[i_wb] = wbshold[i_wb];
	key_arr[1].n = numWBs;
	/*extracting the data on the bottom plate locations and dumping it*/
	for (int i_os = 0; i_os < numOsLoc; i_os++) {
		key_arr[2].v[i_os] = oslochold[i_os].x0;
		key_arr[3].v[i_os] = oslochold[i_os].x1;
	}
	key_arr[2].n = numOsLoc;
	key_arr[3].n = numOsLoc;
	ParseTxt keyout;
	keyout.init();
	std::string keyOutDir = stampDir;
	keyOutDir += TRAINL0STAMP_L0TRAINKEY;
	keyOutDir += TRAINL0STAMP_KEYFILESUF;
	keyout.setOutFile(keyOutDir);
	if (!ErrHandle(keyout.writeCSV(key_arr,4), "Failed to write key out file"))
		return ECODE_FAIL;
	return ECODE_OK;
}
bool TrainL0Stamp::ErrHandle(unsigned char ercode, const char* errstr) {
	if (Err(ercode)) {
		std::cout << "ERROR: " << errstr;
		release();
		return false;
	}
	return true;
}