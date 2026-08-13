#include "TwistedPlate.h"

unsigned char TwistedPlate::init(ExHex* exHex, float sigma) {
	if (exHex == NULL)
		return ECODE_FAIL;
	m_exHex = exHex;
	m_numHanging = exHex->getNumHanging();
	m_sigma = sigma;
	return ECODE_OK;
}


unsigned char TwistedPlate::spawn(s_rtHexPlate* newplate, /*const*/ s_rtHexPlate* basePlate, float radang) {
	if (newplate == NULL || basePlate == NULL)
		return ECODE_ABORT;
	if (basePlate->N <= 0)
		return ECODE_ABORT;
	if(Err(m_exHex->spawn(newplate)))
		return ECODE_FAIL;
	float sigma_actual = m_sigma * newplate->Rhex;
	if(sigma_actual <= 0.f)
		return ECODE_ABORT;
	if (!(rootTwisted(newplate, basePlate, radang, m_numHanging, sigma_actual)))
		return ECODE_FAIL;
	return ECODE_OK;
}
bool TwistedPlate::rootTwisted(s_rtHexPlate* topPlate, /*const*/ s_rtHexPlate* basePlate, float radang, int numHanging, float sigma) {
	if (topPlate == NULL || basePlate == NULL)
		return false;
	if (topPlate->N < 1)
		return false;
	int* linked_set_is = new int[numHanging];
	float* linked_set_dists = new float[numHanging];
	float* ws = new float[numHanging];
	float maxDist = sqrtf((float)(basePlate->height * basePlate->height + basePlate->width * basePlate->width));
	for (int i = 0; i < numHanging; i++) {
		linked_set_is[i] = -1;
		linked_set_dists[i] = maxDist;
		ws[i] = 0.f;
	}
	s_2pt basisU0 = { 1.f, 0.f };
	s_2pt basisU1 = { 0.f, 1.f };
	vecMath::setBasis(radang, basisU0, basisU1);
	for (long hex_i = 0; hex_i < topPlate->N; hex_i++) {
		s_rtHex* topHex = topPlate->get(hex_i);
		s_2pt topLoc = n_HexBase::Loc(topHex, topPlate->hexU, topPlate->Rhex);
		s_2pt rotTopLoc = vecMath::convBasis(basisU0, basisU1, topLoc);
		vecMath::mul(topPlate->Rhex, rotTopLoc);
		for (long lohex_i = 0; lohex_i < basePlate->N; lohex_i++) {
			s_rtHex* baseHex = basePlate->getConst(lohex_i);
			if (baseHex == NULL)
				return false;
			s_2pt baseLoc = n_HexBase::Loc(baseHex, basePlate->hexU, basePlate->Rhex);
			float dr = vecMath::dist(rotTopLoc, baseLoc);
			dr *= basePlate->Rhex;
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
		for (int i = 0; i < numHanging; i++) {
			s_rtHex* baseHex = basePlate->getConst(linked_set_is[i]);
			float dr = linked_set_dists[i];
			ws[i] = Math::Gaussian(dr, sigma);
			sum_ws += ws[i];
			topHex->nodes[i] = (s_Node*)baseHex;
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
		return;
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