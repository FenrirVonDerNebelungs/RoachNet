#include "HexRect.h"

float n_HexRect::tangR(float r) {
	return r * sqrtf(3.f) / 2.f;
}

int n_HexRect::minSpan(float minCovered_width, float r) {
	if (r <= 0.f)
		return 1;
	float tanR = tangR(r);
	float hexWidth = 2.f * tanR;
	/* short width = numLong*hexWidth -1
	*  shortWidth_fully_covered = short_width - 1
	*  shortWidth_fully_covered = numLong*hexWidth - 2
	*  
	*/
	float numLong = (minCovered_width + 2.f) / hexWidth;
	int numHexSpan = (int)ceilf(numLong);
	numHexSpan += 1; /* the short span will be one shorter than the long span*/
	return numHexSpan;
}
int n_HexRect::minSpanOdd(float minCovered_width, float r) {
	int tryNumSpan = minSpan(minCovered_width, r); 
	if (tryNumSpan % 2 != 0) 
		tryNumSpan++; 
	return tryNumSpan;
}
int n_HexRect::minLines(float minCovered_height, float r) {
	float rs = tangR(r);
	float peak_point_height = r/2.f;
	float per_line_increase = peak_point_height + r;
	if (per_line_increase <= 0.f)
		return 1;

	/* numLines*per_line_increase - peak_point_height >=minCovered_height */
	float numLines = (minCovered_height + peak_point_height) / per_line_increase;
	int numHexLines = (int)ceilf(numLines);
	return numHexLines;
}
int n_HexRect::minLinesOdd(float minCovered_height, float r) {
	int tryNumLines = minLines(minCovered_height, r);
	if (tryNumLines % 2 == 0)
		tryNumLines++;
	return tryNumLines;
}
int n_HexRect::maxSpan(float max_width, float r) {
	if (r <= 0.f)
		return 1;
	/* long width = numLong*hexWidth
	*/
	float rs = tangR(r);
	float hexWidth = 2.f * rs;
	float numLong = max_width / hexWidth;
	int numHexSpan = (int)floorf(numLong);
	return numHexSpan;
}
int n_HexRect::maxLines(float max_height, float r) {
	float rs = tangR(r);
	float peak_point_height = r/2.f;
	float per_line_increase = peak_point_height + r;
	if (per_line_increase <= 0.f)
		return 1;
	/*
	*  total_height_middle_lines_and_top_line = numLines * per_line_increase
	*  total_height = total_height_middle_lines_and_top_line + peak_point_height
	*  total_height = numLines*per_line_increase + peak_point_height
	*/

	float numLines = (max_height - peak_point_height)/per_line_increase;
	int numHexLines = (int)floorf(numLines);
	return numHexLines;
}
HexRect::HexRect() :m_R(0.f), m_RS(0.f), m_numSpan(0), m_numInnerSpan(0), m_numLines(0), m_numHexes(0), m_numHanging(0), 
m_minWidth(0.f), m_maxWidth(0.f), m_minHeight(0.f), m_maxHeight(0.f)
{
	for (int i = 0; i < 6; i++)	
		utilStruct::zero2pt(m_U[i]);
}
HexRect::~HexRect() {
	;
}
unsigned char HexRect::init(int numHexSpanLong, int numHexLines, float r, int numHanging) {
	m_R = r;
	const float short_side_ratio = sqrtf(3.f) / 2.f;
	const float peak_only_height = 1.f - 0.5f;
	m_RS = m_R * short_side_ratio;
	n_Hex::genHexU_0(m_U);
	m_numSpan = numHexSpanLong;
	m_numInnerSpan = numHexSpanLong - 1;
	m_numLines = numHexLines;
	m_numHanging = numHanging;
	int numLongShortLinePairs = m_numLines / 2;
	int numHexesInLinePair = m_numSpan + m_numInnerSpan;
	m_numHexes = numLongShortLinePairs * numHexesInLinePair;
	if (m_numLines % 2 != 0) {
		m_numHexes += m_numSpan;
	}

	float num_minSpan_hexes = (float)m_numInnerSpan;
	m_minWidth = num_minSpan_hexes * 2.f*m_RS;
	float num_maxSpan_hexes = (float)m_numSpan;
	m_maxWidth = num_maxSpan_hexes * 2.f * m_RS;
	float inner_height_hex_lines = m_R * (1.f + 0.5f);
	m_minHeight = inner_height_hex_lines * ((float)(m_numLines - 1)) + m_R;
	m_maxHeight = inner_height_hex_lines * ((float)m_numLines) + m_R * peak_only_height;
	return ECODE_OK;
}
void HexRect::release() {
	m_R = 0.f;
	m_RS = 0.f;
	for (int i = 0; i < 6; i++)
		utilStruct::zero2pt(m_U[i]);
	m_numSpan = 0;
	m_numInnerSpan = 0;
	m_numLines = 0;
	m_numHexes = 0;
	m_numHanging = 0;
	m_minWidth = 0.f;
	m_maxWidth = 0.f;
	m_minHeight = 0.f;
	m_maxHeight = 0.f;
}
unsigned char HexRect::spawn(s_HexPlate* plate, s_2pt* hexUs) {
	if (plate == NULL)
		return ECODE_ABORT;
	if (m_numSpan <= 0 || m_numLines <= 0)
		return ECODE_ABORT;
	if (hexUs == NULL) {
		if (plate->init(m_numHexes, m_U, m_R, m_numHanging) != ECODE_OK)
			return ECODE_FAIL;
	}
	else {
		if (plate->init(m_numHexes, hexUs, m_R, m_numHanging) != ECODE_OK)
			return ECODE_FAIL;
	}
	unsigned char err = constructPlate(plate);
	if (err != ECODE_OK)
		return err;
	return ECODE_OK;
}
void HexRect::despawn(s_HexPlate* plate) {
	if (plate == NULL)
		return;
	plate->release();
}
unsigned char HexRect::constructPlate(s_HexPlate* plate) {
	if (plate == NULL)
		return ECODE_ABORT;
	if (m_numSpan <= 0 || m_numLines <= 0)
		return ECODE_ABORT;
	if(plate->N!= m_numHexes)
		return ECODE_ABORT;
	int numHexes = 0;
	int numLongSpans = 0;
	int numShortSpans = 0;
	do {
		unsigned char err = layDownSpan_and_Weave(plate, numHexes, m_numSpan);
		if (err != ECODE_OK)
			return ECODE_FAIL;
		numHexes += m_numSpan;
		numLongSpans++;
		if(numHexes<=(plate->N-m_numInnerSpan)){
			unsigned char err = layDownSpan_and_Weave(plate, numHexes, m_numInnerSpan);
			if (err != ECODE_OK)
				return ECODE_FAIL;
			numHexes += m_numInnerSpan;
			numShortSpans++;
		}
	} while (numHexes <= (plate->N - m_numSpan));
	long span1_start_i =0;
	long span2_start_i = m_numSpan;
	for(int long_line_i=0; long_line_i<numShortSpans; long_line_i++) {
		unsigned char err = weaveLines(plate, span1_start_i, span2_start_i);
		if (err != ECODE_OK)
			return ECODE_FAIL;
		span1_start_i += m_numSpan+m_numInnerSpan;
		span2_start_i += m_numSpan + m_numInnerSpan;
	}
	span1_start_i = m_numSpan;
	span2_start_i = span1_start_i + m_numInnerSpan;
	for (int long_line_i = 0; long_line_i < numShortSpans; long_line_i++) {
		unsigned char err = weaveLinesUpsidedown(plate, span1_start_i, span2_start_i);
		if (err != ECODE_OK)
			return ECODE_FAIL;
		span1_start_i += m_numSpan + m_numInnerSpan;
		span2_start_i += m_numSpan + m_numInnerSpan;
	}
	return ECODE_OK;
}
unsigned char HexRect::layDownSpan_and_Weave(s_HexPlate* plate, long span_start_i, long numSpan) {
	if (plate == NULL)
		return ECODE_ABORT;
	if (span_start_i < 0 || span_start_i >= plate->N)
		return ECODE_ABORT;
	if (numSpan <= 0 || (span_start_i + numSpan) > plate->N)
		return ECODE_ABORT;
	for (long span_i = 0; span_i < (numSpan-1); span_i++) {
		s_Hex* hex = plate->get(span_start_i + span_i);
		if (hex == NULL)
			return ECODE_FAIL;
		s_Hex* nextHex = plate->get(span_start_i + span_i + 1);
		if (nextHex == NULL)
			return ECODE_FAIL;
		n_HexBase::stichHexes(hex, nextHex, 0);	
	}
	return ECODE_OK;
}
unsigned char HexRect::weaveLines(s_HexPlate* plate, long span1_start_i, long span2_start_i) {
	if (plate == NULL)
		return ECODE_ABORT;
	if (span1_start_i < 0 || (span1_start_i+m_numSpan) > span2_start_i)
		return ECODE_ABORT;
	if (span2_start_i < 0 || (span2_start_i+m_numInnerSpan) > plate->N)
		return ECODE_ABORT;
	for(int i_short_span=0; i_short_span<m_numInnerSpan; i_short_span++) {
		s_Hex* hex1 = plate->get(span1_start_i + i_short_span);
		s_Hex* hex2 = plate->get(span2_start_i + i_short_span);
		if (hex1 == NULL || hex2 == NULL)
			return ECODE_FAIL;
		n_HexBase::stichHexes(hex1, hex2, 5);
	}
	for (int i_short_span = 0; i_short_span < m_numInnerSpan; i_short_span++) {
		s_Hex* hex1 = plate->get(span1_start_i + i_short_span+1);
		s_Hex* hex2 = plate->get(span2_start_i + i_short_span);
		if (hex1 == NULL || hex2 == NULL)
			return ECODE_FAIL;
		n_HexBase::stichHexes(hex1, hex2, 4);
	}
	return ECODE_OK;
}
unsigned char HexRect::weaveLinesUpsidedown(s_HexPlate* plate, long span1_start_i, long span2_start_i) {
	if (plate == NULL)
		return ECODE_ABORT;
	if (span1_start_i < 0 || (span1_start_i+m_numInnerSpan) > span2_start_i)
		return ECODE_ABORT;
	if (span2_start_i < 0 || span2_start_i > plate->N)
		return ECODE_ABORT;
	for(int i_short_span=0; i_short_span<m_numInnerSpan; i_short_span++) {
		s_Hex* hex1 = plate->get(span1_start_i + i_short_span);
		s_Hex* hex2 = plate->get(span2_start_i + i_short_span);
		if (hex1 == NULL || hex2 == NULL)
			return ECODE_FAIL;
		n_HexBase::stichHexes(hex1, hex2, 4);
	}
	for (int i_short_span = 0; i_short_span < m_numInnerSpan; i_short_span++) {
		s_Hex* hex1 = plate->get(span1_start_i + i_short_span);
		s_Hex* hex2 = plate->get(span2_start_i + i_short_span+1);
		if (hex1 == NULL || hex2 == NULL)
			return ECODE_FAIL;
		n_HexBase::stichHexes(hex1, hex2, 5);
	}
	return ECODE_OK;
}
rtHexRect::rtHexRect() : m_centerIndex(0), m_exactCenterIndex(false)
{
	;
}
rtHexRect::~rtHexRect() {
	;
}
unsigned char rtHexRect::init(int numHexSpanLong, int numHexLines, float r, int numHanging) {
	unsigned char err = HexRect::init(numHexSpanLong, numHexLines, r, numHanging);
	if (err != ECODE_OK)
		return err;
	err = setCenterIndex();
	if (err != ECODE_OK)
		return err;
	return ECODE_OK;
}
unsigned char rtHexRect::spawn(s_rtHexPlate* plate, s_2pt* hexUs) {
	if (plate == NULL)
		return ECODE_ABORT;
	if (m_numSpan <= 0 || m_numLines <= 0)
		return ECODE_ABORT;
	if (hexUs == NULL) {
		if (plate->init(m_numHexes, m_U, m_R, m_numHanging) != ECODE_OK)
			return ECODE_FAIL;
	}
	else {
		if (plate->init(m_numHexes, hexUs, m_R, m_numHanging) != ECODE_OK)
			return ECODE_FAIL;
	}
	plate->width = (long)ceilf(m_maxWidth);
	plate->height = (long)ceilf(m_maxHeight);
	unsigned char err = constructPlate(plate);
	if (err != ECODE_OK)
		return err;
	if(!n_HexBase::computeVecHexDistances(plate, m_centerIndex))
		return ECODE_FAIL;
	n_HexBase::reduceVecHexDistances(plate);
	return ECODE_OK;
}
void rtHexRect::despawn(s_rtHexPlate* plate) {
	if (plate == NULL)
		return;
	plate->release();
}
unsigned char rtHexRect::setCenterIndex() {
	if (m_numSpan <= 0 || m_numLines <= 0)
		return -1;
	m_exactCenterIndex = true;
	int center_hex_i = m_numHexes / 2;
	/* center_hex_i +1 = center hex count however center index is center hex count -1 */
	if (m_numHexes % 2 == 0) 
		m_exactCenterIndex = false;
	m_centerIndex = center_hex_i;
	return ECODE_OK;
}