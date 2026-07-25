#include "HexEyeImg.h"
HexEyeImg::HexEyeImg() :m_genHexEye(NULL), m_imgWidth(0), m_imgHeight(0), m_Convol(NULL), m_refHexEye(NULL) {
	;
}
HexEyeImg::~HexEyeImg() {
	;
}
unsigned char HexEyeImg::init(Img* baseImg, s_2pt& eye_center, HexEye* genHexEye,
	float sigmaVsR,
	float IMaskRVsR) {
	if (baseImg == NULL)
		return ECODE_FAIL;
	if (baseImg->getWidth() <= 1)
		return ECODE_FAIL;
	if (baseImg->getHeight() <= 1)
		return ECODE_FAIL;
	if (genHexEye == NULL)
		return ECODE_FAIL;
	m_genHexEye = genHexEye;
	m_imgWidth = static_cast<float>(baseImg->getWidth());
	m_imgHeight = static_cast<float>(baseImg->getHeight());
	m_refHexEye = new s_HexEye;
	if(Err(m_genHexEye->spawn(m_refHexEye)))
		return ECODE_FAIL;
	long num_nodes_set = 0;
	for (int i_level = 0; i_level < m_refHexEye->N; i_level++) {
		s_HexPlate* lev_plate = m_refHexEye->get(i_level);
		for (long i_node = 0; i_node < lev_plate->N; i_node++) {
			s_Hex* hex_node = lev_plate->get(i_node);
			unsigned char err = resetNodeToImgCoord(baseImg, eye_center, hex_node);
			if (IsErrFail(err))
				return err;
			if (RetOk(err))
				num_nodes_set++;
		}
	}
	if (num_nodes_set < 1)
		return ECODE_ABORT;/*the eye is off the image*/
	m_Convol = new ConvolHex;
	if (Err(m_Convol->init(baseImg, m_genHexEye->getBottomR())))
		return ECODE_FAIL;
	return ECODE_OK;
}
void HexEyeImg::release() {
	if (m_Convol != NULL) {
		m_Convol->release();
		delete m_Convol;
	}
	m_Convol = NULL;
	if (m_genHexEye != NULL) {
		m_genHexEye->despawn(m_refHexEye);
	}
	m_genHexEye = NULL;
	if (m_refHexEye != NULL)
		delete m_refHexEye;
	m_imgWidth = 0.f;
	m_imgHeight = 0.f;
}

unsigned char HexEyeImg::root(Img* baseImg, s_HexEye& heye) {
	if (static_cast<float>(baseImg->getWidth()) != m_imgWidth || static_cast<float>(baseImg->getHeight()) != m_imgHeight)
		return ECODE_ABORT;
	if (m_refHexEye->N != heye.N)
		return ECODE_ABORT;
	for (int i_level = 0; i_level < m_refHexEye->N; i_level++) {
		s_HexPlate* ref_plate = m_refHexEye->get(i_level);
		s_HexPlate* heye_plate = heye.get(i_level);
		if (heye_plate == NULL)
			return ECODE_FAIL;
		if (ref_plate->N != heye_plate->N)
			return ECODE_FAIL;
		for (long i_node = 0; i_node < ref_plate->N; i_node++) {
			s_Hex* ref_node = ref_plate->get(i_node);
			s_Hex* heye_node = heye_plate->get(i_node);
			if (heye_node == NULL)
				return ECODE_FAIL;
			heye_node->i = ref_node->i;
			heye_node->j = ref_node->j;
			heye_node->x = ref_node->x;
			heye_node->y = ref_node->y;
		}
	}
	return ECODE_OK;
}
unsigned char HexEyeImg::run(Img* baseImg, s_HexEye& heye) {
	if (Err(runSingleThread(baseImg, heye)))
		return ECODE_FAIL;
	if (Err(n_HexEye::updateCol(&heye)))
		return ECODE_FAIL;
	return ECODE_OK;
}

unsigned char HexEyeImg::resetNodeToImgCoord(Img* baseImg, s_2pt& eye_center, s_Hex* hex_nd) {
	if (hex_nd == NULL)
		return ECODE_FAIL;
	/*get the offsets vs the center of the eye which is what genHexEye originally puts in the node*/
	float offset_x = hex_nd->x;
	float offset_y = hex_nd->y;
	float x=offset_x+eye_center.x0;
	float y = offset_y + eye_center.x1;
	/*reset the nodes xy to the image coordinate instead of the offset coordinates*/
	hex_nd->x = x;
	hex_nd->y = y;
	long _i = (long)lroundf(x);
	long _j = (long)lroundf(y);
	if (_i < 0 || _i>=baseImg->getWidth())
		_i = -1;
	if (_j < 0 || _j>=baseImg->getHeight())
		_j = -1;
	unsigned char retcode = ECODE_OK;
	if (_i < 0 || _j < 0) {/*check if node is off the image*/
		_i = -1; _j = -1;
		retcode = ECODE_ABORT;
	}
	hex_nd->i = _i;
	hex_nd->j = _j;
	return retcode;
}
unsigned char HexEyeImg::runSingleThread(Img* baseImg, s_HexEye& heye) {
	s_HexPlate* eyeBase = heye.getBottom();
	s_convKernVars kernVars;
	n_HexEyeImg::fillStruct(*baseImg, *m_Convol, *eyeBase, kernVars);
	for (long i = 0; i < kernVars.num_Hex; i++) {
		kernVars.hex_index = i;
		n_Convol::convCellKernel(kernVars);/*if the indexes of the current hex (ij) are either less than 0 this just returns*/
	}
	return ECODE_OK;
}

void n_HexEyeImg::fillStruct(Img& inimg, ConvolHex& genConvol, s_HexPlate& s_botPlate, s_convKernVars& IOVars) {
	IOVars.img_pix = inimg.getImg();
	IOVars.Img_height = inimg.getHeight();
	IOVars.Img_width = inimg.getWidth();
	IOVars.Img_bpp = inimg.getColorMode();
	IOVars.Img_maxIndex = inimg.getMaxIndex();

	IOVars.mask_pix = genConvol.getMaskF();
	IOVars.MaskBL_offsetX = genConvol.getIMaskBL_offset().x0;
	IOVars.MaskBL_offsetY = genConvol.getIMaskBL_offset().x1;
	IOVars.Mask_height = genConvol.getMaskHeight();
	IOVars.Mask_width = genConvol.getMaskWidth();

	IOVars.hex_index = 0;

	IOVars.num_Hex = s_botPlate.N;
	IOVars.outHex = s_botPlate.getNodes();
}