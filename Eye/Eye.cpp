#include "Eye.h"

unsigned char Eye::init(HexImg* hexImg, float r, float twisted_root_radius, float sigma, int numRot, int numStackLevels) {
	m_hexImg = hexImg;
	m_twisted_root_radius = twisted_root_radius;
	m_hexStack = new HexStack;
	m_hexStack->init(r, numStackLevels);

	float root_base_dim_in_stack_R = getSizeRootBase();
	float root_base_dim = m_hexStack->getBottomR() * root_base_dim_in_stack_R;
	int span_root_plate = n_HexRect::minSpanOdd(root_base_dim, m_hexImg->getRhex());
	int lines_root_plate = n_HexRect::minLinesOdd(root_base_dim, m_hexImg->getRhex());
	m_hexRect = new rtHexRect;
	m_hexRect->init(span_root_plate, lines_root_plate, m_hexImg->getRhex());

	int numHanging = getNumhangingBaseToRoot();
	m_exHex = new ExHex;
	m_exHex->init(m_hexStack, numHanging);

	m_twistedPlate = new TwistedPlate;
	m_twistedPlate->init(m_exHex, sigma);

	m_Luna = new Luna;
	m_Luna->init(m_exHex);


}
float Eye::getSizeExBase() {
	float numHexes_stack_base = m_hexStack->getNumHexesLongDim();
	float numHexes_ex_stack_base = numHexes_stack_base + 1.f;
	const float short_side = 1.f / 2.f;
	float base_long_dim_in_R = sqrtf(short_side * short_side + numHexes_ex_stack_base * numHexes_ex_stack_base);
	return base_long_dim_in_R;
}
float Eye::getSizeRootBase() {
	float base_long_dim_in_R = getSizeExBase();
	float ex_root_radius = EYECORE_root_plate_buffer_factor * m_twisted_root_radius;
	return base_long_dim_in_R + ex_root_radius;
}
int Eye::getNumhangingBaseToRoot() {
	float radiusBottom = m_twisted_root_radius * m_hexStack->getBottomR();
	float area = PI * radiusBottom * radiusBottom;
	const float tri_height_ = sqrtf(3.f) / 2.f;
	const float tri_area = tri_height_ /* * base=1 */ / 2.f;
	const float hex_area = 6.f * tri_area;
	float bottom_hex_size = hex_area * m_hexImg->getRhex();
	float num_hex_in_area = area / bottom_hex_size;
	return (int)ceilf(num_hex_in_area);
}