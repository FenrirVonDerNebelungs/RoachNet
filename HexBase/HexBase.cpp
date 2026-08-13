#include "HexBase.h"

int n_HexBase::revWeb_i(int web_i) {
	if (web_i < 0 || web_i > 5)
		return -1;
	return (web_i + 3) % 6;
}
int n_HexBase::Web_i(int web_i) {
	while (web_i < 0)
		web_i += 6;
	while(web_i >= 6)
		web_i -= 6;
	return web_i;
}
int n_HexBase::Web_dir(s_Hex* hex1, s_Hex* hex2) {
	if (hex1 == NULL || hex2 == NULL)
		return -1;
	for (int web_i = 0; web_i < 6; web_i++) {
		s_Hex* linkedHex = hex1->getWeb(web_i);
		if (linkedHex != NULL) {
			if (linkedHex->thislink == hex2->thislink)
				return web_i;
		}
	}
	return -1;
}

bool n_HexBase::stichHexes(s_Hex* hex1, s_Hex* hex2, int web_i1) {
	if (hex1 == NULL || hex2 == NULL)
		return false;
	if (web_i1 < 0 || web_i1 > 5)
		return false;
	int web_i2 = revWeb_i(web_i1);
	hex1->setWeb((s_Hex*)hex2, web_i1);
	hex2->setWeb((s_Hex*)hex1, web_i2);
	return true;
}
int n_HexBase::stichTogetherWebHexes(s_Hex* centerHex) {
	if (centerHex == NULL)
		return 0;
	int numStiched = 0;
	for (int web_i = 0; web_i < 6; web_i++) {
		s_Hex* firstHex = centerHex->getWeb(web_i);
		int next_web_i = Web_i(web_i + 1);
		s_Hex* nextHex = centerHex->getWeb(next_web_i);
		if (firstHex != NULL && nextHex != NULL) {
			int stich_web_i = Web_i(web_i + 2);
			stichHexes(firstHex, nextHex, stich_web_i);
			numStiched++;
		}
	}
	return numStiched;
}
bool n_HexBase::weaveSharedHexToNeighbors(s_Hex* commonHex, s_Hex* hex1, s_Hex* hex2) {
	if(commonHex==NULL || hex1==NULL || hex2==NULL)
		return false;
	bool Common = false;
	for(int web_i_hex1ToCommon=0; web_i_hex1ToCommon<6; web_i_hex1ToCommon++){
		s_Hex* nebHex1 = hex1->getWeb(web_i_hex1ToCommon);
		if(nebHex1->thislink==commonHex->thislink){
			for (int web_i_hex1ToHex2 = 0; web_i_hex1ToHex2 < 6; web_i_hex1ToHex2++) {
				s_Hex* hex2Test = hex1->getWeb(web_i_hex1ToHex2);
				if (hex2Test->thislink == hex2->thislink) {
					bool adjoiningHexes = abs(web_i_hex1ToCommon - web_i_hex1ToHex2) == 1;
					if (adjoiningHexes) {
						int web_i_CommonToHex2 = 0;
						if (web_i_hex1ToHex2 > web_i_hex1ToCommon)
							web_i_CommonToHex2 = Web_i(web_i_hex1ToCommon + 2);
						else
							web_i_CommonToHex2 = Web_i(web_i_hex1ToCommon - 2);
						stichHexes(commonHex, hex2, web_i_CommonToHex2);
						Common = true;
					}
				}
			}
		}
	}
	return Common;
}
bool n_HexBase::weaveSharedHexNeighbors(/*const*/ s_Hex* commonHexNode, s_Hex* hex1, s_Hex* hex2) {
	if (commonHexNode == NULL || hex1 == NULL || hex2 == NULL)
		return false;
	bool Common = false;
	int web_i_CommonToHex1 = Web_dir(commonHexNode, hex1);
	int web_i_CommonToHex2 = Web_dir(commonHexNode, hex2);
	if(web_i_CommonToHex1<0 || web_i_CommonToHex2<0)
		return false;
	int rot_index_1To2 = web_i_CommonToHex2 - web_i_CommonToHex1;
	if (abs(rot_index_1To2) != 1)
		return false;
	int web_hex1ToHex2 = 0;
	if(rot_index_1To2 >0)
		web_hex1ToHex2 = Web_i(web_i_CommonToHex1 + 2);
	else
		web_hex1ToHex2 = Web_i(web_i_CommonToHex1 - 2);
	stichHexes(hex1, hex2, web_hex1ToHex2);
	return Common;
}
bool n_HexBase::copyWeave(s_HexPlate* sewnPlate, s_HexPlate* loosePlate) {
	if (sewnPlate == NULL || loosePlate == NULL)
		return false;
	if (sewnPlate->N < loosePlate->N)
		return false;
	for (long hex_i = 0; hex_i < loosePlate->N; hex_i++) {
		s_Hex* looseHex = loosePlate->get(hex_i);
		s_Hex* sewnHex = sewnPlate->get(hex_i);
		if (looseHex == NULL || sewnHex == NULL)
			return false;
		for (int web_i = 0; web_i < 6; web_i++) {
			s_Hex* sewnWebHex = sewnHex->getWeb(web_i);
			if (sewnWebHex != NULL) {
				long sewnWebHexIndex = sewnWebHex->thislink;
				if (sewnWebHexIndex >= 0 && sewnWebHexIndex < loosePlate->N) {
					s_Hex* looseWebHex = loosePlate->get(sewnWebHexIndex);
					looseHex->setWeb(looseWebHex, web_i);
				}
			}
		}
	}
	return true;
}
bool n_HexBase::computeVecHexDistances(s_rtHexPlate* plate, int center_index) {
	s_rtHex* startHex = plate->get(center_index);
	for (int i = 0; i < 3; i++)
		startHex->setDv(i,0);
	startHex->setLocSetFlag(true);
	bool unconnectable_found = false;
	int plate_connection_loops = 0;
	do {
		for (long hex_i = 0; hex_i < plate->N; hex_i++) {
			s_rtHex* nextHex = plate->get(hex_i);
			unconnectable_found = true;
			if (!nextHex->getLocSetFlag()) {
				for (int web_i = 0; web_i < 6; web_i++) {
					s_rtHex* adjHex = nextHex->getWeb(web_i);
					if (adjHex != NULL) {
						if (adjHex->getLocSetFlag()) {
							unconnectable_found = false;
							if (web_i < 3) {
								nextHex->setDv(web_i, adjHex->getDv(web_i) + 1);
							}
							else {
								int rev_web_i = revWeb_i(web_i);
								nextHex->setDv(rev_web_i, adjHex->getDv(rev_web_i) - 1);
							}
							nextHex->setLocSetFlag(true);
						}
					}
				}
			}
		}
		plate_connection_loops++;
	} while (unconnectable_found && plate_connection_loops <= plate->N);
	if (unconnectable_found)
		return ECODE_FAIL;
	return ECODE_OK;
}
s_2pt n_HexBase::Loc(const s_rtHex* hex, const s_2pt hexU[], const float r) {
	s_2pt loc;
	loc.x0 = 0.f;
	loc.x1 = 0.f;
	if (hex == NULL)
		return loc;
	float dv[3];
	for (int i = 0; i < 3; i++)
		dv[i] = (float)hex->getDv(i);
	s_2pt vec0 = { hexU[0].x0 * dv[0], hexU[0].x1 * dv[0] };
	s_2pt vec1 = { hexU[1].x0 * dv[1], hexU[1].x1 * dv[1] };
	s_2pt vec2 = { hexU[2].x0 * dv[2], hexU[2].x1 * dv[2] };
	loc.x0 = (vec0.x0 + vec1.x0 + vec2.x0) * r;
	loc.x1 = (vec0.x1 + vec1.x1 + vec2.x1) * r;
	return loc;
}

