#include "HexEye.h"
s_HexEye::s_HexEye() :lev(NULL), N(0), width(0L), height(0L), w_col_center(0.f), w_col_aux(0.f), N_mem(0) {
	;
}
s_HexEye::~s_HexEye() {
	;
}
unsigned char s_HexEye::init(int NumLev) {
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
unsigned char s_HexEye::init(const s_HexEye& other) {
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
void s_HexEye::release() {
	if (lev != NULL) {
		delete[] lev;
	}
	lev = NULL;
	N = 0;
	N_mem = 0;
	w_col_center = 0.f;
	w_col_aux = 0.f;
}

unsigned char n_HexEye::imgRoot(s_HexEye* eye, s_HexPlate* pImg, long center_i) {
	/*assume that center_i is inside of hex map*/
	s_HexPlate* bottom_lev = eye->getBottom();

	int next_web_i = 0;
	s_Hex* eye_nd = bottom_lev->get(0);/* zero is the center of the plate node */
	s_Hex* plt_nd = pImg->get(center_i);
	bool fullRoot = true;
	do {
		eye_nd = n_HexPlate::connLineStackedPlates(eye_nd, plt_nd, next_web_i);
		if (eye_nd != NULL)
			next_web_i = n_HexPlate::turnCornerStackedPlates(&eye_nd, &plt_nd, next_web_i, 0, 3);/*this resets the eye_nd pointer and plt_nd pointer to the next line*/
		else {
			next_web_i = -2;
			break;
		}
	} while (next_web_i >= 0);
	if (next_web_i < -1)
		fullRoot = false;

	next_web_i = 3;
	eye_nd = bottom_lev->get(0);
	plt_nd = pImg->get(center_i);
	do {
		eye_nd = n_HexPlate::connLineStackedPlates(eye_nd, plt_nd, next_web_i);
		if (eye_nd != NULL)
			next_web_i = n_HexPlate::turnCornerStackedPlates(&eye_nd, &plt_nd, next_web_i, 3, 0);
		else {
			next_web_i = -2;
			break;
		}
	} while (next_web_i >= 0);

	if (next_web_i < -1 || !fullRoot)
		return ECODE_ABORT;
	return ECODE_OK;
}

bool n_HexEye::check_imgRoot(s_HexEye* eye, s_HexPlate* pImg) {
	if (eye == NULL || pImg==NULL)
		return false;
	if (eye->N < 1)
		return false;
	if (pImg->N < 1)
		return false;
	s_HexPlate* bottom_lev = eye->getBottom();
	float eye_RDiff = HEXEYE_RDIFFTOL * bottom_lev->Rhex;
	float eye_Rmax = eye_RDiff + bottom_lev->Rhex;
	float eye_Rmin = bottom_lev->Rhex - eye_RDiff;
	if (pImg->Rhex > eye_Rmax || pImg->Rhex < eye_Rmin)
		return false;
	return true;
}
unsigned char n_HexEye::imgRootL2(s_HexEye* eye, s_HexPlate* pImg, long center_i) {
	/*assume that center_i is inside of hex map*/
	s_HexPlate* bottom_lev = eye->getBottom();

	s_Hex* eye_nd = bottom_lev->get(0);/* zero is the center of the plate node */
	s_Hex* plt_nd = pImg->get(center_i);
	bool fullRoot = true;
	/*connect the middle node of the eye to the plate node*/
	eye_nd->nodes[0] = (s_Node*)plt_nd;
	/*there should be 7 lower nodes, and the middle lower node should be connected by the web to the rest*/
	/*go around the middle node*/
	for (int i = 0; i < 6; i++) {
		s_Node* eye_web_nd = eye_nd->web[i];
		s_Node* plt_web_nd = plt_nd->web[i];
		eye_web_nd->nodes[0] = plt_web_nd;
		if (plt_web_nd == NULL) fullRoot = false;
	}
	if (!fullRoot) return ECODE_ABORT;
	return ECODE_OK;
}
bool n_HexEye::check_imgRootL2(s_HexEye* eye, s_HexPlate* pImg) {
	if (!check_imgRoot(eye, pImg))
		return false;
	if (eye->N != 2)
		return false;
	return true;
}

unsigned char n_HexEye::updateCol(s_HexEye* eye) {
	int max_lev_i = eye->N-2;
	if (max_lev_i < 0)
		return ECODE_ABORT;
	for (int i_lev = max_lev_i; i_lev >= 0; i_lev--) {
		if( (runEyeLevel(eye, i_lev))!=ECODE_OK )
			return ECODE_FAIL;
	}
	return ECODE_OK;
}

unsigned char n_HexEye::rootUnderEyeHex(s_HexEye* eye, const s_HexEye* overEye, const int over_lev_i, const int over_hex_i) {
	s_Hex* overHex = overEye->lev[over_lev_i]->get(over_hex_i);
	int trace_lev_N = eye->N - 1;
	long* down_i_trace = new long[trace_lev_N];
	for (int i_tr = 0; i_tr < trace_lev_N; i_tr++) {
		down_i_trace[i_tr] = 0L;
	}
	/*zero the bottom of the eye that is being set*/
	for (long i_bot = 0L; i_bot < eye->getBottom()->N; i_bot++) {
		eye->getBottom()->get(i_bot)->nodes[0] = NULL;
	}
	/*fill bottom of trace*/
	s_Hex* tr_hex = eye->get(0)->get(0L);
	s_Hex* over_tr_hex = overHex;
	bool continue_trace = true;
	do {
		continue_trace = recursiveTraceEyeHexDown(tr_hex, over_tr_hex, trace_lev_N, down_i_trace);
	} while (continue_trace);
	return ECODE_OK;
}
bool n_HexEye::recursiveTraceEyeHexDown(s_Hex* tr_hex, s_Hex* over_tr_hex, int trace_level_N, long down_i_trace[]){
	int i_lev = 0;
	for (i_lev = 0; i_lev < trace_level_N; i_lev++) {/*chase the hexes down the trace*/
		s_Hex* tr_down_hex = (s_Hex*)tr_hex->nodes[down_i_trace[i_lev]];
		s_Hex* over_tr_down_hex = (s_Hex*)over_tr_hex->nodes[down_i_trace[i_lev]];
		tr_hex = tr_down_hex;
		over_tr_hex = over_tr_down_hex;
		if (over_tr_hex == NULL)/*the over hex didn't have enough levels or extended off to where it wasn't connected*/
			return false;
	}
	tr_hex->nodes[0] = (s_Node*)over_tr_hex;/*set the hex at the base of the trace*/
	/*move the trace over to the next hex*/
	i_lev = trace_level_N - 1;
	down_i_trace[i_lev] += 1;
	while (i_lev >= 1 && down_i_trace[i_lev] >= 7) {
		down_i_trace[i_lev] = 0;
		i_lev -= 1;
		down_i_trace[i_lev] += 1;
	}
	bool continue_trace = true;
	if (i_lev == 0 && down_i_trace[i_lev]>=7)/*if i_lev leaves the above loop as 0 it means the loop was trying to move over for a trace that extends above
		             the highest hex. This means the loop has filled the traces at the base for all the hexes and is now done*/
		continue_trace = false;
	return continue_trace;
}
unsigned char n_HexEye::runEyeLevel(s_HexEye* seye, int i_level) {
	s_highConvKernVars IOVars;
	IOVars.w_center = seye->w_col_center;
	IOVars.w_aux = seye->w_col_aux;
	IOVars.hex_index = 0L;
	IOVars.num_Hex = seye->get(i_level)->N;
	IOVars.Hexes = seye->get(i_level)->getNodes();
	return runEyeLevelSingleThread(IOVars);
}
unsigned char n_HexEye::runEyeLevelSingleThread(s_highConvKernVars IOVars) {
	for (long i_hex = 0; i_hex < IOVars.num_Hex; i_hex++) {
		convHexKernel(IOVars);
		IOVars.hex_index++;
	}
	return ECODE_OK;
}
void n_HexEye::convHexKernel(s_highConvKernVars IOVars) {
	s_Hex* topHex = (s_Hex*)IOVars.Hexes[IOVars.hex_index];
	float rgb[3];
	for (int i_rgb = 0; i_rgb < 3; i_rgb++)
		rgb[i_rgb] = IOVars.w_center * ((s_Hex*)topHex->nodes[0])->rgb[i_rgb];
	for (int i_low = 1; i_low < topHex->N; i_low++) {
		for (int i_rgb = 0; i_rgb < 3; i_rgb++)
			rgb[i_rgb] = rgb[i_rgb] + IOVars.w_aux * ((s_Hex*)topHex->nodes[i_low])->rgb[i_rgb];
	}
	for (int i_rgb = 0; i_rgb < 3; i_rgb++)
		topHex->rgb[i_rgb] = rgb[i_rgb];
	return;
}
HexEye::HexEye() :m_r(0.f), m_R(0.f), m_N_levels(-1), m_N_hexes(NULL), m_baseWidth(0L), m_baseHeight(0L) {
	for(int i=0; i<6; i++)
		utilStruct::zero2pt(m_hexU[i]);
}
HexEye::~HexEye() {
	;
}
unsigned char HexEye::init(float r, Img* im) {
	long smallest_dim = (im->getWidth() <= im->getHeight()) ? im->getWidth() : im->getHeight();
	int N_levels = largestN_level(smallest_dim);
	if (N_levels < 0)
		return ECODE_ABORT;
	return init(r, N_levels);
}
unsigned char HexEye::init(float r, int NLevels) {
	if (r < 1.f) return ECODE_ABORT;
	if (NLevels < 1) return ECODE_ABORT;
	m_r = r;
	m_R = r * Math::power(2.f, (NLevels - 1)); /*figure out what largest R is r*2^(N_level-1) */
	m_N_levels = NLevels;
	n_HexPlate::genHexU_0(m_hexU);
	m_N_hexes = new long[m_N_levels];
	if (Err(genNumHexesPerLevel()))
		return ECODE_FAIL;
	if (Err(spawn(&m_refEye)))
		return ECODE_FAIL;
	m_baseWidth = m_refEye.getBottom()->width;
	m_baseHeight = m_refEye.getBottom()->height;
	return ECODE_OK;
}
void HexEye::release() {
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
unsigned char HexEye::spawn(s_HexEye* neye) {
	if (neye == NULL)
		return ECODE_ABORT;
	unsigned char err = initEye(neye);
	if (Err(err))
		return err;
	return genEye(neye);
}
void HexEye::despawn(s_HexEye* neye) {
	releaseEye(neye);
}
unsigned char HexEye::genNumHexesPerLevel() {
	long N_hex = 0;
	for (int i = 0; i < m_N_levels; i++) {
		long N_hex_thisLevel = numHexInLevel(N_hex, i);
		m_N_hexes[i] = N_hex_thisLevel;
		N_hex = N_hex_thisLevel;
	}
	return ECODE_OK;
}

unsigned char HexEye::initEye(s_HexEye* neye) {
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
void HexEye::releaseEye(s_HexEye* neye) {
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
long HexEye::numHexInLevel(long N_prev, int N_index) {
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
float HexEye::sizeOfLevel(int N_level) {
	/* 1+x+x^2 +.. = x^(n+1)-1*/
	float sideEx = Math::powerXseries(0.5f, N_level);/* 1+(1/2)+(1/2)^2+..(1/2)^N_level */
	float mini_side = Math::power(0.5f,N_level);
	sideEx *= m_R * sqrt(3.f) / 2.f;
	mini_side *= m_R;
	return 2.f*sqrtf(sideEx * sideEx + mini_side * mini_side);
}
int HexEye::largestN_level(long img_size) {
	int level_N = -1;
	for (int lev_N = 0; lev_N <= HEXEYE_MAXIMGFITLEVEL; lev_N++) {
		int lev_size = (int)ceilf(sizeOfLevel(lev_N));
		if (lev_size < img_size)
			level_N = lev_N;
		else
			break;
	}
	return level_N;
}
unsigned char HexEye::genEye(s_HexEye* neye) {
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
				nd->nodes[down_i+1] = lo_center_nd->web[down_i];
				nd->N++;
			}
		}
		levels[i + 1]->N = indx;
		curRs /= 2.f;
	}
	return ECODE_OK;
}
int HexEye::collectNebIndexes(s_HexPlate* lev, int i_top, s_2pt_i neb[]) {
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
				neb[numi].x1 = TopNd->nodes[rev_webtopi+1]->thislink;/*this is the index of the node in the bottom that is shared half in the new top 
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
void HexEye::genLowerPattern(s_Node* lev_nds[], float Rs, s_2pt& loc, s_2pt_i neb[], int num_neb, int& indx) {
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
			if(neb_lev_node->web[oplink]==NULL)
				neb_lev_node->web[oplink] = (s_Node*)lev_node;
			if(lev_node->web[i]==NULL)
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
int HexEye::getNebLevIndex(s_2pt_i neb[], int num_neb, int& web_i) {
	int indxFound = foundInAr(neb, num_neb, web_i);
	if (indxFound < 0)
		return -1;
	return neb[indxFound].x1;
}
void HexEye::weaveRound(s_Hex* nd) {
	for (int i = 0; i < 6; i++) {
		s_Hex* prev_nd = (s_Hex*)nd->web[Math::loop(i - 1, 6)];
		s_Hex* cur_nd = (s_Hex*)nd->web[i];
		s_Hex* next_nd = (s_Hex*)nd->web[Math::loop(i + 1, 6)];
		if (cur_nd != NULL) {
			int prev_indx = Math::loop(4 + i, 6);
			int next_indx = Math::loop(2 + i, 6);
			if(cur_nd->web[prev_indx]==NULL)
				cur_nd->web[prev_indx] = prev_nd;
			if(cur_nd->web[next_indx]==NULL)
				cur_nd->web[next_indx] = next_nd;
		}
	}
	return;
}
int HexEye::foundInAr(s_2pt_i ar[], int n, int val) {
	for (int i = 0; i < n; i++) {
		if (ar[i].x0 == val)
			return i;
	}
	return -1;
}