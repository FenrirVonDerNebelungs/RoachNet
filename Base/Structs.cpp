#include "Structs.h"

s_Node::s_Node():x(0.f),y(0.f),thislink(-1),nodes(NULL),w(NULL),N(0),o(0.f), N_mem(0)
{
	;
}
s_Node::s_Node(const s_Node& other) {
	if (other.N_mem >= 1) {
		this->nodes = new s_Node * [other.N_mem];
		this->w = new float[other.N_mem];
	}
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
	w = new float[nNodes];
	if (nodes == NULL || w==NULL)
		return ECODE_FAIL;
	N_mem = nNodes;
	for (int ii = 0; ii < N_mem; ii++) {
		nodes[ii] = NULL;
		w[ii] = 0.f;
	}
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
	if(w!=NULL)
		delete[]w;
	w = NULL;
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
			this->w[ii] = other.w[ii];
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
		for (int ii = 0; ii < this->N; ii++) {
			this->nodes[ii] = other->nodes[ii];
			this->w[ii] = other->w[ii];
		}
	}
	this->o = other->o;
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


