#include "Eye.h"
s_Eye::s_Eye() : refHexStack(NULL), rootPlate(NULL), eyeCores(NULL), N_Cores(0) {
	;
}
s_Eye::~s_Eye() {
	;
}
unsigned char s_Eye::init(int numCores){
	if (numCores < 1)
		return ECODE_ABORT;
	eyeCores = new s_EyeCore * [numCores];
	for (int i = 0; i < numCores; i++)
		eyeCores[i] = NULL;
	N_Cores = numCores;
	return ECODE_OK;
}
void s_Eye::release() {
	if (eyeCores != NULL) {
		for (int i = 0; i < N_Cores; i++) {
			if (eyeCores[i] != NULL) {
				delete eyeCores[i];
				eyeCores[i] = NULL;
			}
		}
		delete[] eyeCores;
		eyeCores = NULL;
	}
	N_Cores = 0;
}

Eye::Eye() : 
	m_hexImg(NULL), 
	m_hexStack(NULL), 
	m_hexRect(NULL), 
	m_twistedPlate(NULL), 
	m_Luna(NULL), 
	m_eyeNets(NULL), 
	m_eyeCore(NULL), 
	m_R_base(0.f), 
	m_twisted_root_radius_in_R(0.f),
	m_numRot(0),
	m_dAng(0.f),
	m_Angs(NULL) {
	;
}
Eye::~Eye() { ; }

unsigned char Eye::init(HexImg* hexImg, float r, float twisted_root_radius_in_R, float sigma, int numRot, int numCurvePatterns, int numStackLevels) {
	m_hexImg = hexImg;
	m_R_base = r;
	m_twisted_root_radius_in_R = twisted_root_radius_in_R;
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
	m_hexStack->init(m_R_base, numStackLevels);

	m_twistedPlate = new TwistedPlate;
	m_twistedPlate->init(m_hexImg, m_hexStack, sigma, m_twisted_root_radius_in_R);

	float root_base_dim = m_twistedPlate->getSizeRootBase();
	int span_root_plate = n_HexRect::minSpanOdd(root_base_dim, m_hexImg->getRhex());
	int lines_root_plate = n_HexRect::minLinesOdd(root_base_dim, m_hexImg->getRhex());
	m_hexRect = new rtHexRect;
	m_hexRect->init(span_root_plate, lines_root_plate, m_hexImg->getRhex());

	m_Luna = new Luna;
	m_Luna->init(m_hexStack, m_twistedPlate->getExHex());

	m_eyeNets = new EyeNets;
	m_eyeNets->init(m_hexStack, m_Luna, numCurvePatterns);

	m_eyeCore = new EyeCore;
	m_eyeCore->init(m_twistedPlate, m_Luna, m_eyeNets);
	return ECODE_OK;
}
unsigned char Eye::setWB(int net_i, s_Node_w weights[], s_Node_w biases[]) {
	return m_eyeNets->setWB(net_i, weights, biases);
}
void Eye::release() {
	if (m_eyeCore != NULL) {
		m_eyeCore->release();
		delete m_eyeCore;
		m_eyeCore = NULL;
	}
	if (m_eyeNets != NULL) {
		m_eyeNets->release();
		delete m_eyeNets;
		m_eyeNets = NULL;
	}
	if (m_Luna != NULL) {
		m_Luna->release();
		delete m_Luna;
		m_Luna = NULL;
	}
	if (m_twistedPlate != NULL) {
		m_twistedPlate->release();
		delete m_twistedPlate;
		m_twistedPlate = NULL;
	}
	if (m_hexRect != NULL) {
		m_hexRect->release();
		delete m_hexRect;
		m_hexRect = NULL;
	}
	if (m_hexStack != NULL) {
		m_hexStack->release();
		delete m_hexStack;
		m_hexStack = NULL;
	}
	if (m_Angs != NULL) {
		delete[] m_Angs;
		m_Angs = NULL;
	}
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
void Eye::despawn(s_Eye* eye) {
	if (eye == NULL)
		return;
	if (eye->eyeCores != NULL) {
		for (int i_core = 0; i_core < eye->N_Cores; i_core++) {
			if (eye->eyeCores[i_core] != NULL) {
				m_eyeCore->despawn(eye->eyeCores[i_core]);
				delete eye->eyeCores[i_core];
				eye->eyeCores[i_core] = NULL;
			}
		}
	}
	if (eye->rootPlate != NULL) {
		m_hexRect->despawn(eye->rootPlate);
		delete eye->rootPlate;
		eye->rootPlate = NULL;
	}
	if (eye->refHexStack != NULL) {
		m_hexStack->despawn(eye->refHexStack);
		delete eye->refHexStack;
		eye->refHexStack = NULL;
	}
	eye->release();
}


bool n_Eye::run(s_Eye* eye) {
	if (eye == NULL)
		return false;
	for (int i_root_hex = 0; i_root_hex < eye->rootPlate->N; i_root_hex++) {
		s_rtHex* rootHex = eye->rootPlate->get(i_root_hex);
		s_rtHex* baseHex = rootHex->getHanging(0);
		if (baseHex != NULL) {
			rootHex->setRGB(baseHex->rgb);
			rootHex->i = baseHex->i;
			rootHex->j = baseHex->j;
		}
		else{
			rootHex->clearRGB();
			rootHex->i = -1;
			rootHex->j = -1;
		}
		rootHex->o = filter(rootHex->rgb);
	}
	for(int i_core=0; i_core<eye->N_Cores; i_core++){
		if(!n_EyeCore::run(eye->eyeCores[i_core]))
			return false;
	}
	return true;
}
float n_Eye::filter(float rgb[]) {
	float len = sqrtf(rgb[0] * rgb[0] + rgb[1] * rgb[1] + rgb[2] * rgb[2]);
	return len * EYCORE_max_RGB_len_inv;
}