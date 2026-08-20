//#include "framework.h"
#include "ConvolHex.h"
ConvolHex::ConvolHex():m_img(NULL), m_Rhex(0.f), m_sigmaVsR(0.f), m_IMaskRVsR(0.f), m_sigma(0.f), m_gaussNorm(0.f), m_gaussExpConst(0.f), m_IMaskR(0.f)
{
	m_Mask.Mask_img=NULL;
	m_Mask.Mask = NULL;
	m_Mask.height = 0L;
	m_Mask.width = 0L;
	m_Mask.MaskCenter_x0 = 0.f;
	m_Mask.MaskCenter_x1 = 0.f;
	m_Mask.MaskBL_offset_x0 = 0.f;
	m_Mask.MaskBL_offset_x1 = 0.f;
}
ConvolHex::~ConvolHex()
{
	;
}

unsigned char ConvolHex::init(Img* img, float Rhex, float sigmaVsR, float IMaskRVsR)
{
	m_img = img;
	m_Rhex = Rhex;
	m_sigmaVsR = sigmaVsR;
	m_IMaskRVsR = IMaskRVsR;

	genIMask();
	return ECODE_OK;
}
void ConvolHex::release()
{
	if (m_Mask.Mask_img != NULL) {
		m_Mask.Mask_img->release();
		delete m_Mask.Mask_img;
	}
	m_Mask.Mask_img = NULL;
	if (m_Mask.Mask != NULL) {
		delete[] m_Mask.Mask;
	}
	m_Mask.Mask = NULL;
	m_Mask.height = 0L;
	m_Mask.width = 0L;
}

unsigned char ConvolHex::genIMask()
{
	/*assume m_Rhex and m_sigmaVsR m_IMaskRVsR have already been set */
	m_sigma = m_sigmaVsR * m_Rhex;
	if (m_sigma < 1.f)
		return ECODE_ABORT;
	m_gaussNorm = 1.f / (m_sigma * sqrt(2.f * PI));
	m_gaussExpConst = 2 * m_sigma * m_sigma;
	m_IMaskR = m_IMaskRVsR * m_Rhex;
	if (m_IMaskR < 1.f)
		return ECODE_ABORT;
	int sizeM = (int)ceilf(2 * m_IMaskR) + 1;
	m_Mask.Mask_img = new Img;
	if (m_Mask.Mask_img == NULL)
		return ECODE_MEMERR_FAIL;
	m_Mask.height = (long)sizeM;
	m_Mask.width = (long)sizeM;
	m_Mask.Mask_img->init(m_Mask.height, m_Mask.width, 1);
	m_Mask.Mask_img->clearToChar(0x00);
	long maskTotalSize = m_Mask.height * m_Mask.width;
	m_Mask.Mask = new float[maskTotalSize];
	for (int i = 0; i < maskTotalSize; i++)
		m_Mask.Mask[i] = 0.f;

	m_Mask.MaskCenter_x0 = roundf(m_IMaskR);
	m_Mask.MaskCenter_x1 = roundf(m_IMaskR);
	s_2pt MaskCenter = { m_Mask.MaskCenter_x0, m_Mask.MaskCenter_x1 };
};
	s_2pt centerPt = { 0.f, 0.f };
	float gaussMax = calcGaussian(centerPt);
	for (int j = 0; j < sizeM; j++) {
		for (int i = 0; i < sizeM; i++) {
			s_2pt pt = { (float)i, float(j) };
			int index = j * m_Mask.Mask_img->getWidth() + i;
			s_2pt ptRel = vecMath::v12(MaskCenter, pt);
			float gaussVal = calcGaussian(ptRel);
			m_Mask.Mask[index] = gaussVal/gaussMax;
			gaussVal = 255.f * gaussVal / gaussMax;
			gaussVal = roundf(gaussVal);
			if (gaussVal > 255.f)
				gaussVal = 255.f;
			if (gaussVal < 0.f)
				gaussVal = 0.f;
			unsigned char maskVal = (unsigned char)gaussVal;
			m_Mask.Mask_img->setChar(index, maskVal);
		}
	}
	m_Mask.MaskBL_offset_x0 = (long)roundf(m_Mask.MaskCenter_x0);
	m_Mask.MaskBL_offset_x1 = (long)roundf(m_Mask.MaskCenter_x1);
	return ECODE_OK;
}

float ConvolHex::calcGaussian(s_2pt& pt)
{
	float dist2 = pt.x0*pt.x0 + pt.x1*pt.x1;
	float expval = dist2 / m_gaussExpConst;
	float val = expf(-expval);
	return val * m_gaussNorm;
}

bool n_ConvolHex::convulMaskToHex(const Img* img, const s_ConvolHex& Mask, const s_2pt_i& hex_loc, float rgb[])
{
	/*not the fastest convul*/
	long i_start = hex_loc.x0 - Mask.MaskBL_offset_x0;
	long j_start = hex_loc.x1 - Mask.MaskBL_offset_x1;
	long i_big = i_start;
	long j_big = j_start;
	float r_ = 0.f;
	float g_ = 0.f;
	float b_ = 0.f;
	float cnt = 0.f;
	for (long j = 0; j < Mask.height; j++) {
		j_big = (j_start + j);
		i_big = i_start;
		for (long i = 0; i < Mask.width; i++) {
			if (img->inImg(i_big, j_big)) {
				//s_rgba brgba = m_img->GetRGBA(i_big, j_big);
				s_rgb brgba = img->GetRGB(i_big, j_big);
				long i_small = j * Mask.width + i;
				float maskVal = Mask.Mask[i_small];
				float R = (float)brgba.r;
				float G = (float)brgba.g;
				float B = (float)brgba.b;
				R *= maskVal;
				G *= maskVal;
				B *= maskVal;
				r_ += R;
				g_ += G;
				b_ += B;
				cnt += maskVal;
			}
			i_big++;
		}
	}
	rgb[0] = r_;
	rgb[1] = g_;
	rgb[2] = b_;
	if (cnt >= 0.00001f) {
		for (int i_rgb = 0; i_rgb < 3; i_rgb++) {
			rgb[i_rgb] /= cnt;
			if (rgb[i_rgb] >= 255.f)
				rgb[i_rgb] = 255.f;
			if (rgb[i_rgb] < 0.f)
				rgb[i_rgb] = 0.f;
		}
	}
	return true;
}
void n_ConvolHex::convulMaskToHex(s_ConvolHex MaskVars, s_convKernVars IOVars) {
	long hex_i = ((s_Hex*)IOVars.outHex[IOVars.hex_index])->i;
	long hex_j = ((s_Hex*)IOVars.outHex[IOVars.hex_index])->j;
	if (hex_i < 0 || hex_j < 0)
		return;
	long i_start = hex_i - MaskVars.MaskBL_offset_x0;
	long j_start = hex_j - MaskVars.MaskBL_offset_x1;
	long i_big = i_start;
	long j_big = j_start;
	float r_ = 0.f;
	float g_ = 0.f;
	float b_ = 0.f;
	float cnt = 0.f;
	for (long j = 0; j < MaskVars.height; j++) {
		j_big = (j_start + j);
		i_big = i_start;
		long img_index = j_big * IOVars.Img_width + i_big;
		img_index *= IOVars.Img_bpp;
		for (long i = 0; i < MaskVars.width; i++) {
			if (img_index < IOVars.Img_maxIndex) {
				long i_small = j * MaskVars.width + i;
				float maskVal = MaskVars.Mask[i_small];

				float R = (float)IOVars.img_pix[img_index];
				float G = (float)IOVars.img_pix[img_index + 1];
				float B = (float)IOVars.img_pix[img_index + 2];

				R *= maskVal;
				G *= maskVal;
				B *= maskVal;
				r_ += R;
				g_ += G;
				b_ += B;
				cnt += maskVal;
			}
			img_index += IOVars.Img_bpp;
		}
	}
	if (cnt >= 0.00001f) {
		r_ /= cnt;
		g_ /= cnt;
		b_ /= cnt;
	}

	((s_Hex*)IOVars.outHex[IOVars.hex_index])->setRGB(r_, g_, b_);
	((s_Hex*)IOVars.outHex[IOVars.hex_index])->setColSet();

	return;
}


namespace threadedConvol {
#ifndef MECVISPI_WIN
	inline bool convInImg(long width, long height, long x_i, long y_i) {
		if (x_i < 0 || y_i < 0)
			return false;
		if (x_i >= width || y_i >= height)
			return false;
		return true;
	}
	void* runConvThread(void* IOVarsVoid) {
		s_convKernVars* IOVars = (s_convKernVars*)IOVarsVoid;
		long indexShift = (long)IOVars->hex_index;
		/*divide number of hexes by number of threads*/
		long num_passes = IOVars->num_Hex / THREADEDCONVOL_NUMTHREADS;
		for (long i = 0; i < num_passes; i++) {
			long cur_index = i * THREADEDCONVOL_NUMTHREADS + indexShift;
			IOVars->hex_index = cur_index;
			n_Convul::convCellKernel(*IOVars);
		}
		pthread_exit(NULL);
		return NULL;
	}
#endif
}
 

