#include "HexRoot.h"

int n_HexRoot::revWeb_i(int web_i) {
	if (web_i < 0 || web_i > 5)
		return -1;
	return (web_i + 3) % 6;
}
int n_HexRoot::Web_i(int web_i) {
	while (web_i < 0)
		web_i += 6;
	while(web_i >= 6)
		web_i -= 6;
	return web_i;
}
int n_HexRoot::Web_dir(s_Hex* hex1, s_Hex* hex2) {
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
bool n_HexRoot::stichHexes(s_Hex* hex1, s_Hex* hex2, int web_i1) {
	if (hex1 == NULL || hex2 == NULL)
		return false;
	if (web_i1 < 0 || web_i1 > 5)
		return false;
	int web_i2 = revWeb_i(web_i1);
	hex1->setWeb((s_Hex*)hex2, web_i1);
	hex2->setWeb((s_Hex*)hex1, web_i2);
	return true;
}
bool n_HexRoot::weaveSharedHexToNeighbors(s_Hex* commonHex, s_Hex* hex1, s_Hex* hex2) {
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
bool n_HexRoot::weaveSharedHexNeighbors(/*const*/ s_Hex* commonHexNode, s_Hex* hex1, s_Hex* hex2) {
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