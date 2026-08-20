#include "HexImg.h"

HexImg::HexImg() :m_img(NULL), m_Convol(NULL)
{
	utilStruct::zero2pt(m_corner_margin);
	utilStruct::zero2pt_i(m_corner_start_hex_center);
}
HexImg::~HexImg() {
	;
}

unsigned char HexImg::init(Img* img,
	float r,
	float sigmaVsR,
	float IMaskRVsR) {
	if (m_img == NULL || r<=1.f)
		return ECODE_ABORT;
	m_img = img;
	m_R = r;
	float img_width = (float)m_img->getWidth();
	float img_height = (float)m_img->getHeight();
	if (img_width < 3.f || img_height < 3.f)
		return ECODE_ABORT;
	int maxSpan_hexes = n_HexRect::maxSpan(img_width, m_R);
	int maxLines_hexes = n_HexRect::maxLines(img_height, m_R);
	if (Err(rtHexRect::init(maxSpan_hexes, maxLines_hexes, m_R)))
		return ECODE_ABORT;
	m_Convol = new ConvolHex;
	if (Err(m_Convol->init(m_img, r, sigmaVsR, IMaskRVsR)))
		return ECODE_ABORT;
	float img_span_margin = (img_width - m_maxWidth)/2.f;
	float img_height_margin = (img_height - m_maxHeight)/2.f;
	if (img_span_margin <= 0.f || img_height_margin <= 0.f)
		return ECODE_FAIL;
	m_corner_margin.x0 = img_span_margin;
	m_corner_margin.x1 = img_height_margin;
	const float hex_half_height = m_R;
	const float hex_half_width = m_RS;
	float hex_X_start = img_span_margin + hex_half_width;
	float hex_Y_start = (img_height-1.f) - (img_height_margin + hex_half_height);
	m_corner_start_hex_center.x0 = (long)floorf(hex_X_start);
	m_corner_start_hex_center.x1 = (long)ceilf(hex_Y_start);
	if (m_corner_start_hex_center.x0 <= 0 || m_corner_start_hex_center.x1 >= m_img->getHeight())
		return ECODE_FAIL;
	return ECODE_OK;
}
void HexImg::release() {
	if (m_Convol != NULL) {
		m_Convol->release();
		delete m_Convol;
	}
	m_Convol = NULL;
	m_img = NULL;
	rtHexRect::release();
}

unsigned char HexImg::spawn(s_rtHexPlate* plate) {
	if (plate == NULL)
		return ECODE_ABORT;
	if(Err(rtHexRect::spawn(plate)))
		return ECODE_ABORT;
	long start_i = m_corner_start_hex_center.x0;
	long start_j = m_corner_start_hex_center.x1;
	for (long hex_i = 0; hex_i < m_numHexes; hex_i++) {
		s_rtHex* hex = plate->get(hex_i);
		s_2pt hex_rel_loc = n_HexBase::Loc(hex, m_U, m_R);
		long rel_i_loc = (long)roundf(hex_rel_loc.x0);
		long rel_j_loc = (long)roundf(hex_rel_loc.x1);
		long i_loc = start_i + rel_i_loc;
		long j_loc = start_j + rel_j_loc;
		if (i_loc < 0)
			i_loc = 0;
		if (i_loc >= m_img->getWidth())
			i_loc = m_img->getWidth();
		if (j_loc < 0)
			j_loc = 0;
		if (j_loc >= m_img->getHeight())
			j_loc = m_img->getHeight();
		hex->i = i_loc;
		hex->j = j_loc;
	}
	return ECODE_OK;
}
bool n_HexImg::update(s_rtHexPlate* plate, const Img* img, const s_ConvolHex& MaskVars) {
	for (long i_hex = 0; i_hex < plate->N; i_hex++) {
		s_rtHex* hex = plate->get(i_hex);
		s_2pt_i hex_loc = { hex->i, hex->j };
		float rgb[3] = { 0.f, 0.f, 0.f };
		if (!n_ConvolHex::convulMaskToHex(img, MaskVars, hex_loc, rgb))
			return false;
		hex->setRGB(rgb);
	}
	return true;
}
#ifndef MECVISPI_WIN
unsigned char HexImg::run()
{
	s_convKernVars IOVars = {
		m_img->getImg(),
		m_img->getHeight(),
		m_img->getWidth(),
		m_img->getColorMode(),
		m_img->getMaxIndex(),

		m_Convol->getMaskF(),
		m_Convol->getIMaskBL_offset().x0,
		m_Convol->getIMaskBL_offset().x1,
		m_Convol->getMaskHeight(),
		m_Convol->getMaskWidth(),

		0,

		m_p->N,
		m_p->getNodes()
	};
	s_convKernVars IOVars1 = IOVars;
	s_convKernVars IOVars2 = IOVars;
	s_convKernVars IOVars3 = IOVars;
	IOVars1.hex_index = 1;
	IOVars2.hex_index = 2;
	IOVars3.hex_index = 3;

	pthread_t thread0;
	pthread_t thread1;
	pthread_t thread2;
	pthread_t thread3;

	int thread_res = 0;
	thread_res = pthread_create(&thread0, NULL, threadedConvol::runConvThread, (void*)&IOVars);
	thread_res = pthread_create(&thread1, NULL, threadedConvol::runConvThread, (void*)&IOVars1);
	thread_res = pthread_create(&thread2, NULL, threadedConvol::runConvThread, (void*)&IOVars2);
	thread_res = pthread_create(&thread3, NULL, threadedConvol::runConvThread, (void*)&IOVars3);
	pthread_join(thread0, NULL);
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);

	/*finish of any extra if the number of hexes was not divisible by 4*/
	long num_passes = IOVars.num_Hex / THREADEDCONVOL_NUMTHREADS;
	long num_scanned = 4 * (num_passes);
	for (int i = num_scanned; i < m_p->N; i++) {
		IOVars.hex_index = i;
		threadedConvol::convCellKernel(IOVars);
	}
	//pthread_exit(NULL);
	return ECODE_OK;
}
#else
;
#endif
