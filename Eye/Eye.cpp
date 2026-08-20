#include "Eye.h"

unsigned char s_Eye::init(int numCores){
	if (numCores < 1)
		return ECODE_ABORT;
	eyeCores = new s_EyeCore * [numCores];
	for (int i = 0; i < numCores; i++)
		eyeCores[i] = NULL;
	N_Cores = numCores;
}

unsigned char Eye::init(HexImg* hexImg, float r, float twisted_root_radius, float sigma, int numRot, int numCurvePatterns, int numStackLevels) {
	m_hexImg = hexImg;
	m_twisted_root_radius = twisted_root_radius;
	m_numRot = 1;
	if (numRot >= 1)
		m_numRot = numRot;
	m_dAng = 2.f * PI / ((float)m_numRot);
	m_Angs = new float[m_numRot];
	float culmAng = 0.f;
	for (int rot_i = 0; rot_i < m_numRot; rot_i++) {
		m_Angs[rot_i] = culmAng;
		culmAng += m_dAng;
	}
	m_hexStack = new HexStack;
	m_hexStack->init(r, numStackLevels);

	float root_base_dim_in_stack_R = getSizeRootBase();
	float root_base_dim = m_hexStack->getBottomR() * root_base_dim_in_stack_R;
	int span_root_plate = n_HexRect::minSpanOdd(root_base_dim, m_hexImg->getRhex());
	int lines_root_plate = n_HexRect::minLinesOdd(root_base_dim, m_hexImg->getRhex());
	m_hexRect = new rtHexRect;
	m_hexRect->init(span_root_plate, lines_root_plate, m_hexImg->getRhex());

	int numHanging = getNumhangingBaseToRoot();
	m_exHex = new ExHex;
	m_exHex->init(m_hexStack, numHanging);

	m_twistedPlate = new TwistedPlate;
	m_twistedPlate->init(m_exHex, sigma);

	m_Luna = new Luna;
	m_Luna->init(m_exHex);

	m_eyeNets = new EyeNets;
	m_eyeNets->init(m_hexStack, m_Luna, numCurvePatterns);

	m_eyeCore = new EyeCore;
	m_eyeCore->init(m_twistedPlate, m_Luna, m_eyeNets);

}
unsigned char Eye::setWB(int net_i, s_Node_w weights[], s_Node_w biases[]) {
	return m_eyeNets->setWB(net_i, weights, biases);
}
unsigned char Eye::spawn(s_Eye* newEye) {
	if (newEye == NULL)
		return ECODE_ABORT;
	if (Err(newEye->init(m_numRot)))
		return ECODE_ABORT;
	unsigned char err=ECODE_OK;
	newEye->refHexStack = new s_HexStack;
	err = m_hexStack->spawn(newEye->refHexStack);
	if (Err(err))
		return err;
	newEye->rootPlate = new s_rtHexPlate;
	err=m_hexRect->spawn(newEye->rootPlate);
	if (Err(err))
		return err;
	for (int i_core = 0; i_core < m_numRot; i_core++) {
		newEye->eyeCores[i_core] = new s_EyeCore;
		err = m_eyeCore->spawn(m_Angs[i_core], newEye->rootPlate, newEye->eyeCores[i_core]);
		if (Err(err))
			return err;
	}
	return ECODE_OK;
}
float Eye::getSizeRootBase() {
	float hexStack_NumHexesLongDim = m_hexStack->getNumHexesLongDim();
	float sizeExBase = n_Eye::getSizeExBase(hexStack_NumHexesLongDim);
	return n_Eye::getSizeRootBase(sizeExBase, m_twisted_root_radius);
}

int Eye::getNumhangingBaseToRoot() {
	float radiusBottom = m_twisted_root_radius * m_hexStack->getBottomR();
	float area = PI * radiusBottom * radiusBottom;
	const float tri_height_ = sqrtf(3.f) / 2.f;
	const float tri_area = tri_height_ /* * base=1 */ / 2.f;
	const float hex_area = 6.f * tri_area;
	float bottom_hex_size = hex_area * m_hexImg->getRhex();
	float num_hex_in_area = area / bottom_hex_size;
	return (int)ceilf(num_hex_in_area);
}

float n_Eye::getSizeExBase(float hexStack_NumHexesLongDim) {
	float numHexes_stack_base = hexStack_NumHexesLongDim;
	float numHexes_ex_stack_base = numHexes_stack_base + 1.f;
	const float short_side = 1.f / 2.f;
	float base_long_dim_in_R = sqrtf(short_side * short_side + numHexes_ex_stack_base * numHexes_ex_stack_base);
	return base_long_dim_in_R;
}
float n_Eye::getSizeRootBase(float sizeExBase, float twisted_root_radius) {
	float base_long_dim_in_R = sizeExBase;
	float ex_root_radius = EYECORE_root_plate_buffer_factor * twisted_root_radius;
	return base_long_dim_in_R + ex_root_radius;
}