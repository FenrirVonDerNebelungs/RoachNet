#include "Hex.h"
void n_Hex::genHexU_0(s_2pt hexU[])
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
s_Hex::s_Hex() :i(-1), j(-1) {
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
unsigned char s_Hex::init(long plate_index, int numHanging) {
	x = -1.f;
	y = -1.f;
	i = -1;
	j = -1;
	o = -1.f;
	thislink = plate_index;
	for (int ii = 0; ii < 6; ii++)
		web[ii] = NULL;
	return s_Node::init(numHanging);
}
unsigned char s_Hex::init(const s_Hex* other) {
	unsigned char err = init(other->thislink, other->N);
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
	s_Node::operator=(other);
	this->i = other.i;
	this->j = other.j;
	for (int ii = 0; ii < 6; ii++)
		this->web[ii] = other.web[ii];
	for (int ii = 0; ii < 3; ii++) {
		this->rgb[ii] = other.rgb[ii];
	}
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
	unsigned char err = s_Hex::init(plate_index);
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
	unsigned char err = s_Hex::init((s_Hex*)other);
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

s_HexPlate::s_HexPlate() :height(0), width(0), Rhex(0.f), RShex(0.f), Shex(0.f) {
	for (int ii = 0; ii < 6; ii++) {
		utilStruct::zero2pt(hexU[ii]);
	}
}
s_HexPlate::~s_HexPlate() {
	;
}
unsigned char s_HexPlate::init(long nNodes, int numHanging) {
	genHexU_0();
	unsigned char err = s_Plate::init(nNodes);
	if (err != ECODE_OK)
		return err;
	for (long ii = 0; ii < N_mem; ii++) {
		nodes[ii] = new s_Hex;
		if (nodes[ii] == NULL)
			return ECODE_FAIL;
		((s_Hex*)nodes[ii])->init(N, numHanging);
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
	for (int ii = 0; ii < N_mem; ii++) {
		if (this->nodes[ii] != NULL) {
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
	if (err != ECODE_OK)
		return err;
	for (int ii = 0; ii < this->N; ii++) {
		s_Hex* this_hex = this->get(ii);
		const s_Hex* other_hex = other->getConst(ii);
		this_hex->nodes[0] = (s_Node*)other_hex;
		for (int i_hanging = 1; i_hanging < this_hex->N; i_hanging++) {
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
	n_Hex::genHexU_0(hexU);
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