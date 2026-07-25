#include "Color.h"

s_ColPlateLayer::s_ColPlateLayer() { ; }
s_ColPlateLayer::~s_ColPlateLayer() { ; }

Col::Col() :m_genCol(NULL), m_Cols(NULL), m_N_Cols(0), m_mem_Cols(0) {
	;
}
Col::~Col() {
	;
}
unsigned char Col::init(int nCols) {
	m_genCol = new ColPlate;
	if (Err(m_genCol->init()))
		return ECODE_FAIL;
	if (nCols < 1)
		return ECODE_FAIL;
	m_mem_Cols = nCols;
	m_Cols = new s_ColWheel[m_mem_Cols];
	m_N_Cols = 0;
	return ECODE_OK;
}
void Col::release() {
	if (m_Cols != NULL) {
		delete[] m_Cols;
		m_Cols = NULL;
	}
	m_N_Cols = 0;
	m_mem_Cols = 0;
	if (m_genCol != NULL) {
		m_genCol->release();
		delete m_genCol;
		m_genCol = NULL;
	}
}
unsigned char Col::addCol(s_ColWheel* col) {
	if (col == NULL)
		return ECODE_ABORT;
	if (m_N_Cols >= m_mem_Cols)
		return ECODE_ABORT;
	n_ColWheel::copy(m_Cols[m_N_Cols], *col);
	m_N_Cols++;
	return ECODE_OK;
}
unsigned char Col::spawn(s_HexPlate* hexedImg, s_ColPlateLayer* colPlates) {
	unsigned char errc = ECODE_OK;
	errc = initPlateLayer(colPlates);
	if (Err(errc))
		return errc;
	for (int i = 0; i < m_N_Cols; i++) {
		errc |= m_genCol->spawn(hexedImg, (s_ColPlate*)colPlates->get(i), m_Cols[i]);
	}
	return errc;
}
void Col::despawn(s_ColPlateLayer* colPlates) {
	if (colPlates == NULL)
		return;
	for (int i = 0; i < m_N_Cols; i++)
		m_genCol->despawn((s_ColPlate*)colPlates->get(i));
	releasePlateLayer(colPlates);
}
unsigned char Col::run(s_HexPlate* hexedImg, s_ColPlateLayer* colPlates) {
	long num_hexes = hexedImg->N;
	for (long hex_i = 0; hex_i < num_hexes; hex_i++) {
		n_Col::run(hexedImg, colPlates, hex_i);
	}
	return ECODE_OK;
}

unsigned char Col::initPlateLayer(s_ColPlateLayer* colPlates) {
	if (colPlates == NULL)
		return ECODE_ABORT;
	if (Err(colPlates->init(m_N_Cols)))
		return ECODE_FAIL;
	colPlates->N = 0;
	for (int i = 0; i < m_N_Cols; i++) {
		colPlates->p[i] = (s_HexPlate*)(new s_ColPlate);
		colPlates->N++;
	}
	return ECODE_OK;
}
void Col::releasePlateLayer(s_ColPlateLayer* colPlates) {
	/*assumes that the individual plates in the layer have already been released*/
	if (colPlates != NULL) {
		for (int i = 0; i < colPlates->getNmem(); i++) {
			if (colPlates->p[i] != NULL)
				delete colPlates->p[i];
		}
		colPlates->release();
	}
}

bool n_Col::run(s_HexPlate* hexedImg, s_ColPlateLayer* colPlates, long plate_hex_index) {
	bool retVal = true;
	for (long ii = 0; ii < colPlates->N; ii++)
		retVal &= runPlate(hexedImg, colPlates, ii, plate_hex_index);
	return retVal;
}
bool n_Col::runPlate(s_HexPlate* hexedImg, s_ColPlateLayer* colPlates, long layer_index, long plate_hex_index) {
	s_ColPlate* colPlt = colPlates->get(layer_index);
	return n_ColPlate::run(hexedImg, colPlt, plate_hex_index);
}