#include "TestL0.h"
namespace fs = std::filesystem;

TestL0::TestL0() :m_r(0.0), m_len_wbs(0), m_num_stamps(0), m_num_rendered_plates(0), 
m_baseDir(TRAINL0STAMP_BASEDIR), m_num_baseImgs(0), m_ids_stack(NULL),m_wbs_stack(NULL), 
m_renderedStampImgs(NULL),
m_tga(NULL), m_render(NULL),
m_genHexEye(NULL), m_genHexedImg(NULL), m_genColLayer(NULL), m_genLunaLayer(NULL), m_genL0Layer(NULL),
m_seye(NULL), m_colPlates(NULL), m_lunaPlates(NULL), m_L0Plates(NULL){
	for (int i = 0; i < TESTL0_MAXNUMINPUTIMGFILES; i++) {
		m_inFileNames[i] = NULL;
		m_baseImgs[i] = NULL;
	}
	utilStruct::zero2pt_i(m_eye_center);
	n_ColWheel::clear(m_sCol);
}
TestL0::~TestL0() {
	;
}
unsigned char TestL0::init() {
	m_baseDir += '/';
	m_baseDir += TRAINL0STAMP_L0DIR;
	m_baseDir += '/';
	std::string l0key_dir = m_baseDir;
	l0key_dir += TRAINL0STAMP_L0KEYFILE;
	l0key_dir += TRAINL0STAMP_KEYFILESUF;
	ParseTxt TParser;
	TParser.init();
	TParser.setInFile(l0key_dir);
	s_datLine fline_in[1];
	int numLinesRead  =TParser.readCSV(fline_in, 1);
	if (numLinesRead < 1) {
		std::cout << "Error Couldn't read file: " << l0key_dir << "\n";
		return ECODE_ABORT;
	}
	m_r = fline_in[0].v[0];
	m_len_wbs = static_cast<int>(floorf(fline_in[0].v[1]));
	m_num_stamps = static_cast<int>(floorf(fline_in[0].v[2]));
	if (m_r <= 0.0 || m_len_wbs < 1 || m_num_stamps < 1) {
		std::cout << "Error: corrupted data in " << l0key_dir << "\n";
		return ECODE_ABORT;
	}
	if (!ErrHandle(readInImgs(), "Failed to read in input images"))
		return ECODE_ABORT;
	int full_len_wbs_arr = m_len_wbs * m_num_stamps;
	m_wbs_stack = new float[full_len_wbs_arr];
	m_ids_stack = new float[m_num_stamps];
	if (!ErrHandle(readInWBs(m_ids_stack, m_wbs_stack), "Failed to read in the nnet weight file"))
		return ECODE_ABORT;

	m_tga = new CTargaImage;
	m_tga->Init();

	genWhiteColWheel();
	m_genHexEye = new HexEye;
	m_genHexedImg = new HexEyeImg;
	m_genColLayer = new Col;
	m_genLunaLayer = new LunaLayer;
	m_genL0Layer = new L0Layer;

	m_eye_center = m_baseImgs[0]->getCenter();
	s_2pt eye_center = { Math::ltof(m_eye_center.x0), Math::ltof(m_eye_center.x1) };
	if (!ErrHandle(m_genHexEye->init(m_r, TRAINL0STAMP_num_img_eye_lev),"Failed eye init probably wrong r dim"))
		return ECODE_FAIL;
	if (!ErrHandle(m_genHexedImg->init(m_baseImgs[0], eye_center, m_genHexEye)))
		return ECODE_FAIL;
	m_genColLayer->init(1);
	m_genColLayer->addCol(&m_sCol);
	m_genLunaLayer->init(m_r);
	int num_luna_plates = m_genLunaLayer->getNumLuna();
	if (!ErrHandle(m_genL0Layer->init(m_genLunaLayer, m_wbs_stack, m_len_wbs, m_num_stamps), "Failed to init L0Layer"))
		return ECODE_FAIL;
	if (m_wbs_stack != NULL) {
		delete[] m_wbs_stack;
	}
	m_wbs_stack = NULL;
	if (!ErrHandle(initStampRenderImgs(), "Error could not init images probably problem with image dim"))
		return ECODE_FAIL;

	/*data objects*/
	m_seye = new s_HexEye;
	m_colPlates = new s_ColPlateLayer;
	m_lunaPlates = new s_HexPlateLayer;
	m_L0Plates = new s_HexPlateLayer;

	m_genHexEye->spawn(m_seye);
	m_genHexedImg->root(m_baseImgs[0], *m_seye);
	m_genColLayer->spawn(m_seye->getBottom(), m_colPlates);
	m_genLunaLayer->spawn(m_colPlates->get(0), m_lunaPlates);
	m_genL0Layer->spawn(m_lunaPlates, m_L0Plates);

	m_render = new RenderBase;
	if (ErrHandle(m_render->init(m_r), "Uknown error failed to generate render"))
		return ECODE_FAIL;
	return ECODE_OK;
}
void TestL0::release() {
	if (m_render != NULL) {
		m_render->release();
	}
	m_render = NULL;
	if (m_genL0Layer != NULL) 
		m_genL0Layer->despawn(m_L0Plates);
	if (m_genLunaLayer != NULL)
		m_genLunaLayer->despawn(m_lunaPlates);
	if (m_genColLayer != NULL)
		m_genColLayer->despawn(m_colPlates);
	if (m_genHexEye != NULL)
		m_genHexEye->despawn(m_seye);
	if (m_L0Plates != NULL)
		delete m_L0Plates;
	m_L0Plates = NULL;
	if (m_lunaPlates != NULL)
		delete m_lunaPlates;
	m_lunaPlates = NULL;
	if (m_colPlates != NULL)
		delete m_colPlates;
	m_colPlates = NULL;
	if (m_seye != NULL)
		delete m_seye;
	m_seye = NULL;
	releaseStampRenderImgs();
	if (m_genL0Layer != NULL) {
		m_genL0Layer->release();
		delete m_genL0Layer;
		m_genL0Layer = NULL;
	}
	if (m_genColLayer != NULL) {
		m_genColLayer->release();
		delete m_genColLayer;
		m_genColLayer = NULL;
	}
	if (m_genHexedImg != NULL) {
		m_genHexedImg->release();
		delete m_genHexedImg;
		m_genHexedImg = NULL;
	}
	if (m_genHexEye != NULL) {
		m_genHexEye->release();
		delete m_genHexEye;
		m_genHexEye = NULL;
	}

	if (m_tga != NULL) {
		m_tga->Release();
		delete m_tga;
		m_tga = NULL;
	}
	if (m_wbs_stack != NULL) {
		delete[] m_wbs_stack;
		m_wbs_stack = NULL;
	}
	if (m_ids_stack != NULL) {
		delete[] m_ids_stack;
		m_ids_stack = NULL;
	}
	releaseInImgs();
}
unsigned char TestL0::run() {
	if (m_num_baseImgs < 1)
		return ECODE_ABORT;
	for (int i = 0; i < m_num_baseImgs; i++) {
		if (ErrHandle(runAndWriteRenderForImg(i), "Error: failed to run input image and write out rendered plates"))
			return ECODE_FAIL;
	}
	return ECODE_OK;
}
unsigned char TestL0::genWhiteColWheel() {
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
unsigned char TestL0::readInImgs() {
	std::string imgDir = m_baseDir;
	imgDir += TESTL0_TESTDIR;
	imgDir += '/';
	imgDir += TESTL0_INDIR;
	int cnt_num_files=0;
	std::string imgkeystr = TESTL0_IMGINFILE;
	std::string imgsuf = TESTL0_IMGFILESUF;
	CTargaImage tgaIO;
	tgaIO.Init();
	for (const auto& entry : fs::directory_iterator(imgDir)) {
		if (cnt_num_files >= TESTL0_MAXNUMINPUTIMGFILES)
			break;
		std::string imgfilename = (entry.path()).string();
		size_t strtestpos = imgfilename.find(imgkeystr);
		std::string strippedName;
		if (strtestpos != std::string::npos && n_ParseTxt::stripStringBeforeSuf(imgsuf,imgfilename,strippedName)) {
			std::string imgPath = imgDir;
			imgDir += imgfilename;
			if (RetOk(tgaIO.Open(imgDir.c_str()))) {
				unsigned char* tgaImgDat = tgaIO.GetImage();
				m_baseImgs[cnt_num_files] = new Img;
				if (Err(m_baseImgs[cnt_num_files]->init(tgaImgDat, tgaIO.GetWidth(), tgaIO.GetHeight(), tgaIO.GetColorMode()))) {
					delete m_baseImgs[cnt_num_files];
					m_baseImgs[cnt_num_files]=NULL;
				}
				else {
					m_inFileNames[cnt_num_files] = new std::string(strippedName);
					cnt_num_files++;
				}
				tgaIO.Close();
			}
		}
	}
	m_num_baseImgs = cnt_num_files;
	if (m_num_baseImgs < 1)
		return ECODE_ABORT;

	return ECODE_OK;
}
void TestL0::releaseInImgs() {
	for (int i = 0; i < TESTL0_MAXNUMINPUTIMGFILES; i++) {
		if (m_inFileNames[i] != NULL) {
			delete m_inFileNames[i];
			m_inFileNames[i] = NULL;
		}
		if (m_baseImgs[i] != NULL) {
			m_baseImgs[i]->release();
			delete m_baseImgs[i];
			m_baseImgs[i] = NULL;
		}
	}
	m_num_baseImgs = 0;
}
unsigned char TestL0::readInWBs(float ids_arr[], float wbs_arr[]) {
	std::string wbsFile = m_baseDir;
	wbsFile += TRAINL0STAMP_L0BASEOFILE;
	wbsFile += TRAINL0STAMP_L0BASEOFILESUF;
	ParseTxt TParser;
	TParser.init();
	TParser.setInFile(wbsFile);
	s_datLine* netOs = new s_datLine[m_num_stamps];
	int num_stamps_read = TParser.readCSV(netOs, m_num_stamps);
	/*expected line length is the number of wbs nodes +1 for the id at the front*/
	int line_len_good = m_len_wbs + 1;
	if (num_stamps_read != m_num_stamps) {
		delete[] netOs;
		TParser.release();
		return ECODE_ABORT;
	}
	for (int stamp_i = 0; stamp_i < m_num_stamps; stamp_i++) {
		if (netOs[stamp_i].n != line_len_good) {
			delete[] netOs;
			TParser.release();
			return ECODE_ABORT;
		}
		ids_arr[stamp_i] = netOs[stamp_i].v[0];
		for (int wbs_i = 0; wbs_i < m_len_wbs; wbs_i++) {
			int netOs_i = wbs_i + 1;
			int wbs_arr_start_i = m_len_wbs * stamp_i;
			int wbs_arr_i = wbs_arr_start_i + wbs_i;
			wbs_arr[wbs_arr_i] = netOs[stamp_i].v[netOs_i];
		}
	}
	TParser.release();
	delete[] netOs;
	return ECODE_OK;
}
unsigned char TestL0::initStampRenderImgs() {
	if (m_num_stamps < 1 || m_baseImgs[0]==NULL)
		return ECODE_FAIL;
	m_num_rendered_plates = 1 + m_genLunaLayer->getNumLuna() + m_num_stamps;
	m_renderedStampImgs = new Img * [m_num_rendered_plates];
	long imgwidth = m_baseImgs[0]->getWidth(); 
	long imgheight = m_baseImgs[0]->getHeight();
	long imgcolmode = m_baseImgs[0]->getColorMode();
	for (int i = 0; i < m_num_rendered_plates; i++)
		m_renderedStampImgs[i] = NULL;
	for (int i = 0; i < m_num_rendered_plates; i++) {
		m_renderedStampImgs[i] = new Img;
		if (Err(m_renderedStampImgs[i]->init(imgwidth, imgheight, imgcolmode))) {
			return ECODE_FAIL;
		}
	}
	return ECODE_OK;
}
void TestL0::releaseStampRenderImgs() {
	if (m_renderedStampImgs != NULL) {
		for (int i = 0; i < m_num_rendered_plates; i++) {
			if (m_renderedStampImgs[i] != NULL) {
				m_renderedStampImgs[i]->release();
				delete m_renderedStampImgs[i];
				m_renderedStampImgs[i] = NULL;
			}
		}
		delete[] m_renderedStampImgs;
		m_renderedStampImgs = NULL;
	}
	m_num_rendered_plates = 0;
}
unsigned char TestL0::runAndWriteRenderForImg(int img_index) {
	if (Err(runOnImg(m_baseImgs[img_index])))
		return ECODE_FAIL;
	if (Err(renderImgs())) /*fills m_renderedStampImgs*/
		return ECODE_FAIL;
	std::string render_img_dir_base = m_baseDir;
	render_img_dir_base += TESTL0_TESTDIR;
	render_img_dir_base += "/";
	render_img_dir_base += TESTL0_OUTDIR;
	render_img_dir_base += "/";
	/*now add the first part of the string that is the same for every product of the base image*/
	render_img_dir_base += *(m_inFileNames[img_index]);
	/*col img*/
	std::string colimglabel = TESTL0_OUTCOL;
	if (Err(writeRenderImg(render_img_dir_base, colimglabel, m_renderedStampImgs[0])))
		return ECODE_ABORT;
	std::string lunaimglabelpre = TESTL0_OUTSPACERLUNA;
	for (long i_luna = 0; i_luna < m_lunaPlates->N; i_luna++) {
		std::string lunanumstr;
		n_ParseTxt::intToFixedLenStr(i_luna, TRAINL0STAMP_INTSTRLEN, lunanumstr);
		std::string lunaimglabel(lunaimglabelpre);
		lunaimglabel += lunanumstr;
		if (Err(writeRenderImg(render_img_dir_base, lunaimglabel, m_renderedStampImgs[i_luna + 1])))
			return ECODE_ABORT;
	}
	int stamp_offset = m_num_rendered_plates - m_num_stamps;
	std::string stamplabelpre = TESTL0_OUTSPACERL0;
	for (long i_stamp = 0; i_stamp < m_L0Plates->N; i_stamp++) {
		std::string stampnumstr;
		n_ParseTxt::intToFixedLenStr(i_stamp, TRAINL0STAMP_INTSTRLEN, stampnumstr);
		std::string stampimglabel(stamplabelpre);
		stampimglabel += stampnumstr;
		if (Err(writeRenderImg(render_img_dir_base, stampimglabel, m_renderedStampImgs[stamp_offset + i_stamp])))
			return ECODE_ABORT;
	}
	return ECODE_OK;
}
unsigned char TestL0::runOnImg(Img* netImg) {
	if (netImg == NULL)
		return ECODE_ABORT;
	if (netImg->getWidth() != m_baseImgs[0]->getWidth() || netImg->getHeight() != m_baseImgs[0]->getHeight())
		return ECODE_ABORT;
	m_genHexedImg->run(netImg, *m_seye);
	m_genColLayer->run(m_seye->getBottom(), m_colPlates);
	m_genLunaLayer->run(m_colPlates->get(0), m_lunaPlates);
	for (long plate_i = 0; plate_i < m_lunaPlates->get(0)->N; plate_i++) {
		n_L0Layer::run(m_genL0Layer, m_lunaPlates, m_L0Plates, plate_i);
	}
	return ECODE_OK;
}
unsigned char TestL0::renderImgs() {
	if (Err(renderHexPlate(m_colPlates->get(0), m_renderedStampImgs[0])))
		return ECODE_FAIL;
	for (int i = 0; i < m_num_rendered_plates; i++)
		m_renderedStampImgs[i]->clearToChar(TESTL0_render_null_col);
	for (long i_luna = 0; i_luna < m_lunaPlates->N; i_luna++) {
		if (Err(renderHexPlate(m_lunaPlates->get(i_luna), m_renderedStampImgs[i_luna + 1])))
			return ECODE_FAIL;
	}
	int stamp_offset = m_num_rendered_plates - m_num_stamps;
	if (stamp_offset != m_L0Plates->N)
		return ECODE_FAIL;
	for (long i_stamp = 0; i_stamp < m_L0Plates->N; i_stamp++) {
		long s_index = stamp_offset + i_stamp;
		if (Err(renderHexPlate(m_L0Plates->get(i_stamp), m_renderedStampImgs[s_index])))
			return ECODE_FAIL;
	}
	return ECODE_OK;
}
unsigned char TestL0::colHexPlate(s_HexPlate* plt) {
	for (long i = 0; i < plt->N; i++) {
		s_Hex* curhex = plt->get(i);
		float o = curhex->o;
		float o_scaled = RENDERBASE_pix_max * o;
		for (int i_rgb = 0; i_rgb < 3; i_rgb++)
			curhex->rgb[i_rgb] = o_scaled;
	}
	return ECODE_OK;
}
unsigned char TestL0::renderHexPlate(s_HexPlate* plt, Img* renderedimg) {
	colHexPlate(plt);
	s_2pt_i center = { 0L, 0L };
	return m_render->RenderHexPlate(plt, center, renderedimg);
}
unsigned char TestL0::writeRenderImg(const std::string& render_img_dir_base, const std::string& imglabel, Img* renderedImg) {
	std::string col_img_dir = render_img_dir_base;
	col_img_dir += imglabel;
	col_img_dir += TESTL0_IMGFILESUF;
	if (Err(m_tga->Open(renderedImg->getImg(), (unsigned short)renderedImg->getWidth(), (unsigned short)renderedImg->getHeight(), false, IMAGE_RGB)))
		return ECODE_FAIL;
	if (Err(m_tga->Write(col_img_dir.c_str(), (unsigned char)renderedImg->getColorMode()))) {
		m_tga->Close();
		return ECODE_ABORT;
	}
	m_tga->Close();
	return ECODE_OK;
}
bool TestL0::ErrHandle(unsigned char errc, const char* errmsg) {
	if (Err(errc)) {
		release();
		std::cout <<"Error: "<< errmsg << '\n';
		return false;
	}
	return true;
}