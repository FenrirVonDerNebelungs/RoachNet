#include "CNets.h"
s_CNets::s_CNets() :net(NULL), eye(NULL), N(0), trigger_node(NULL), N_mem(0) {
	;
}
s_CNets::~s_CNets() {
	;
}
unsigned char s_CNets::init(int nNets) {
	N = 0;
	N_mem = 0;
	if (nNets < 1)
		return ECODE_OK;
	net = new s_Net * [nNets];
	if (net == NULL)
		return ECODE_FAIL;
	N_mem = nNets;
	for (int ii = 0; ii < N_mem; ii++)
		net[ii] = NULL;
	return ECODE_OK;
}
unsigned char s_CNets::init(const s_CNets& other) {
	unsigned char err = init(other.N_mem);
	if (err != ECODE_OK)
		return err;
	/*generally  assume that eye is NOT owned*/
	this->eye = other.eye;
	if (other.net != NULL) {
		for (int ii = 0; ii < N_mem; ii++) {
			if (other.net[ii] != NULL) {
				this->net[ii] = new s_Net;
				if (this->net[ii] == NULL)
					return ECODE_FAIL;
				err = this->net[ii]->init(*other.net[ii]);
				if (err == ECODE_FAIL)
					return err;
				this->N++;
			}
		}
	}
	if (other.trigger_node != NULL) {
		this->trigger_node = new s_nNode;
		if (this->trigger_node == NULL)
			return ECODE_FAIL;
		err = this->trigger_node->init(other.trigger_node);
		if (err != ECODE_OK)
			return err;
	}
	else
		this->trigger_node = NULL;
	return ECODE_OK;
}
unsigned char s_CNets::newNets() {
	if (net == NULL)
		return ECODE_FAIL;
	for (int ii = 0; ii < N_mem; ii++) {
		if (net[ii] != NULL)
			return ECODE_FAIL;
		net[ii] = new s_Net;
		N++;
	}
	return ECODE_OK;
}
void s_CNets::delNets() {
	if (net != NULL) {
		for (int ii = 0; ii < N; ii++) {
			if (net[ii] != NULL)
				delete net[ii];
			net[ii] = NULL;
		}
	}
}
void s_CNets::release() {
	if (trigger_node != NULL) {
		trigger_node->release();
		delete trigger_node;
	}
	trigger_node = NULL;
	N = 0;
	if (net != NULL) {
		for (int ii = 0; ii < N_mem; ii++) {
			if (net[ii] != NULL) {
				net[ii]->release();
				delete net[ii];
			}
			net[ii] = NULL;
		}
		delete[] net;
	}
	net = NULL;
	N_mem = 0;
}
bool n_CNets::runNNet(s_CNets* nets, s_HexPlate* plate, long plate_index) {
	if (!rootOnPlate(nets, plate, plate_index))
		return false;
	for (int i_net = 0; i_net < nets->N; i_net++) {
		n_Net::runRootedNNet(nets->net[i_net]);
	}
	return true;
}
bool n_CNets::runNNet(s_CNets* nets, s_HexPlateLayer* plates, long plate_index) {
	if (!rootOnPlates(nets, plates, plate_index))
		return false;
	for (int i_net = 0; i_net < nets->N; i_net++) {
		n_Net::runRootedNNet(nets->net[i_net]);
	}
	return true;
}
bool n_CNets::rootOnPlate(s_CNets* nets, s_HexPlate* plate, long plate_index) {
	if (!rootEye(nets, plate, plate_index))
		return false;
	rootNetsOnPlate(nets, plate);
	return true;
}
bool n_CNets::rootOnPlates(s_CNets* nets, s_HexPlateLayer* plates, long plate_index) {
	s_HexPlate* first_plate = plates->get(0);
	if (!rootEye(nets, first_plate, plate_index))
		return false;
	rootNetsOnPlates(nets, plates);
	return true;
}
bool n_CNets::rootEye(s_CNets* nets, s_HexPlate* basePlate, long plate_index) {
	/*in interest of speed assumes the eye has been setup correctly as non-null*/
	unsigned char err = n_HexEye::imgRoot(nets->eye, basePlate, plate_index);
	if (err != ECODE_OK)
		return false;
	return true;
}

void n_CNets::rootNetsOnPlate(s_CNets* nets, s_HexPlate* plate) {
	for (int i_net = 0; i_net < nets->N; i_net++) {
		n_Net::rootNNet(nets->net[i_net], plate);
	}
}

void n_CNets::rootNetsOnPlates(s_CNets* nets, s_HexPlateLayer* plates) {
	for (int i_net = 0; i_net < nets->N; i_net++) {
		n_Net::rootNNet(nets->net[i_net], plates);
	}
}
CNets::CNets() :m_genHexEye(NULL), m_genNet(NULL), m_nNets(0) {
	;
}
CNets::~CNets() {
	;
}
unsigned char CNets::init(HexEye* eye, int nNets, int nLev, int numLevNodes[], int numHanging) {
	m_genHexEye = NULL;/*no eye attached in this case*/
	if (nNets < 1)
		return ECODE_ABORT;
	m_nNets = nNets;
	m_genNet = new sNet;
	unsigned char err = m_genNet->init(eye, nLev, numLevNodes, numHanging);
	if (Err(err))
		return ECODE_FAIL;
	return ECODE_OK;
}
unsigned char CNets::init(int nNets, HexEye* eye, int numHanging) {
	if (nNets < 1)
		return ECODE_ABORT;
	m_nNets = nNets;
	m_genNet = new sNet;
	unsigned char err = m_genNet->init(eye, numHanging, hexeye_structure);
	if (Err(err))
		return ECODE_FAIL;
	m_genHexEye = eye;
	return ECODE_OK;
}
void CNets::release() {
	m_genHexEye = NULL;
	if (m_genNet != NULL) {
		m_genNet->release();
		delete m_genNet;
	}
	m_genNet = NULL;
	m_nNets = 0;
}
unsigned char CNets::spawnFullConn(s_CNets* cn, s_HexEye* eye) {
	if (cn == NULL || eye == NULL || m_genHexEye == NULL || m_genNet == NULL || m_nNets < 1)
		return ECODE_ABORT;
	unsigned char err = cn->init(m_nNets);
	cn->N = 0;
	for (int ii = 0; ii < m_nNets; ii++) {
		err = m_genNet->spawn(cn->net[ii], eye);
		if (Err(err))
			return ECODE_FAIL;
		cn->N++;
	}
	return ECODE_OK;
}
unsigned char CNets::spawn(s_CNets* cn, s_HexEye* eye) {
	if (cn == NULL || eye == NULL || m_genHexEye == NULL || m_genNet == NULL || m_nNets < 1)
		return ECODE_ABORT;
	if (Err(cn->init(m_nNets)))
		return ECODE_FAIL;
	if (Err(cn->newNets()))
		return ECODE_FAIL;
	cn->eye = eye;
	for (int ii = 0; ii < m_nNets; ii++) {
		unsigned char err = m_genNet->spawn(cn->net[ii], eye);
		if (Err(err))
			return ECODE_FAIL;
	}
	return ECODE_OK;
}
unsigned char CNets::spawn(s_CNets* cn) {
	if (cn == NULL || m_genNet == NULL || m_nNets < 1)
		return ECODE_ABORT;
	if (Err(cn->init(m_nNets)))
		return ECODE_FAIL;
	if (Err(cn->newNets()))
		return ECODE_FAIL;
	for (int ii = 0; ii < m_nNets; ii++) {
		unsigned char err = m_genNet->spawn(cn->net[ii], cn->eye);
		if (Err(err))
			return ECODE_FAIL;
	}
	return ECODE_OK;
}
void CNets::despawn(s_CNets* cn) {
	if (cn == NULL || m_genNet == NULL)
		return;
	for (int ii = 0; ii < m_nNets; ii++) {
		m_genNet->despawn(cn->net[ii]);
	}
	cn->delNets();
	cn->release();
}