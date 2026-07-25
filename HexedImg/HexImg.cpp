#include "HexImg.h"

HexImg::HexImg() :m_img(NULL), m_p(NULL), m_Convol(NULL), m_toppixHex(0.f)
{
	utilStruct::zero2pt(m_nWH);
	utilStruct::zero2pt(m_BR);
	utilStruct::zero2pt_i(m_hexMaskBL_offset);
}
HexImg::~HexImg() {
	;
}

unsigned char HexImg::init(Img* img,
	s_HexBasePlate* plate,
	float Rhex,
	float sigmaVsR,
	float IMaskRVsR) {
	m_img = img;
	m_p = plate;
	m_p->initRs(Rhex);
	m_nWH = genMeshLoc(m_p->Rhex, m_p->RShex, m_p->Shex);
	long hex_size = (long)ceilf(2.f * m_nWH.x0 * m_nWH.x1);
	((s_HexPlate*)m_p)->init(hex_size);
	if(Err(genMesh()))
		return ECODE_FAIL;
	if (Err(genPlateRowStart()))
		return ECODE_FAIL;
	m_Convol = new ConvolHex;
	m_Convol->init(m_img, m_p->getNodes(), m_p->Rhex, sigmaVsR, IMaskRVsR);
	utilStruct::zero2pt(m_nWH);
	utilStruct::zero2pt(m_BR);
	utilStruct::zero2pt_i(m_hexMaskBL_offset);
	m_toppixHex = 0.f;
	return ECODE_OK;
}
void HexImg::release() {
	if (m_Convol != NULL) {
		m_Convol->release();
		delete m_Convol;
	}
	m_Convol = NULL;
	if (m_p != NULL) {
		releasePlateRowStart();
		m_p->release();
	}
	m_p = NULL;/* do not delete m_p since it is owned by another object*/
	m_img = NULL;
}
unsigned char HexImg::update(Img* img) {
	if (img->getWidth() != m_img->getWidth() || img->getHeight() != m_img->getHeight())
		return ECODE_ABORT;
	m_img = img;
	return run();
}
unsigned char HexImg::genMesh() {
	if(Err(genMeshLocFromBR(m_nWH.x0, m_nWH.x1)))
		return ECODE_FAIL;
	if (Err(genMeshWeb()))
		return ECODE_FAIL;
	return ECODE_OK;
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
unsigned char HexImg::run()
{
	for (long i = 0; i < m_p->N; i++) {
		m_Convol->convulToHex(i);
	}
	return ECODE_OK;
}
#endif
s_2pt HexImg::genMeshLoc(float Rhex, float RShex, float Shex) {
	float width = (float)m_img->getWidth();
	float height = (float)m_img->getHeight();
	float segLen_y = Rhex + (Shex / 2.f);
	float segLen_x = RShex * 2.f;
	float nW = floorf(width / segLen_x);
	float nH = floorf(height / segLen_y);
	nH--;
	s_2pt nWnH = { -1.f, -1.f };
	if (nW < 1.f || nH < 1.f)
		return nWnH;
	float w_margin = width - nW * segLen_x;
	w_margin /= 2.f;
	float h_margin = height - nH * segLen_y;
	h_margin /= 2.f;

	m_BR.x0 = w_margin + RShex;
	m_BR.x1 = h_margin + Rhex;
	nWnH.x0 = nW;
	nWnH.x1 = nH;
	return nWnH;
}
unsigned char HexImg::genMeshWeb() {
	if (m_p->N_wHex < 2)
		return ECODE_FAIL;
	/*rotate counterclockwise starting from full right pos x*/
	int index = 0;
	int rHigh_index = 0;
	for (int j = 1; j < m_p->N_hHex; j += 2) {
		for (int i = 0; i < (m_p->N_wHex - 1); i++) {
			index = j * m_p->N_wHex + i;
			s_Hex* hex = m_p->get(index);
			if (hex->i >= 0) {
				long target_index= (i < (m_p->N_wHex - 2)) ? index + 1 : -1;
				m_p->setWeb(index, 0, target_index);
				if (j >= 1) {
					m_p->setWeb(index, 1, index - m_p->N_wHex + 1);
					m_p->setWeb(index, 2, index - m_p->N_wHex);
				}
				target_index = (i > 0) ? index - 1 : -1;
				m_p->setWeb(index, 3, target_index);
				if (j <= (m_p->N_hHex - 2)) {
					m_p->setWeb(index, 4, index + m_p->N_wHex);
					m_p->setWeb(index, 5, hex->web[4]->thislink + 1);
				}
			}
		}
	}
	for (int j = 0; j < m_p->N_hHex; j += 2) {
		index = j * m_p->N_wHex;
		m_p->setWeb(index, 0, index + 1);
		if (j >= 1)
			m_p->setWeb(index, 1, index - m_p->N_wHex);
		if (j <= (m_p->N_hHex - 2))
			m_p->setWeb(index, 5, index + m_p->N_wHex);
		for (int i = 1; i < (m_p->N_wHex - 1); i++) {
			index = j * m_p->N_wHex + i;
			s_Hex* hex = m_p->get(index);
			m_p->setWeb(index, 0, index + 1);
			if (j >= 1) {
				m_p->setWeb(index, 1, index - m_p->N_wHex);
				m_p->setWeb(index, 2, hex->web[1]->thislink - 1);
			}
			m_p->setWeb(index, 3, index - 1);
			if (j <= (m_p->N_hHex - 2)) {
				m_p->setWeb(index, 5, index + m_p->N_wHex);
				m_p->setWeb(index, 4, hex->web[5]->thislink - 1);
			}
		}
		index = j * m_p->N_wHex + (m_p->N_wHex - 1);
		if (j >= 1)
			m_p->setWeb(index, 2, index - m_p->N_wHex - 1);
		m_p->setWeb(index, 3, index - 1);
		if (j <= (m_p->N_hHex - 2))
			m_p->setWeb(index, 4, index + m_p->N_wHex - 1);
	}
	return ECODE_OK;
}
unsigned char HexImg::genMeshLocFromBR(float nW, float nH) {
	float Rhex = m_p->Rhex;
	float RShex = m_p->RShex;
	float Shex = m_p->Shex;
	float segLen_y = Rhex + (Shex / 2.f);
	float segLen_x = RShex * 2.f;
	float w_margin = m_BR.x0;
	float h_margin = m_BR.x1;
	int n_w = (int)nW;
	int n_h = (int)ceilf(nH / 2.f);

	float x = w_margin;
	float y;
	for (int j = 0; j < n_h; j++) {
		y = h_margin + 2.f * segLen_y * (float)j;
		x = w_margin;
		for (int i = 0; i < n_w; i++) {
			int index = 2 * j * n_w + i;
			s_Hex* hex = m_p->get(index);
			hex->x = x;
			hex->y = y;
			hex->i = (long)roundf(x);
			hex->j = (long)roundf(y);
			hex->thislink = index;
			x += segLen_x;
		}
	}
	m_p->N = n_w * n_h;
	m_p->N_wHex = n_w;
	m_p->N_hHex = n_h;
	n_h = (int)floorf(nH / 2.f);
	int n_w_full = n_w;
	n_w -= 1;
	for (int j = 0; j < n_h; j++) {
		y = h_margin + segLen_y + 2.f * segLen_y * (float)j;
		x = w_margin + m_p->RShex;
		for (int i = 0; i < n_w; i++) {
			int index = (2 * j + 1) * n_w_full + i;
			s_Hex* hex = m_p->get(index);
			hex->x = x;
			hex->y = y;
			hex->i = (long)roundf(x);
			hex->j = (long)roundf(y);
			hex->thislink = index;
			x += segLen_x;
		}
	}
	m_p->N += n_w_full * n_h;
	m_p->N_hHex += n_h;
	return ECODE_OK;
}
unsigned char HexImg::genPlateRowStart() {
	m_p->Row_N = 0;
	m_p->Row_d = 3.f * m_p->Rhex / 2.f;
	m_p->Col_d = 2.f * m_p->RShex;
	for (long i = 0; i < m_p->N; i++) {
		s_Hex* hex = m_p->get(i);
		if (hex->web[3] == NULL && hex->x >= 0) {/* at the end of each row is an extra hex that is not connected or filled*/
			(m_p->Row_N)++;
		}
	}
	m_p->initRowStart(m_p->Row_N);
	long rowCnt = 0;
	for (long i = 0; i < m_p->N; i++) {
		s_Hex* hex = m_p->get(i);
		if (hex->web[3] == NULL && hex->x >= 0) {
			m_p->RowStart[rowCnt].x0 = hex->x;
			m_p->RowStart[rowCnt].x1 = hex->y;
			m_p->RowStart_is[rowCnt].x0 = i;
			m_p->RowStart_is[rowCnt].x1 = 0;
			s_Hex* endCandHex = m_p->get(i + m_p->RowStart_is[rowCnt].x1);
			while (endCandHex->web[0] != NULL) {
				(m_p->RowStart_is[rowCnt].x1) += 1;
			};
			rowCnt++;
		}
	}
	return ECODE_OK;
}
void HexImg::releasePlateRowStart() {
	m_p->releaseRowStart();
}