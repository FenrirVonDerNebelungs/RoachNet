#include "TwistedPlate.h"
TwistedPlate::TwistedPlate() :m_exHex(NULL), m_hexImg(NULL), m_numHanging(0), m_sigma(0.f), m_twistedRadius(0.f), m_sizeRootBase(0.f), m_Width(0L), m_Height(0L) {
	;
}
TwistedPlate::~TwistedPlate() {
	;
}

unsigned char TwistedPlate::init(HexImg* hexImg, HexStack* stack, float sigma, float twisted_root_radius_in_R) {
	if (stack == NULL || hexImg == NULL)
		return ECODE_FAIL;
	m_hexImg = hexImg;
	m_sigma = sigma;
	m_twistedRadius = stack->getBottomR() * twisted_root_radius_in_R;
	m_numHanging = getNumhangingBaseToRoot();/*used m_sigma and m_twistedRadius */
	m_exHex = new ExHex;
	unsigned char err = m_exHex->init(stack, m_numHanging);
	if (err != ECODE_OK)
		return err;

	float sizeExBase = (float)m_exHex->getBaseSize();
	float ex_root_radius = TWISTEDPLATE_root_plate_buffer_factor * m_twistedRadius;
	float size_root_with_twisted_expansion = sizeExBase + ex_root_radius;
	m_sizeRootBase = size_root_with_twisted_expansion;
	m_Width = (long)ceilf(m_sizeRootBase);
	m_Height = m_Width;
	return ECODE_OK;
}
void TwistedPlate::release() {
	if (m_exHex != NULL) {
		m_exHex->release();
		delete m_exHex;
		m_exHex = NULL;
	}
	m_numHanging = 0;
	m_sigma = 0.f;
}

unsigned char TwistedPlate::spawn(s_rtHexPlate* newplate, /*const*/ s_rtHexPlate* basePlate, float radang) {
	if (newplate == NULL || basePlate == NULL)
		return ECODE_ABORT;
	if (basePlate->N <= 0)
		return ECODE_ABORT;
	s_2pt twisted_hexUs[6];
	genTwistedUs(basePlate, radang, twisted_hexUs);
	if(Err(m_exHex->spawn(newplate, twisted_hexUs)))
		return ECODE_FAIL;
	float sigma_actual = m_sigma * newplate->Rhex;
	if(sigma_actual <= 0.f)
		return ECODE_ABORT;
	if (!(rootTwisted(newplate, basePlate, m_numHanging, sigma_actual)))
		return ECODE_FAIL;
	return ECODE_OK;
}
void TwistedPlate::despawn(s_rtHexPlate* plate) {
	if (m_exHex != NULL)
		m_exHex->despawn(plate);
}

int TwistedPlate::getNumhangingBaseToRoot() {
	float radiusBottom = m_twistedRadius;
	float area = PI * radiusBottom * radiusBottom;
	const float tri_height_ = sqrtf(3.f) / 2.f;
	const float tri_area = tri_height_ /* * base=1 */ / 2.f;
	const float hex_area = 6.f * tri_area;
	float bottom_hex_size = hex_area * m_hexImg->getRhex() * m_hexImg->getRhex();/*area requires r^2*/
	float num_hex_in_area = area / bottom_hex_size;
	int num_hex_to_link = (int)ceilf(num_hex_in_area);
	if (num_hex_to_link < TWISTEDPLATE_min_num_hanging)
		num_hex_to_link = TWISTEDPLATE_min_num_hanging;
	return num_hex_to_link;
}

void TwistedPlate::genTwistedUs(s_rtHexPlate* basePlate, float radang, s_2pt hexUs[]) {
	s_2pt basisU0 = { 1.f, 0.f };
	s_2pt basisU1 = { 0.f, 1.f };
	vecMath::setBasis(radang, basisU0, basisU1);
	for (int i = 0; i < 6; i++) {
		hexUs[i] = vecMath::convBasis(basisU0, basisU1, basePlate->hexU[i]);
	}
}
bool TwistedPlate::rootTwisted(s_rtHexPlate* topPlate, /*const*/ s_rtHexPlate* basePlate, int numHanging, float sigma) {
	if (topPlate == NULL || basePlate == NULL)
		return false;
	if (topPlate->N < 1)
		return false;
	int* linked_set_is = new int[numHanging];
	float* linked_set_dists = new float[numHanging];
	float* ws = new float[numHanging];
	float maxDist = sqrtf((float)(basePlate->height * basePlate->height + basePlate->width * basePlate->width));

	for (long hex_i = 0; hex_i < topPlate->N; hex_i++) {
		s_rtHex* topHex = topPlate->get(hex_i);
		s_2pt rotTopLoc = n_HexBase::Loc(topHex, topPlate->hexU, topPlate->RShex); //hexU is twisted
		/* set the i, j loc in the twistetd plate relative to the center of the plate, or eye center but in pixels */
		topHex->i = Math::ftol(rotTopLoc.x0);
		topHex->j = Math::ftol(rotTopLoc.x1);

		/*set the weights for the linked low hexes */
		for (int i = 0; i < numHanging; i++) {
			linked_set_is[i] = -1;
			linked_set_dists[i] = maxDist;
			ws[i] = 0.f;
		}
		for (long lohex_i = 0; lohex_i < basePlate->N; lohex_i++) {
			s_rtHex* baseHex = basePlate->getConst(lohex_i);
			if (baseHex == NULL)
				return false;
			s_2pt baseLoc = n_HexBase::Loc(baseHex, basePlate->hexU, basePlate->Rhex);
			float dr = vecMath::dist(rotTopLoc, baseLoc);
			float maxdistinset = -1.f;
			int maxdist_i = -1;
			for (int i = 0; i < numHanging; i++) {
				if (linked_set_dists[i] > maxdistinset) {
					maxdistinset = linked_set_dists[i];
					maxdist_i = i;
				}
			}
			if (maxdist_i < 0)
				return false;
			if (dr < maxdistinset) {
				linked_set_is[maxdist_i] = lohex_i;
				linked_set_dists[maxdist_i] = dr;
			}
		}
		reorderArraysByDist(linked_set_dists, linked_set_is, numHanging);
		float sum_ws = 0.f;
		topHex->N = 0;
		for (int i = 0; i < numHanging; i++) {
			s_rtHex* baseHex = basePlate->getConst(linked_set_is[i]);
			float dr = linked_set_dists[i];
			ws[i] = Math::Gaussian(dr, sigma);
			sum_ws += ws[i];
			topHex->nodes[i] = (s_Node*)baseHex;
			topHex->N++;
		}
		if (sum_ws <= 0.f)
			return false;
		for (int i = 0; i < numHanging; i++) {
			topHex->w[i] = ws[i] / sum_ws;
		}
	}
	return true;
}
bool TwistedPlate::reorderArraysByDist(float* dists, int* indices, int n) {
	if (dists == NULL || indices == NULL || n < 1)
		return false;
	for (int i = 0; i < n; i++) {
		for (int j = i + 1; j < n; j++) {
			if (dists[j] < dists[i]) {
				float temp_dist = dists[i];
				dists[i] = dists[j];
				dists[j] = temp_dist;
				int temp_index = indices[i];
				indices[i] = indices[j];
				indices[j] = temp_index;
			}
		}
	}
	return true;
}

bool n_TwistedPlate::run(s_rtHexPlate* twistedPlate) {
	for(long hex_i = 0; hex_i < twistedPlate->N; hex_i++) {
		s_rtHex* hex = twistedPlate->get(hex_i);
		if(hex == NULL)
			return false;
		float sum_w = 0.f;
		for (int i = 0; i < hex->N; i++) {
			s_rtHex* base_hex = hex->getHanging(i);
			float base_o = base_hex->o;
			sum_w += hex->w[i] * base_o;
		}
		hex->o = sum_w;
	}
	return true;
}