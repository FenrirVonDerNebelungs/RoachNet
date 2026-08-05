#include "HexRect.h"

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
	return numHexSpan;
}
int n_HexRect::minLines(float minCovered_height, float r) {
	float rs = tangR(r);
	float peak_point_height = r - rs;
	float per_line_increase = peak_point_height + rs;
	if (per_line_increase <= 0.f)
		return 1;
	/*
	*  total_height_middle_lines_and_top_line = numLines * per_line_increase
	*  total_height = total_height_middle_lines_and_top_line + peak_point_height
	*  total_height_fully_covered = total_height - 2*peak_point_height
	* 
	*  total_height_fully_covered = total_height_middle_lines_and_top_line - peak_point_height
	*  total_height_fully_covered = numLines*per_line_increase - peak_point_height
	*/
	float numLines = (minCovered_height + peak_point_height) / per_line_increase;
	int numHexLines = (int)ceilf(numLines);
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
}
int n_HexRect::maxLines(float max_height, float r) {
	float rs = tangR(r);
	float peak_point_height = r - rs;
	float per_line_increase = peak_point_height + rs;
	if (per_line_increase <= 0.f)
		return 1;
	/*
	*  total_height_middle_lines_and_top_line = numLines * per_line_increase
	*  total_height = total_height_middle_lines_and_top_line + peak_point_height
	*  total_height = numLines*per_line_increase + peak_point_height
	*/

	float numLines = (max_height - peak_point_height)/per_line_increase;
	int numHexLines = (int)floorf(numLines);
}
unsigned char HexRect::spawn(s_HexPlate* plate) {
	if (plate == NULL)
		return ECODE_ABORT;
	if (m_numSpan <= 0 || m_numLines <= 0)
		return ECODE_ABORT;
	if(plate->init(m_numHexes, m_numHanging) != ECODE_OK)
		return ECODE_FAIL;
	unsigned char err = constructPlate(plate);
	if (err != ECODE_OK)
		return err;
	return ECODE_OK;
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
	if (numLongSpans > numShortSpans) {
		span2_start_i = span1_start_i;
		span1_start_i = span2_start_i - m_numInnerSpan;
		unsigned char err = weaveLines(plate, span1_start_i, span2_start_i);
		if (err != ECODE_OK)
			return ECODE_FAIL;
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
		if (span_i > 0) {
			s_Hex* nextHex = plate->get(span_start_i + span_i + 1);
			if (nextHex == NULL)
				return ECODE_FAIL;
			n_HexBase::stichHexes(hex, nextHex, 0);
		}
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

unsigned char rtHexRect::spawn(s_rtHexPlate* plate) {
	if (plate == NULL)
		return ECODE_ABORT;
	if (m_numSpan <= 0 || m_numLines <= 0)
		return ECODE_ABORT;
	if(plate->init(m_numHexes, m_numHanging) != ECODE_OK)
		return ECODE_FAIL;
	unsigned char err = constructPlate(plate);
	if (err != ECODE_OK)
		return err;
	if(!n_HexBase::computeVecHexDistances(plate, m_centerIndex))
		return ECODE_FAIL;
	return ECODE_OK;
}
unsigned char rtHexRect::setCenterIndex() {
	if (m_numSpan <= 0 || m_numLines <= 0)
		return -1;
	m_exactCenterIndex = true;
	long num2line_stacks = (long)m_numLines / 2; /*number of sets of long top line and short bottom line */
	if (m_numLines % 2 == 0)
		m_exactCenterIndex = false;
	if (num2line_stacks % 2 == 0) {
		/*even number of 2 line stacks center is on a long line*/
		long len_pre_stacks = num2line_stacks / 2L * (long)(m_numSpan + m_numInnerSpan);
		long center_of_line = m_numSpan / 2;
		if(m_numSpan%2==0)
			m_exactCenterIndex = false;
		else 
			center_of_line += 1;
		m_centerIndex = (len_pre_stacks + center_of_line);
	}
	else {
		/*odd number of 2 line stacks center is on a short line*/
		long len_pre_stacks = ((num2line_stacks-1) / 2L) * (long)(m_numSpan + m_numInnerSpan);
		long pre_len_lines = len_pre_stacks + m_numSpan;
		long center_of_line = m_numInnerSpan / 2;
		if(m_numInnerSpan%2==0)
			m_exactCenterIndex = false;
		else 
			center_of_line += 1;
		m_centerIndex = (pre_len_lines + center_of_line);
	}
	return ECODE_OK;
}