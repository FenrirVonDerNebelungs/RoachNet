#include "HexStack.h"
s_HexStack::s_HexStack() :lev(NULL), N(0), width(0L), height(0L), w_col_center(0.f), w_col_aux(0.f), N_mem(0) {
	;
}
s_HexStack::~s_HexStack() {
	;
}
unsigned char s_HexStack::init(int NumLev) {
	if (lev != NULL)
		return ECODE_ABORT;
	if (NumLev < 1)
		return ECODE_FAIL;
	lev = new s_HexPlate * [NumLev];
	if (lev == NULL)
		return ECODE_FAIL;
	N_mem = NumLev;
	for (int ii = 0; ii < N_mem; ii++)
		lev[ii] = NULL;
	N = 0;
	w_col_aux = 0.f;
	w_col_center = 0.f;
	return ECODE_OK;
}
unsigned char s_HexStack::init(const s_HexStack& other) {
	unsigned char err = init(other.N_mem);
	if (err != ECODE_OK)
		return err;
	this->N = other.N;
	this->width = other.width;
	this->height = other.height;
	this->w_col_center = other.w_col_center;
	this->w_col_aux = other.w_col_aux;
	for (int ii = 0; ii < N; ii++) {
		if (lev[ii] != NULL)
			err=lev[ii]->init(other.lev[ii]);
		if (err != ECODE_OK)
			return err;
	}
	if (N < 2)
		return ECODE_OK;
	/* now fix intra plate links */
	for (int ii = 0; ii < (N - 1); ii++) {
		s_Plate* topLev = this->lev[ii];
		s_Plate* lowLev = this->lev[ii + 1];
		n_Plate::fixStackedPlateLinks(topLev, lowLev);
	}
	return ECODE_OK;
}
void s_HexStack::release() {
	if (lev != NULL) {
		delete[] lev;
	}
	lev = NULL;
	N = 0;
	N_mem = 0;
	w_col_center = 0.f;
	w_col_aux = 0.f;
}

HexStack::HexStack() :m_r(0.f), m_R(0.f), m_N_levels(-1), m_N_hexes(NULL), m_baseWidth(0L), m_baseHeight(0L) {
	for (int i = 0; i < 6; i++)
		utilStruct::zero2pt(m_hexU[i]);
}
HexStack::~HexStack() {
	;
}

unsigned char HexStack::init(float r, int NLevels) {
	if (r < 1.f) return ECODE_ABORT;
	if (NLevels < 1) return ECODE_ABORT;
	m_r = r;
	m_R = r * Math::power(2.f, (NLevels - 1)); /*figure out what largest R is r*2^(N_level-1) */
	m_N_levels = NLevels;
	n_Hex::genHexU_0(m_hexU);
	m_N_hexes = new long[m_N_levels];
	if (Err(genNumHexesPerLevel()))
		return ECODE_FAIL;
	if (Err(spawn(&m_refEye)))
		return ECODE_FAIL;
	m_baseWidth = m_refEye.getBottom()->width;
	m_baseHeight = m_refEye.getBottom()->height;
	return ECODE_OK;
}
void HexStack::release() {
	m_baseHeight = 0L;
	m_baseWidth = 0L;
	despawn(&m_refEye);
	if (m_N_hexes != NULL)
		delete[] m_N_hexes;
	m_N_hexes = NULL;
	m_N_levels = -1;
	m_R = 0.f;
	m_r = 0.f;
}
unsigned char HexStack::spawn(s_HexStack* neye) {
	if (neye == NULL)
		return ECODE_ABORT;
	unsigned char err = initStack(neye);
	if (Err(err))
		return err;
	return genEye(neye);
}
void HexStack::despawn(s_HexStack* neye) {
	releaseStack(neye);
}
int HexStack::getNumEdgeHexes(int i_level) {
	/*
	* 1						= 1							i_level=0
	* 1*2 + 1				= 2^1 + 1					i_level=1
	* (1*2+1)*2 +1			= 2^2 + 2^1 + 1				i_level=2
	* ((1*2+1)*2 +1)*2 +1   = 2^3 + 2^2 + 2^1 + 1		i_level=3
	*/
	int numSpanHexes = (int)roundf(Math::powerXseries(2, i_level));
	int numEdgeHexes = 1;
	if (numSpanHexes > 1) {
		int numTriInteriorSegHexes = (numSpanHexes - 3) / 2;/*the number of interior hexes along a side of the triangle not including the ends*/
		numEdgeHexes = 6 * numTriInteriorSegHexes + 6;/*6 sides of the triangle plus 6 corner hexes*/
	}
	return numEdgeHexes;
}
float HexStack::getNumHexesLongDim(int N_level) {
	int Nlev = N_level;
	if (N_level < 0 || N_level >= m_N_levels)
		Nlev = m_N_levels - 1;
	/* 1+x+x^2 +.. = (1-x^(n+1))/(1-x) */
	float sideEx = 0.5f * (Math::powerXseries(0.5f, Nlev));/*(1/2) * (1+(1/2)+(1/2)^2+..(1/2)^N_level) */
	return 2.0f*sideEx;
}
unsigned char HexStack::genNumHexesPerLevel() {
	long N_hex = 0;
	for (int i = 0; i < m_N_levels; i++) {
		long N_hex_thisLevel = numHexInLevel(N_hex, i);
		m_N_hexes[i] = N_hex_thisLevel;
		N_hex = N_hex_thisLevel;
	}
	return ECODE_OK;
}

unsigned char HexStack::initStack(s_HexStack* neye) {
	/* m_R should already have been set along with m_N_levels and N_lowetNodePtrs*/
	if (neye == NULL)
		return ECODE_FAIL;
	if (Err(neye->init(m_N_levels)))
		return ECODE_FAIL;
	float R_lev = m_R;
	for (int i = 0; i < m_N_levels; i++) {
		long N_hex_thisLevel = m_N_hexes[i];
		if (neye->lev[neye->N] != NULL)
			return ECODE_FAIL;
		neye->lev[neye->N] = new s_HexPlate;
		if (neye->lev[neye->N] == NULL)
			return ECODE_FAIL;
		if (Err(neye->lev[neye->N]->init(N_hex_thisLevel)))/*this creates the s_Hex nodes and initializes them in the plate*/
			return ECODE_FAIL;
		neye->lev[neye->N]->initRs(R_lev);
		float levelDim = sizeOfLevel(i);/*N of level starts at 0*/
		neye->lev[neye->N]->height = (long)ceilf(levelDim);
		neye->lev[neye->N]->width = (long)ceilf(levelDim);
		(neye->N)++;
		R_lev /= 2.f;
	}
	neye->height = neye->lev[neye->N - 1]->height;
	neye->width = neye->lev[neye->N - 1]->width;
	/* w_center/w_aux= col_frac   w_center+6*w_aux=1
	* w_center = col_frac*w_aux   col_frac*w_aux + 6*w_aux=1
	* (col_frac+6)*w_aux=1
	*/
	if (HEXEYE_W_RATIO_COL_SUB_CENTER > 0.f)
		neye->w_col_aux = 1.f / (HEXEYE_W_RATIO_COL_SUB_CENTER + 6.f);
	neye->w_col_center = 1.f - 6.f * neye->w_col_aux;
	return ECODE_OK;
}
void HexStack::releaseStack(s_HexStack* neye) {
	if (neye != NULL) {
		for (int i = 0; i < neye->N; i++) {
			if (neye->lev[i] != NULL) {
				neye->lev[i]->release();
				delete neye->lev[i];
				neye->lev[i] = NULL;
			}
		}
		neye->release();
	}
}
long HexStack::numHexInLevel(long N_prev, int N_index) {
	/*for each level
	* for each previous level hex n_prev there will be 1 full hex and 6 half hexes or 4 hexes generated for the next level
	* this gives n_prev*4 for the fully overlapped new hexes
	* for each edge hex there will be half a new hex or 1/2 * n_new_edge_hexes
	* n_new_edge_hexes equals the number of hexes from the middle to the edge excluding the middle hex (since 2 half hexes contribute to each side and the number of hexes in a side is the same as the number of hexes including the middle to a edge point)
	* each edge hex contributes 2 hexes to this number (1 hex and 2 half hexes) except the end hex which contributes 1/2 hex and the middle hex also contributes 1/2 hex although it is not counted
	* this leads to n_prev_edge_to_point hexes contributing 2*n_prev_edge_to_point + 1
	*
	* so if there were n_l0 hexes and n_l0_point_to_edge hexes there will be
	* n_l1_internal hexes = n_l0*4
	* and n_l1_side hexes = 2*n_l0_point_to_edge - 1
	* where n_l1_side hexes = n_l1_point_to_edge - 1
	* n_l1_point_to_edge = 2* n_l0_point_to_edge
	*
	* n_l1 = n_l1_internal + 6*n_l1_side/2
	* n_l1 = 4*n_l0 + 6*(2*(n_l0_point_to_edge - 1))/2
	*
	* l_point_to_edge = 1, 2, 4, 8 = 2^{N-1} where N is the level ( and N-1 is the index of level)
	*
	* n_level_{N} = 4*n_level_{N-1} + 3*(2^{N-1}-1):  {N-1}= index
	*
	******* this function is passed the index not the level*******
	*/
	int N_level = N_index + 1;
	if (N_level == 1)
		return 1L;

	int N_prev_level = N_index;
	/*the total number of hexes along a side, including the end hexes*/
	float numFullSide = floorf(Math::power(2.f, N_prev_level));

	/*since each of the ends only contribute half a hex, remove the equivalent of one end */
	float numSide = numFullSide - 1.f;
	/* the number of no overlap per side is 6 half hexes per numSide
	which is the equivalent if 3 full hexes * numSide being added around the edge of the new level*/
	float num_non_overlap = 3.f * numSide;
	/*for each of the previous interior hexes a middle hex and 6 half hexes are added, or 4 new interior hexes are added for each prev hex*/
	float num_interior = N_prev * 4.f;
	/* the total number of new hexes spawned is the interior hexes plus the non overlapping edge hexes*/
	float numLevelHexes = num_interior + num_non_overlap;
	return (long)floorf(numLevelHexes);
}
float HexStack::sizeOfLevel(int N_level) {
	float sideEx = getNumHexesLongDim(N_level);
	sideEx /= 2.0f;
	float mini_side = 0.5f * Math::power(0.5f, N_level);
	mini_side *= (2.f/sqrt(3.f))*0.5;
	return m_R * 2.f * sqrtf(sideEx * sideEx + mini_side * mini_side);
}

unsigned char HexStack::genEye(s_HexStack* neye) {
	s_HexPlate** levels = neye->lev;
	s_Hex* nd = (s_Hex*)levels[0]->nodes[0];
	nd->x = 0.f;
	nd->y = 0.f;
	nd->thislink = 0;
	levels[0]->N = 1;
	float curRs = levels[0]->RShex;
	int numToGen = m_N_levels - 1;
	for (int i = 0; i < numToGen; i++) {
		int indx = 0;
		/*loop over all the hexes in the current level and generate the hexes for the next lower level*/
		for (int i_top = 0; i_top < levels[i]->N; i_top++) {
			nd = (s_Hex*)levels[i]->nodes[i_top];
			int sub_hex_start_indx = indx; /*save the index of the center of the new pattern being assembled*/
			s_2pt loc = { nd->x, nd->y };
			s_2pt_i nebi[HEXEYE_MAXNEBINDXS];
			int num_neb = collectNebIndexes(levels[i], i_top, nebi);
			genLowerPattern(levels[i + 1]->nodes, curRs, loc, nebi, num_neb, indx);/*index advances to final pattern filled but not beyond*/
			indx++;/*get ready to fill the next one*/
			nd->N = 0;
			nd->nodes[0] = (levels[i + 1]->nodes[sub_hex_start_indx]);/*center of newly assembled pattern being linked to top node*/
			nd->N++;
			s_Hex* lo_center_nd = (s_Hex*)(nd->nodes[0]);
			for (int down_i = 0; down_i < 6; down_i++) {
				nd->nodes[down_i + 1] = lo_center_nd->web[down_i];
				nd->N++;
			}
		}
		levels[i + 1]->N = indx;
		curRs /= 2.f;
	}
	return ECODE_OK;
}
int HexStack::collectNebIndexes(s_HexPlate* lev, int i_top, s_2pt_i neb[]) {
	int numi = 0;
	for (int i_webtop = 0; i_webtop < 6; i_webtop++) {
		s_Hex* CenterTopNd = (s_Hex*)lev->nodes[i_top];
		s_Hex* TopNd = (s_Hex*)(CenterTopNd)->web[i_webtop];/* Top node is the current node webed in the direction i_webtop
																		   * to the current center top node */
		if (TopNd != NULL) {
			int rev_webtopi = Math::loop(i_webtop + 3, 6); /*this is the index of the web that will point from the webed node (top Node) to
															* the current center top node */
															/*check if down links for this top hex have been generated*/
			if (TopNd->nodes[0] != NULL) {
				/*if the centerlink has been generated then so to have the web links,
				which means the shared hex is already filled*/
				neb[numi].x0 = i_webtop;//index direction from new top to half overlaped hex below
				neb[numi].x1 = TopNd->nodes[rev_webtopi + 1]->thislink;/*this is the index of the node in the bottom that is shared half in the new top
																	  * TopNd is the node adjoining the center so to go halfway in the direction of
																	  * the center is backwards along the direction from the CenterTopNd to the the TopNd
																	  * or rev_webtopi
																	  * The lower links are filled in the pattern where 0 is the node directly beneth the
																	  * top node and the 6 web directions take up the indexes from 1 to 6
																	  * hence 1 must be added to rev_webtopi to get the correct link */
				numi++;
			}
		}
	}
	return numi;
}
void HexStack::genLowerPattern(s_Node* lev_nds[], float Rs, s_2pt& loc, s_2pt_i neb[], int num_neb, int& indx) {
	/*
	* This function positions a lower node right underneath the upper node and then assembles a pattern of 6 nodes around it
	* generating new nodes if no previous node exists.
	* If a previous node exists, halfway between two upper pattern nodes neb will contain it; such nodes are linked to the new center.
	*
	* The hexes (s_Nodes) for the lower level have already been generated but not positioned or linked
	*
	lev_nds are the lower nodes,
	Rs is the current hex short radius
	loc is the center of this 7 pack pattern
	indx is the index of the current center in its level (the lower level which is generating)
	*/
	s_Hex* lev_node = (s_Hex*)lev_nds[indx]; /*lev_nds are the nodes for the lower level the one being created*/
	lev_node->x = loc.x0;
	lev_node->y = loc.x1;
	lev_node->thislink = indx;
	int cindx = indx;
	for (int i = 0; i < 6; i++) {
		int nebLev_i = getNebLevIndex(neb, num_neb, i); /*i is current web direction from the lower level node around which the pattern is being assembled*/
		int oplink = Math::loop(i + 3, 6);
		if (nebLev_i >= 0) {
			/*this neighbor is alredy generated*/
			s_Hex* neb_lev_node = (s_Hex*)lev_nds[nebLev_i];
			if (neb_lev_node->web[oplink] == NULL)
				neb_lev_node->web[oplink] = (s_Node*)lev_node;
			if (lev_node->web[i] == NULL)
				lev_node->web[i] = (s_Node*)neb_lev_node;
		}
		else {/*only fill if these have not previously been filled*/
			indx++;/*add new hex*/
			s_Hex* rot_lev_node = (s_Hex*)lev_nds[indx];
			rot_lev_node->x = m_hexU[i].x0 * Rs + loc.x0;
			rot_lev_node->y = m_hexU[i].x1 * Rs + loc.x1;
			rot_lev_node->thislink = indx;

			rot_lev_node->web[oplink] = (s_Node*)lev_node;
			lev_node->web[i] = (s_Node*)rot_lev_node;
		}
	}
	weaveRound(lev_node);
	/* shared hexes can have adj cells that need to be woven together */
	for (int i = 0; i < 6; i++) {
		int nebLev_i = getNebLevIndex(neb, num_neb, i);
		if (nebLev_i >= 0) {
			s_Hex* neb_lev_node = (s_Hex*)lev_nds[nebLev_i];
			weaveRound(neb_lev_node);
		}
	}
	return;
}
int HexStack::getNebLevIndex(s_2pt_i neb[], int num_neb, int& web_i) {
	int indxFound = foundInAr(neb, num_neb, web_i);
	if (indxFound < 0)
		return -1;
	return neb[indxFound].x1;
}
void HexStack::weaveRound(s_Hex* nd) {
	for (int i = 0; i < 6; i++) {
		s_Hex* prev_nd = (s_Hex*)nd->web[Math::loop(i - 1, 6)];
		s_Hex* cur_nd = (s_Hex*)nd->web[i];
		s_Hex* next_nd = (s_Hex*)nd->web[Math::loop(i + 1, 6)];
		if (cur_nd != NULL) {
			int prev_indx = Math::loop(4 + i, 6);
			int next_indx = Math::loop(2 + i, 6);
			if (cur_nd->web[prev_indx] == NULL)
				cur_nd->web[prev_indx] = prev_nd;
			if (cur_nd->web[next_indx] == NULL)
				cur_nd->web[next_indx] = next_nd;
		}
	}
	return;
}
int HexStack::foundInAr(s_2pt_i ar[], int n, int val) {
	for (int i = 0; i < n; i++) {
		if (ar[i].x0 == val)
			return i;
	}
	return -1;
}



