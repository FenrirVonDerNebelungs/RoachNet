#include "Structs.h"

s_Node::s_Node():x(0.f),y(0.f),thislink(-1),nodes(NULL),N(0),o(0.f)
{
	;
}
s_Node::s_Node(const s_Node& other) {
	if(other.N_mem>=1)
		this->nodes = new s_Node * [other.N_mem];
	this->N_mem = other.N_mem;
	copy(&other);
}
s_Node::~s_Node() {
	;
}
unsigned char s_Node::init(int nNodes) {
	if (nodes != NULL)
		return ECODE_ABORT;
	if (nNodes < 1) {
		N = 0;
		return ECODE_OK;
	}
	nodes = new s_Node * [nNodes];
	if (nodes == NULL)
		return ECODE_FAIL;
	N_mem = nNodes;
	for (int ii = 0; ii < N_mem; ii++)
		nodes[ii] = NULL;
	N = 0;
	return ECODE_OK;
}
unsigned char s_Node::init(const s_Node* other) {
	unsigned char err = init(other->N_mem);
	if (err != ECODE_OK)
		return err;
	copy(other);
	return ECODE_OK;
}
void s_Node::release() {
	if (nodes != NULL) {
		/*assume pointers usually are not owned*/
		delete[]nodes;
	}
	nodes = NULL;
	N_mem = 0;
	N = 0;
}
unsigned char s_Node::genSubNodes() {
	for (int ii = 0; ii < N_mem; ii++) {
		if (nodes[ii] != NULL)
			return ECODE_ABORT;
	}
	for (int ii = 0; ii < N_mem; ii++) {
		nodes[ii] = new s_Node;
		if (nodes[ii] == NULL)
			return ECODE_FAIL;
	}
	N = N_mem;
	return ECODE_OK;
}
void s_Node::releaseSubNodes() {
	if (nodes == NULL)
		return;
	for (int ii = 0; ii < N; ii++) {
		if (nodes[ii] != NULL) {
			nodes[ii]->release();
			delete nodes[ii];
		}
		nodes[ii] = NULL;
	}
	N = 0;
}
void s_Node::reset() {
	this->x = 0.f;
	this->y = 0.f;
	this->thislink = -1;
	this->o = -1.f;
}
s_Node& s_Node::operator=(const s_Node& other) {
	if (this == &other)
		return *this;
	this->x = other.x;
	this->y = other.y;
	this->thislink = other.thislink;
	this->o = other.o;
	if (this->N_mem == other.N_mem) {
		for (int ii = 0; ii < this->N_mem; ii++) {
			this->nodes[ii] = other.nodes[ii];
		}
		this->N = other.N;
	}
	return *this;
}
void s_Node::copy(const s_Node* other) {
	this->x = other->x;
	this->y = other->y;
	this->thislink = other->thislink;
	if (this->N_mem == other->N_mem) {
		for (int ii = 0; ii < this->N_mem; ii++)
			this->nodes[ii] = NULL;
		this->N = other->N;
		for (int ii = 0; ii < this->N; ii++)
			this->nodes[ii] = other->nodes[ii];
	}
	this->o = other->o;
}
s_Hex::s_Hex():i(-1),j(-1) {
	for (int ii = 0; ii < 6; ii++)
		web[ii] = NULL;
	for (int ii = 0; ii < 3; ii++)
		rgb[ii] = 0.f;
}
s_Hex::s_Hex(const s_Hex& other) {
	s_Node* otherP = (s_Node*)&other;
	s_Node::copy(otherP);
	this->i = other.i;
	this->j = other.j;
	for (int ii = 0; ii < 6; ii++)
		this->web[ii] = other.web[ii];
	for (int ii = 0; ii < 3; ii++)
		this->rgb[ii] = other.rgb[ii];
}
s_Hex::~s_Hex() {
	;
}
unsigned char s_Hex::init(long plate_index) {
	x = -1.f;
	y = -1.f;
	i = -1;
	j = -1;
	o = -1.f;
	thislink = plate_index;
	for (int ii = 0; ii < 6; ii++)
		web[ii] = NULL;
	return s_Node::init(7);
}
unsigned char s_Hex::init(const s_Hex* other) {
	unsigned char err = init(other->thislink);
	if (err != ECODE_OK)
		return err;
	copy(other);
	return ECODE_OK;
}
void s_Hex::release() {
	s_Node::release();
	for (int ii = 0; ii < 6; ii++)
		web[ii] = NULL;
	i = -1;
	j = -1;
	o = -1.f;
}
s_Hex& s_Hex::operator=(const s_Hex& other) {
	if (this == &other)
		return *this;
	this->i = i;
	this->j = j;
	for (int ii = 0; ii < 6; ii++)
		this->web[ii] = other.web[ii];
	for (int ii = 0; ii < 3; ii++) {
		this->rgb[ii] = other.rgb[ii];
	}
	s_Node::operator=(other);
	return *this;
}
void s_Hex::copy(const s_Hex* other) {
	s_Node::copy(other);
	this->i = other->i;
	this->j = other->j;
	for (int ii = 0; ii < 6; ii++)
		this->web[ii] = other->web[ii];
	for (int ii = 0; ii < 3; ii++)
		this->rgb[ii] = other->rgb[ii];
}
unsigned char s_lunHex::init(long plate_index) {
	unsigned char err=s_Hex::init(plate_index);
	if (err != ECODE_OK)
		return err;
	w = new float[this->N_mem];
	for (int ii = 0; ii < this->N_mem; ii++)
		w[ii] = 0.f;
	col_i = -1;
	return ECODE_OK;
}
unsigned char s_lunHex::init(const s_Hex* other) {
	unsigned char err = s_Hex::init((s_Hex*)other);
	if (err != ECODE_OK)
		return err;
	w = new float[this->N_mem];
	for (int ii = 0; ii < this->N_mem; ii++)
		w[ii] = 0.f;
	col_i = -1;
	return ECODE_OK;
}
unsigned char s_lunHex::init(const s_lunHex* other) {
	unsigned char err=s_Hex::init((s_Hex*)other);
	if (err != ECODE_OK)
		return err;
	w = new float[this->N_mem];
	for (int ii = 0; ii < this->N_mem; ii++)
		this->w[ii] = other->w[ii];
	this->col_i = other->col_i;
	return ECODE_OK;
}
void s_lunHex::release() {
	if (w != NULL) {
		delete[] w;
	}
	w = NULL;
	s_Hex::release();
}
s_nNode::s_nNode() :hex(NULL), w(NULL), b(0.f) {
	;
}
s_nNode::~s_nNode() {
	;
}

unsigned char s_nNode::init(int nNodes) {
	if (w != NULL)
		return ECODE_ABORT;
	s_Node::init(nNodes);
	if (N_mem >= 1) {
		w = new float[N_mem];
		if (w == NULL)
			return ECODE_FAIL;
	}
	reset();
	return ECODE_OK;
}
unsigned char s_nNode::init(const s_nNode* other) {
	unsigned char err = init(other->N_mem);
	copy(other);
	return ECODE_OK;
}
void s_nNode::release() {
	if (w != NULL) {
		delete[]w;
	}
	w = NULL;
	s_Node::release();
	reset();
}
void s_nNode::reset() {
	s_Node::reset();
	hex = NULL;
	b = 0.f;
	if(w!=NULL)
		for (int ii = 0; ii < N_mem; ii++)
			w[ii] = 0.f;
}
s_nNode& s_nNode::operator=(const s_nNode& other) {
	this->hex = other.hex;
	if (other.w != NULL) {
		if (this->N_mem == other.N_mem && this->w!=NULL) {
			for (int ii = 0; ii < N_mem; ii++)
				this->w[ii] = other.w[ii];
			this->b = other.b;
		}
	}
	s_Node::operator=(other);
	return *this;
}
void s_nNode::copy(const s_nNode* other) {
	s_Node::copy(other);
	this->hex = other->hex;
	if (other->w != NULL) {
		for (int ii = 0; ii < N_mem; ii++) {
			this->w[ii] = other->w[ii];
		}
	}
	this->b = other->b;
}

s_Plate::s_Plate() :nodes(NULL), N(0), N_mem(0) {
	;
}
s_Plate::~s_Plate() {
	;
}
unsigned char s_Plate::init(long nNodes) {
	if (nodes != NULL)
		return ECODE_ABORT;
	nodes = new s_Node * [nNodes];
	if (nodes == NULL)
		return ECODE_FAIL;
	N_mem = nNodes;
	N = 0;
	for (long ii = 0; ii < N_mem; ii++) {
		nodes[ii] = NULL;
	}
	return ECODE_OK;
}
unsigned char s_Plate::init(const s_Plate* other) {
	unsigned char err = init(other->N_mem);
	if (err != ECODE_OK)
		return err;
	this->N = other->N;
	for (long ii = 0; ii < N; ii++)
		this->nodes[ii] = other->nodes[ii];
	return ECODE_OK;
}
void s_Plate::release() {
	N = 0; 
	if (nodes != NULL) {
		for (long ii = 0; ii < N_mem; ii++) {
			if (nodes[ii] != NULL) {
				nodes[ii]->release();
				delete nodes[ii];
			}
			nodes[ii] = NULL;
		}
		delete[]nodes;
	}
	nodes = NULL;
	N_mem = 0;
}
void s_Plate::reset() {
	N = 0;
}

unsigned char n_Plate::fixStackedPlateLinks(s_Plate* topP, s_Plate* botP) {
	if (topP == NULL || botP == NULL)
		return ECODE_ABORT;
	for (long top_i = 0; top_i < topP->N; top_i++) {
		s_Node* top_node = topP->get(top_i);
		for (int hang_i = 0; hang_i < top_node->N; hang_i++) {
			long bot_i = top_node->nodes[hang_i]->thislink;
			s_Node* bot_node = botP->get(bot_i);
			top_node->nodes[hang_i] = bot_node;
		}
	}
	return ECODE_OK;
}

s_HexPlate::s_HexPlate() :height(0), width(0), Rhex(0.f), RShex(0.f), Shex(0.f) {
	for (int ii = 0; ii < 6; ii++) {
		utilStruct::zero2pt(hexU[ii]);
	}
}
s_HexPlate::~s_HexPlate() {
	;
}
unsigned char s_HexPlate::init(long nNodes) {
	genHexU_0();
	unsigned char err=s_Plate::init(nNodes);
	if (err != ECODE_OK)
		return err;
	for (long ii = 0; ii < N_mem; ii++) {
		nodes[ii] = new s_Hex;
		if (nodes[ii] == NULL)
			return ECODE_FAIL;
		((s_Hex*)nodes[ii])->init(N);
		N++;
	}
	return ECODE_OK;
}
unsigned char s_HexPlate::init(const s_HexPlate* other) {
	if (other == NULL)
		return ECODE_ABORT;
	unsigned char err = s_Plate::init(other->N_mem);
	if (err != ECODE_OK)
		return err;
	this->N = other->N;
	this->height = other->height;
	this->width = other->width;
	this->Rhex = other->Rhex;
	this->RShex = other->RShex;
	this->Shex = other->Shex;
	for (int ii = 0; ii < 6; ii++)
		utilStruct::copy2pt(this->hexU[ii], other->hexU[ii]);
	for (int ii = 0; ii < N_mem; ii++) {
		const s_Hex* other_hex = other->getConst(ii);
		if (other_hex != NULL) {
			this->nodes[ii] = new s_Hex;
			if ((this->nodes[ii]) == NULL)
				return ECODE_FAIL;
			((s_Hex*)this->nodes[ii])->init(other_hex);
		}
	}
	/*the plate is now created but the webs still point to the old plate */
	for(int ii=0; ii<N_mem; ii++){
		if(this->nodes[ii]!=NULL){
			/*now fix the web since the old web will point to the web on the original plate*/
			for (int i_web = 0; i_web < 6; i_web++) {
				s_Hex* this_hex = (s_Hex*)this->nodes[ii];
				s_Hex* lower_web_hex = (s_Hex*)this_hex->web[i_web];
				if (lower_web_hex != NULL) {
					long this_index_in_plate = lower_web_hex->thislink;/*thislink index is correct alghough pointer is wrong*/
					s_Node* this_plate_ptr = this->nodes[this_index_in_plate];
					this_hex->web[i_web] = this_plate_ptr;
				}
				else
					this_hex->web[i_web] = NULL;
			}
		}
	}
	return ECODE_OK;
}
unsigned char s_HexPlate::initFixDownTarget(const s_HexPlate* other) {
	unsigned char err = init(other);
	if (err!=ECODE_OK)
		return err;
	for (int ii = 0; ii < this->N; ii++) {
		s_Hex* this_hex = this->get(ii);
		const s_Hex* other_hex = other->getConst(ii);
		this_hex->nodes[0] = (s_Node*)other_hex;
		for (int i_hanging = 1; i_hanging < this_hex->N; i_hanging++) {/*this will always be 6 since s_Hex always spawns with  7 hanging nodes*/
			this_hex->nodes[i_hanging] = (s_Hex*)other_hex->web[i_hanging - 1];
		}
	}
	return ECODE_OK;
}
void s_HexPlate::initRs(float inRhex) {
	Rhex = inRhex;
	RShex = Rhex * sqrt(3.f) / 2.f;
	Shex = Rhex; //equallat tri
}
void s_HexPlate::release() {
	if (nodes != NULL) {
		for (long ii = 0; ii < N_mem; ii++) {
			if (nodes[ii] != NULL) {
				((s_Hex*)nodes[ii])->release();
				delete nodes[ii];
			}
			nodes[ii] = NULL;
		}
		N = 0;
	}
	reset();
	s_Plate::release();
}
void s_HexPlate::setWeb(long index, int web_i, long target_i) {
	if (target_i >= 0)
		((s_Hex*)nodes[index])->web[web_i] = nodes[target_i];
	else
		((s_Hex*)nodes[index])->web[web_i] = NULL;
}
bool s_HexPlate::inHex(const long hexNode_i, const s_2pt& pt, const float padding) const
{
	s_Hex* h = (s_Hex*)nodes[hexNode_i];
	float xdiff = pt.x0 - h->x;
	float ydiff = pt.x1 - h->y;
	float diff = sqrtf(xdiff * xdiff + ydiff * ydiff);
	if (diff > Rhex)
		return false;
	s_2pt vpt = { xdiff, ydiff };
	float max_proj = 0.f;
	for (int i = 0; i < 6; i++) {
		float proj = vecMath::dot(vpt, hexU[i]);
		if (proj > max_proj)
			max_proj = proj;
	}
	bool inside = false;
	if (max_proj <= (RShex + padding))
		inside = true;
	return inside;
}

void s_HexPlate::reset() {
	height = 0;
	width = 0;
	Rhex = 0.f;
	RShex = 0.f;
	Shex = 0.f;
	for (int ii = 0; ii < 6; ii++)
		utilStruct::zero2pt(hexU[ii]);
	s_Plate::reset();
}
void s_HexPlate::genHexU_0() {
	n_HexPlate::genHexU_0(hexU);
}
void n_HexPlate::genHexU_0(s_2pt hexU[])
{
	float longs = sqrtf(3.f) / 2.f;
	float shorts = 0.5f;
	/*start with to the right*/
	hexU[0].x0 = 1.f;
	hexU[0].x1 = 0.f;

	hexU[1].x0 = shorts;
	hexU[1].x1 = longs;

	hexU[2].x0 = -shorts;
	hexU[2].x1 = longs;

	hexU[3].x0 = -1.f;
	hexU[3].x1 = 0.f;

	hexU[4].x0 = -shorts;
	hexU[4].x1 = -longs;

	hexU[5].x0 = shorts;
	hexU[5].x1 = -longs;
}
int n_HexPlate::rotateCLK(const s_Hex* hexNode, const int start_web_i) {
	int web_i = -1;
	s_Hex* ndPtr = NULL;
	bool found = false;
	/*rotate at end to swap back*/
	for (int i = 0; i < 3; i++) {
		web_i = start_web_i - i;
		if (web_i < 0)
			web_i += 6;
		ndPtr = (s_Hex*)hexNode->web[web_i];
		if (ndPtr != NULL) {
			found = true;
			break;
		}
	}
	return found ? web_i : -1;
}
int n_HexPlate::rotateCCLK(const s_Hex* hexNode, const int start_web_i) {
	int web_i = -1;
	s_Hex* ndPtr = NULL;
	bool found = false;
	/*rotate at end to swap back*/
	for (int i = 0; i < 3; i++) {
		web_i = start_web_i + i;
		/*only values for strt i will be 0 and 3 so don't need a check for >=6*/
		if (web_i >=6)
			web_i -= 6;
		ndPtr = (s_Hex*)hexNode->web[web_i];
		if (ndPtr != NULL) {
			found = true;
			break;
		}
	}
	return found ? web_i : -1;
}
bool n_HexPlate::indexChainRoot(s_HexPlate* root, s_HexPlate* base, s_Hex* root_first_node, s_Hex* base_first_node, long hex_start_i, long hex_end_i) {
	s_Hex* lo_hex = base_first_node;
	s_Hex* hi_hex = root_first_node;
	int web_start_i = 0;
	for (long hex_i = (hex_start_i+1); hex_i < hex_end_i; hex_i++) {
		hi_hex->nodes[0] = lo_hex;
		s_Hex* next_hi_hex = root->get(hex_i);
		int web_to_next_nd = getWebDir(hi_hex, next_hi_hex, web_start_i);
		if (web_to_next_nd < 0)
			return false;
		s_Hex* next_low_hex = (s_Hex*)lo_hex->web[web_to_next_nd];
		lo_hex = next_low_hex;
		hi_hex = next_hi_hex;
	}
	return true;
}
int n_HexPlate::getWebDir(s_Hex* start_nd, s_Hex* end_nd, int& web_start_i) {
	int web_dir_i = -1;
	for (int web_cnt = 0; web_cnt < 6; web_cnt++) {
		if ((start_nd->web[web_start_i]) == end_nd) {
			web_dir_i = web_start_i;
			break;
		}
		int inc_web_start_i = web_start_i + 1;
		web_start_i = Math::loop(inc_web_start_i, 6);
	}
	return web_dir_i;
}
s_Hex* n_HexPlate::connLineStackedPlates(s_Hex* nd_hi, s_Hex* nd_lo, int next_web_i) {
	int hex_i = -1;
	s_Hex* hi_hex = NULL;
	s_Hex* lo_hex = NULL;
	s_Hex* next_hi = nd_hi;
	s_Hex* next_lo = nd_lo;
	do {
		hi_hex = next_hi;
		lo_hex = next_lo;
		hi_hex->nodes[0] = lo_hex;
		hi_hex->N = 1;
		/*advance*/
		next_hi = (s_Hex*)hi_hex->web[next_web_i];
		next_lo = (s_Hex*)lo_hex->web[next_web_i];
	} while (next_hi != NULL && next_lo != NULL);
	if (next_hi != NULL && next_lo == NULL)
		return NULL;
	return hi_hex;
}
int n_HexPlate::turnCornerStackedPlates(s_Hex** nd_hi, s_Hex** nd_lo, int next_web_i, int fwd_web_i, int rev_web_i) {
	/*assumes nd_hi is already connected to its low node*/
	*nd_lo = (s_Hex*)(*nd_hi)->nodes[0];
	if (next_web_i == fwd_web_i) {
		next_web_i = rotateCLK(*nd_hi, fwd_web_i);
		if (next_web_i >= 0) {
			*nd_hi = (s_Hex*)(*nd_hi)->web[next_web_i];
			*nd_lo = (s_Hex*)(*nd_lo)->web[next_web_i];
			next_web_i = ((*nd_lo) != NULL) ? rev_web_i : -2;
		}
	}else if (next_web_i == rev_web_i) {
		next_web_i = rotateCCLK(*nd_hi, rev_web_i);
		if (next_web_i >= 0) {
			*nd_hi = (s_Hex*)(*nd_hi)->web[next_web_i];
			*nd_lo = (s_Hex*)(*nd_lo)->web[next_web_i];
			next_web_i = ((*nd_lo) != NULL) ? fwd_web_i : -2;
		}
	}
	return next_web_i;
}
long n_HexPlate::countNumHexesInLine(long start_i, int dir_web_i, s_HexPlate* o) {
	if (start_i < 0 || start_i >= o->N || dir_web_i<0 || dir_web_i>=6)
		return 0;
	long cur_i = start_i;
	long num_hexes = 1;
	s_Hex* cur_hex = o->get(cur_i);
	for (long ii = start_i; ii < o->N; ii++) {
		s_Node* next_over_hex = cur_hex->web[dir_web_i];
		if (next_over_hex == NULL)
			break;
		cur_hex = (s_Hex*)next_over_hex;
		num_hexes++;
	}
	return num_hexes;
}
unsigned char n_HexPlate::pool2init(s_HexPlate* o, s_HexPlate* pool) {
	if (o == NULL || pool == NULL)
		return ECODE_FAIL;
	if (o->N < 7)
		return ECODE_ABORT;
	pool->height = o->height;
	pool->width = o->width;
	pool->Rhex = 2.f * o->Rhex;
	pool->RShex = 2.f * o->RShex;
	pool->Shex = 2.f * o->Shex;
	for (int ii = 0; ii < 6; ii++)
		utilStruct::copy2pt(pool->hexU[ii], o->hexU[ii]);
	s_Hex** large_hexes = new s_Hex * [o->N];
	long num_large_hexes = 0;
	/*count the number of large hexes and load all the good large hex centers into an array, 
	this is not particularlly efficient*/
	/*find start hex one down*/
	s_Hex* start_hex = o->get(0L);
	s_Hex* next_down_hex = NULL;
	const int web_left_i = 3;
	const int web_right_i = 0;
	do {
		next_down_hex = NULL;
		int next_down_hex_web_i = rotateCCLK(start_hex, web_left_i);
		if (next_down_hex_web_i >= 0)
			next_down_hex = (s_Hex*)start_hex->web[next_down_hex_web_i];
		if (next_down_hex != NULL) {
			s_Hex* large_hex = next_down_hex;
			do {
				/*check if the hex is fully linked around, in which case it will be one of the pool hexes*/
				bool is_large_candidate = true;
				for (int ii = 0; ii < 6; ii++)
					if (large_hex->web[ii] ==NULL)
						is_large_candidate = false;
				if (is_large_candidate) {
					large_hexes[num_large_hexes] = large_hex;
					num_large_hexes++;
					/*find next next over*/
					s_Hex* one_over_hex = (s_Hex*)large_hex->web[web_right_i];
					if (one_over_hex != NULL)
						/*and over again*/
						large_hex = (s_Hex*)one_over_hex->web[web_right_i];
				}
				else {
					/*try moving one over*/
					s_Hex* prev_large_hex = large_hex;
					large_hex = (s_Hex*)prev_large_hex->web[web_right_i];
				}
			} while (large_hex != NULL);
		}
		/*need to drop two lines*/
		start_hex = next_down_hex;
		next_down_hex = NULL;
		next_down_hex_web_i = rotateCCLK(start_hex, web_left_i);
		if (next_down_hex_web_i >= 0)
			next_down_hex = (s_Hex*)start_hex->web[next_down_hex_web_i];

		start_hex = next_down_hex;
	} while (start_hex != NULL);
	
	/*next setup the nodes*******************************/
	if (num_large_hexes < 1)
		return ECODE_ABORT;
	pool->init(num_large_hexes);/*this will create unfilled s_Hex nodes in the pool plate*/
	for (long ii = 0; ii < num_large_hexes; ii++) {
		/*send the values from the center of the large on the lower plate into the large on the higher plate*/
		s_Hex* small_hex = large_hexes[ii]; 
		s_Hex* large_hex = pool->get(ii);
		/*start with s_node values*/
		large_hex->x = small_hex->x;
		large_hex->y = small_hex->y;
		/*this link is the index in the plane which is different for the large and small because the large has fewer nodes*/
		/*there are 7 s_nodes that will be linked downward to the 7 s_Hex nodes on the original un-pooled plate
		  the convention followed by the lunas seems to be that 0 is the center node then 1 to 6 are the web nodes 0 to 5*/
		large_hex->nodes[0] = (s_Node*)small_hex;
		for (int i_web = 0; i_web < 6; i_web++) {
			s_Node* web_node = small_hex->web[i_web];
			large_hex->nodes[1 + i_web] = web_node;
		}
		large_hex->i = small_hex->i;
		large_hex->j = small_hex->j;
	}
	delete []large_hexes;
	/*link the nodes************************************/
	/* not very efficient */
	for (long ii = 0; ii < num_large_hexes; ii++) {
		s_Hex* large_hex = pool->get(ii);
		/*rotate around the large hex linking the other large hexes to it in the web*/
		for (int i_web = 0; i_web < 6; i_web++) {
			/*lower overlapping node*/
			s_Hex* small_web_comb_node = (s_Hex*)large_hex->nodes[1 + i_web];
			s_Node* center_lower_link_node = NULL;
			if (small_web_comb_node != NULL)
				center_lower_link_node = small_web_comb_node->web[i_web];
			/*check if there is another node one over from the current large node*/
			if (center_lower_link_node != NULL) {
				/*if there is find the index on the lower plate of the node one over
				  this node needs its upper large node linked to to the large node at this web index*/
				long center_index = center_lower_link_node->thislink;
				/*this is not efficient find the large hex */
				for (int jj = 0; jj < num_large_hexes; jj++) {
					s_Hex* match_candidate_node = (s_Hex*)pool->get(jj);
					/*find the lower link node under this large hex*/
					s_Node* lower_match_candidate_node = match_candidate_node->nodes[0];
					if (lower_match_candidate_node == NULL)
						continue;
					/*find the index of the bottom node that is under the center of the large node that is a candidate for web link*/
					long match_candidate_index = lower_match_candidate_node->thislink;
					if (center_index == match_candidate_index) {
						large_hex->web[i_web] = (s_Node*)match_candidate_node;
						break;
					}
				}
			}
		}
	}
	return ECODE_OK;
}

s_HexBasePlate::s_HexBasePlate() :N_wHex(0), N_hHex(0), RowStart(NULL), RowStart_is(NULL), Row_N(0), Col_d(0.f), Row_d(0.f)
{
	;
}
s_HexBasePlate::~s_HexBasePlate() {
	;
}
unsigned char s_HexBasePlate::init(const s_HexBasePlate* other) {
	unsigned char err = s_HexPlate::init((s_HexPlate*)other);
	if (err != ECODE_OK)
		return err;
	this->N_wHex = other->N_wHex;
	this->N_hHex = other->N_hHex;
	if (other->RowStart != NULL && other->RowStart_is != NULL && other->Row_N >= 1) {
		err = initRowStart(other->Row_N);
		if (err != ECODE_OK)
			return err;
		for (long ii = 0; ii < this->Row_N; ii++) {
			utilStruct::copy2pt(this->RowStart[ii], other->RowStart[ii]);
			utilStruct::copy2pt_i(this->RowStart_is[ii], other->RowStart_is[ii]);
		}
		this->Col_d = other->Col_d;
		this->Row_d = other->Row_d;
	}
	return ECODE_OK;
}
unsigned char s_HexBasePlate::initRowStart(long rowN) {
	if (RowStart != NULL || RowStart_is != NULL)
		return ECODE_ABORT;
	RowStart = new s_2pt[rowN];
	if (RowStart == NULL)
		return ECODE_FAIL;
	RowStart_is = new s_2pt_i[rowN];
	if (RowStart_is == NULL)
		return ECODE_FAIL;
	Row_N = rowN;
	for (long ii = 0; ii < rowN; ii++) {
		utilStruct::zero2pt(RowStart[ii]);
		utilStruct::zero2pt_i(RowStart_is[ii]);
	}
	return ECODE_OK;
}
void s_HexBasePlate::releaseRowStart() {
	if (RowStart_is != NULL) {
		delete[] RowStart_is;
	}
	RowStart_is = NULL;
	if (RowStart != NULL) {
		delete[] RowStart;
	}
	RowStart = NULL;
	Row_N = 0;
}
void s_HexBasePlate::reset() {
	Col_d = 0.f;
	Row_d = 0.f;
}
unsigned char n_HexBasePlate::initHexBasePlate_from_HexPlate(s_HexBasePlate* p) {
	if(p == NULL)
		return ECODE_ABORT;
	if (p->N < 1)
		return ECODE_ABORT;
	p->Col_d = 2.f / 3.f * p->Rhex;
	p->Row_d = 2.f * p->RShex;
	s_Node** row_start_buffer = new s_Node * [p->N];
	long row_count = 0;
	long row_start_i = 0;
	p->N_wHex = 0;
	do{
		s_Hex* row_start_node = (s_Hex*)p->nodes[row_start_i];
		s_Hex* web_next = NULL;
		long cur_row_count = 0;
		do {
			web_next = (s_Hex*)row_start_node->web[0];
			cur_row_count++;
		} while (web_next != NULL);
		if (cur_row_count > p->N_wHex)
			p->N_wHex = cur_row_count;
		row_start_buffer[row_count] = row_start_node;
		row_count++;
		/*try to go down a row*/
		row_start_i = -1;
		if (row_start_node->web[4] != NULL)
			row_start_i = row_start_node->web[4]->thislink;
		else if (row_start_node->web[5] != NULL)
			row_start_i = row_start_node->web[5]->thislink;
	} while (row_start_i>0);
	p->N_hHex = row_count;
	p->initRowStart(row_count);/*this inits the mem*/
	/*now fill the row starts*/
	for (long ii = 0; ii < p->Row_N; ii++) {
		s_Hex* row_start_node = (s_Hex*)row_start_buffer[ii];
		long i_node = row_start_node->i;
		long j_node = row_start_node->j;
		float x_node = row_start_node->x;
		float y_node = row_start_node->y;
		p->RowStart[ii].x0 = x_node;
		p->RowStart[ii].x1 = y_node;
		p->RowStart_is[ii].x0 = i_node;
		p->RowStart_is[ii].x1 = j_node;
	}
	delete[] row_start_buffer;
	return ECODE_OK;
}
unsigned char n_HexBasePlate::pool2init(s_HexBasePlate* o, s_HexBasePlate* pool) {
	unsigned char errc = n_HexPlate::pool2init((s_HexPlate*)o, (s_HexPlate*)pool);
	if (errc != ECODE_OK)
		return errc;
	return initHexBasePlate_from_HexPlate(pool);
}
unsigned char s_nPlate::init(long nNodes, int nLowerNodes) {
	if ((s_Plate::init(nNodes))!=ECODE_OK) return ECODE_FAIL;
	for (long ii = 0; ii < N_mem; ii++) {
		if (nodes[ii] != NULL)
			return ECODE_FAIL;
		nodes[ii] = new s_nNode;
		if (nodes[ii] == NULL)
			return ECODE_FAIL;
		((s_nNode*)nodes[ii])->init(nLowerNodes);
		N++;
	}
	num_hanging = nLowerNodes;
	return ECODE_OK;
}
unsigned char s_nPlate::init(const s_nPlate* other) {
	unsigned char err = s_Plate::init(other->N_mem);
	if (err != ECODE_OK)
		return err;
	this->N = other->N;
	for (long ii = 0; ii < N_mem; ii++) {
		/*nodes[ii] should always be null*/
		if (this->nodes[ii] != NULL)
			return ECODE_FAIL;
		this->nodes[ii] = new s_nNode;
		if (this->nodes[ii] == NULL)
			return ECODE_FAIL;
		((s_nNode*)this->nodes[ii])->init(other->getConst(ii));
	}
	this->num_hanging = other->num_hanging;
	return ECODE_OK;
}
unsigned char s_nPlate::init(s_HexPlate* hex_plate, int nLowerNodes) {
	long N_nodes = hex_plate->N;
	if (N_nodes < 1)
		return ECODE_ABORT;
	unsigned char err = init(N_nodes, nLowerNodes);
	if (err != ECODE_OK) return err;
	err = setHexes(hex_plate);
	return err;
}
unsigned char s_nPlate::setHexes(s_HexPlate* hex_plate) {
	if (N != hex_plate->N)
		return ECODE_ABORT;
	for (long ii = 0; ii < N; ii++) {
		((s_nNode*)nodes[ii])->hex = (s_Hex*)hex_plate->nodes[ii];
	}
	return ECODE_OK;
}
s_HexPlateLayer::s_HexPlateLayer() :p(NULL), N(0), N_mem(0) { ; }
s_HexPlateLayer::~s_HexPlateLayer() { ; }
unsigned char s_HexPlateLayer::init(int Nplates) {
	p = new s_HexPlate * [Nplates];
	if (p == NULL)
		return ECODE_FAIL;
	for (int ii = 0; ii < Nplates; ii++)
		p[ii] = NULL;
	N_mem = Nplates;
	N = 0;
	return ECODE_OK;
}
unsigned char s_HexPlateLayer::init(const s_HexPlateLayer* pl) {
	if (pl->p == NULL)
		return ECODE_ABORT;
	int Nplates = pl->N;
	unsigned char errc = init(Nplates);
	if (errc != ECODE_OK)
		return errc;
	for (int ii = 0; ii < Nplates; ii++) {
		if (pl->p[ii] == NULL)
			return ECODE_ABORT;
		errc = this->p[ii]->init(pl->p[ii]);
		if (errc != ECODE_OK)
			return ECODE_FAIL;
	}
	return ECODE_OK;
}
void s_HexPlateLayer::release() {
	if (p != NULL) {
		delete[] p;
	}
	p = NULL;
	N_mem = 0;
	N = 0;
}