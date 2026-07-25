#include "ColPlate.h"

s_ColPlate::s_ColPlate() {
	n_ColWheel::clear(Col);
}
s_ColPlate::~s_ColPlate()
{
	;
}
ColPlate::ColPlate() {
	;
}
ColPlate::~ColPlate() {
	;
}
unsigned char ColPlate::init() {
	return ECODE_OK;
}
void ColPlate::release() {
	;
}
unsigned char ColPlate::spawn(s_HexPlate* hexedImg, s_ColPlate* colPlate, const s_ColWheel& Col) {
	if (hexedImg == NULL || colPlate == NULL)
		return ECODE_ABORT;
	unsigned char err=colPlate->init(hexedImg);
	if (Err(err))
		return err;
	err = setDownLinks(hexedImg, colPlate);
	if (Err(err))
		return err;
	n_ColWheel::copy(colPlate->Col, Col);
	setColWheelUnitVectors(colPlate->Col);
	colPlate->Col.DhueRes = 2.f - colPlate->Col.Dhue;
	return ECODE_OK;
}
void ColPlate::despawn(s_ColPlate* colPlate) {
	if (colPlate == NULL)
		return;
	colPlate->release();
}
void ColPlate::resetCol(s_ColPlate* colPlate, const s_ColWheel& Col) {
	if (colPlate == NULL)
		return;
	n_ColWheel::resetCol(colPlate->Col, Col);
}
unsigned char ColPlate::setDownLinks(s_HexPlate* hexedImg, s_ColPlate* colPlate) {
	if (hexedImg->N != colPlate->N)
		return ECODE_ABORT;
	for (long ii = 0; ii < hexedImg->N; ii++) {
		s_Hex* botHex = hexedImg->get(ii);
		s_Hex* topHex = colPlate->get(ii);
		for (int i_link = 0; i_link < topHex->N; i_link++)
			topHex->nodes[i_link] = NULL;
		topHex->nodes[0] = (s_Node*)botHex;/* 0 is ok since all s_Hex's have 7 down nodes in mem*/
		topHex->N = 1;
	}
	return ECODE_OK;
}
void ColPlate::setColWheelUnitVectors(s_ColWheel& col) {
	return n_ColWheel::setColWheelUnitVectors(col);
}
void n_ColWheel::setColWheelUnitVectors(s_ColWheel& col) {
	/*asume Ur goes out along the x0 axis*/
	col.Ur.x0 = 1.f;
	col.Ur.x1 = 0.f;
	/*the other 2 vectors are 120 degrees or 2PI/3 rad away from this one*/
	col.Ug.x0 = -cosf(PI / 3.f);
	col.Ug.x1 = sinf(PI / 3.f);
	col.Ub.x0 = col.Ug.x0;
	col.Ub.x1 = -col.Ug.x1;
}

void n_ColWheel::clear(s_ColWheel& Col) {
	Col.pixMax = 255.f;
	Col.Dhue = 1.f;
	Col.DI = 1.f;
	Col.DSat = 1.f;
	Col.HueFadeV = 0.f;
	Col.I_target = 1.f;
	utilStruct::zero2pt(Col.Hue_target);
	Col.Sat_target = 1.f;
	Col.finalScaleFactor = 1.f;
	utilStruct::zero2pt(Col.Ur);
	utilStruct::zero2pt(Col.Ug);
	utilStruct::zero2pt(Col.Ub);
	Col.DhueRes = 0.f;
}
void n_ColWheel::resetCol(s_ColWheel& col, const s_ColWheel& orig) {
	col.Dhue = orig.Dhue;
	col.DI = orig.DI;
	col.DSat = orig.DSat;
	col.HueFadeV = orig.HueFadeV;
	col.I_target = orig.I_target;
	utilStruct::copy2pt(col.Hue_target, orig.Hue_target);
	col.Sat_target = orig.Sat_target;
	//col.finalScaleFactor = orig.finalScaleFactor;
	col.DhueRes = 2.f - col.Dhue;
}
void n_ColWheel::copy(s_ColWheel& Col, const s_ColWheel& orig) {
	Col.pixMax = orig.pixMax;
	Col.finalScaleFactor = orig.finalScaleFactor;
	utilStruct::copy2pt(Col.Ur, orig.Ur);
	utilStruct::copy2pt(Col.Ug, orig.Ug);
	utilStruct::copy2pt(Col.Ub, orig.Ub);
	resetCol(Col, orig);
}

bool n_ColPlate::run(s_HexPlate* hexedImg, s_ColPlate* colPlate, long plate_index) {
	/*hexedImg->N must equal colPlate->N*/
	s_Hex* imgHex = hexedImg->get(plate_index);
	float col_val = findColDistances(colPlate->Col, imgHex->rgb);
	s_Hex* colHex = colPlate->get(plate_index);
	colHex->o = col_val;
	return true;
}
float n_ColPlate::findColDistances(const s_ColWheel& cw, float rgb[]) {
	s_ColWheel_Col wheel_col = findColWheelHue(cw,rgb);
	float hDist = findHueDistance(cw,wheel_col.hue,wheel_col.huelen);
	float IDist = findIDistance(cw,wheel_col);
	float SatDist = findSatDistance(cw,wheel_col);
	//float col_dist = 2.f*((hDist * IDist * SatDist)-0.5f);
	//return Math::StepFunc(cw.finalScaleFactor * col_dist);
	/*modify for symetric instead*/
	float col_dist = (hDist * IDist * SatDist);
	return Math::StepFuncSym(cw.finalScaleFactor * col_dist);
}
s_ColWheel_Col n_ColPlate::findColWheelHue(const s_ColWheel& cw, float rgb[]) {
	s_ColWheel_Col wcol;
	float shortest_rgb = rgb[0];
	for (int ii = 1; ii < 3; ii++) {
		if (rgb[ii] < shortest_rgb) {
			shortest_rgb = rgb[ii];
		}
	}
	shortest_rgb /= cw.pixMax;
	wcol.shortest_rgb = shortest_rgb;


	for (int ii = 0; ii < 3; ii++) {
		wcol.rgb[ii] = rgb[ii] / cw.pixMax;		
	}

	wcol.hue_Intensity = 0.f;
	float r = wcol.rgb[0] - shortest_rgb;
	float g = wcol.rgb[1] - shortest_rgb;
	float b = wcol.rgb[2] - shortest_rgb;
	wcol.hue_Intensity = (r + g + b) / 3.f;

	s_2pt Rv = { r * cw.Ur.x0, r * cw.Ur.x1 };
	s_2pt Gv = { g * cw.Ug.x0, g * cw.Ug.x1 };
	s_2pt Bv = { b * cw.Ub.x0, b * cw.Ub.x1 };
	wcol.hue = vecMath::add(Rv, Gv, Bv);
	wcol.huelen = vecMath::len(wcol.hue);

	wcol.Intensity = (wcol.rgb[0] + wcol.rgb[1] + wcol.rgb[2]) / 3.f;

	return wcol;
}

float n_ColPlate::findHueDistance(const s_ColWheel& cw, s_2pt& hueV, float hueLen) {
	if (cw.DhueRes <= 0.f)
		return 1.f;
	if (hueLen < cw.HueFadeV) {
		return 1.f;
	}
	/*find rot dist in color wheel*/
	s_2pt hueU = { hueV.x0 / hueLen, hueV.x1 / hueLen };
	float hueDot = vecMath::dot(hueU, cw.Hue_target);/*this should range from 1 to -1*/
	float hueDiff = 1.f - hueDot;/*0 for aligned with target, 2 for opposite to target*/

	float hueRet = 0.f;
	if (hueDiff <= cw.Dhue) {
		float sepval = hueDiff / cw.Dhue; /*sep value ranges from 1 for hueDiff=Dhue to 0 for hueDiff=0 aligned with target*/
		sepval = 1.f - sepval;
		hueRet = sepval;/*hue ret is 1 for hue aligned with target, 0 for hue a distance of hueDiff=Dhue away from target*/
	}
	else if (hueDiff > cw.Dhue) {
		float resid = 2.f - hueDiff; /*this can range from 2.f-Dhue for most aligned to 0 for hue opposite to target*/
		float sepval = resid / cw.DhueRes; /* ranges from 2-Dhue/2-Dhue = 1 for most aligned to 0 for hue opposite to target*/
		sepval = 1.f - sepval;/* 0 for hue Dhue away from target, 1 for hue opposite to target*/
		sepval = -sepval;
		hueRet = sepval;/* 0 for hue Dhue away from target, -1 for hue opposite to target*/
	}
	return (hueRet >= 0.f) ? hueRet : 0.f;/*currently only reporting hues within Dhue*/
}
float n_ColPlate::findIDistance(const s_ColWheel& cw, s_ColWheel_Col& wcol) {
	float Idist = fabsf(wcol.Intensity - cw.I_target);
	float norDist = Idist / cw.DI;/*0 if I is at target, 1 if I is at DI from target, >1 if I is at a distance of greater than DI from target*/
	norDist -= 1.f;/*-1 if I is at target, 0 if I is DI from target, >0 if I is greater than DI from target*/
	norDist = -norDist;/*1 if I is at target 0 if I is DI from target <0 if I is greater than DI from target*/
	return (norDist >= 0.f) ? norDist : 0.f;/*1 if I is at target to 0 if I is at DI from target and 0 if I is farther than DI from target*/
}
float n_ColPlate::findSatDistance(const s_ColWheel& cw, s_ColWheel_Col& wcol) {
	if (cw.DSat > 1.f)
		return 1.f;

	float satVal = ( wcol.Intensity > 0.f) ? wcol.hue_Intensity / wcol.Intensity : 0.f; /*at highest hue intensity 1.0 at lowest 0*/
	float valDiff = fabsf(satVal - cw.Sat_target);/*max 1.0 if far from Sat target 0 for on sat target*/
	float NormValDiff = (cw.DSat > 0.f) ? valDiff / cw.DSat : valDiff; /*if DSat is >0; then 0 for on target 1 for DSat away and >1 for greater than Dsat away*/
	float satDist = (1.f - NormValDiff);/*1 for on target, 0 for dSat away from target, <0 for more than Dsat away from target*/
	return (satDist >= 0.f) ? satDist : 0.f;/*1 for on target 0 for dSat or more away from target*/
}

bool n_ColPlate::setRGBFromColWheel(const s_ColWheel& cw, s_ColWheel_Col& wcol, float rgb[]) {
	if (!getRGBColoredFromHueV(cw,wcol.hue, wcol.rgb))
		return false;
	/* h1*m + h2*m   hue 1st component and hue 2nd component * hue magnitude, m
	*  h1*m +s +h2*m + s + s = I*3     hues plus s, the shortest rgb
	*  h1*m+h2*m = I*3 - 3*s
	*  (h1+h2)*m = 3(I-s)
	*  m = 3(I-s)/(h1+h2)
	*/
	/*compute multiplier for hue*/
	float Uhue_sum = 0.f;
	for (int ii = 0; ii < 3; ii++)
		Uhue_sum += wcol.rgb[ii];
	if (Uhue_sum <= 0.f)
		return false;
	float hue_mul = 3.f * (wcol.Intensity - wcol.shortest_rgb) / Uhue_sum;
	float max_allowed_hue_mul = 1.f - wcol.shortest_rgb;
	if (hue_mul > max_allowed_hue_mul)
		hue_mul = max_allowed_hue_mul;
	/*reset the rgb to the correct len*/
	for (int ii = 0; ii < 3; ii++)
		wcol.rgb[ii] *= hue_mul;
	/*now set the rgb by adding a and b to the correct pixels*/
	for (int ii = 0; ii < 3; ii++)
		wcol.rgb[ii] += wcol.shortest_rgb;

	/*now set the return value*/
	for (int ii = 0; ii < 3; ii++)
		rgb[ii] = wcol.rgb[ii];
	return true;
}
bool n_ColPlate::getRGBFromColWheel(const s_ColWheel& cw, float rgb[]) {
	for (int i_col = 0; i_col < 3; i_col++)
		rgb[i_col] = 0.f;

	if (cw.I_target < 0.f) {
		return true;
	}
	float col_rgb[3];
	if (!getRGBColoredFromHueV(cw, cw.Hue_target, col_rgb))
		return false;
	for (int i_col = 0; i_col < 3; i_col++)
		rgb[i_col] = col_rgb[i_col];
	/*Sat_target = hue_Intensity/I_target*/
	float hue_I = cw.Sat_target * cw.I_target;

	/*hue_I = (col_rgb[0]+col_rgb[1] + col_rgb[2])/3,  Intensity = (r + g +b) /3
	*  col_rgb = m*u_rgb m is the magnitude of that scales the hue unit vector
	* hue_I = (m * u_rgb[0] + m*u_rgb[1]+m*u_rgb[2])/3
	* 3*hue_I/(u_rgb[0] + u_rgb[1]+u_rgb[2]) = m
	* 
	*  s= shortest_rgb
	*  (col_rgb[0]+s + col_gb[1]+s + col_rgb[2]+s)/3 = Intensity
	*  (col_rgb[0]+col_rgb[1]+col_rgb[2])/3 + s = Intensity
	*  hue_I + s = Intensity
	*  s=Intensity - hue_I
	*/
	float shortest_rgb = cw.I_target - hue_I;
	float sum_u_rgb = 0.f;
	for (int i_col = 0; i_col < 3; i_col++)
		sum_u_rgb += rgb[i_col];/*this should not =0 since that would return false*/
	float mul_col_rgb_fac = 3.f * hue_I/sum_u_rgb;

	for (int i_col = 0; i_col < 3; i_col++) {
		rgb[i_col] *= mul_col_rgb_fac;
		rgb[i_col] += shortest_rgb;
	}
	return true;
}
bool n_ColPlate::getRGBFromHueVIntensity(const s_ColWheel& cw, const s_2pt& hueV, float Intensity, float rgb[]) {
	float rgbcol[3];
	for (int i = 0; i < 3; i++)
		rgbcol[i] = 0.f;
	if (!getRGBColoredFromHueV(cw, hueV, rgbcol))
		return false;
	/* ((c1+c)+(c2+c)+(0+c))/3.f=I
	*  (c1+c2+3*c)/3.f=I
	*  3*I=c1+c2+3*c
	*  3*I-(c1+c2) = 3*c
	*  I-(c1+c2)/3 = c
	*/
	/*only 2 of the rgbcol will be set sofind c1+c2*/
	float c1Plusc2 = 0.f;
	for (int i = 0; i < 3; i++)
		c1Plusc2 += rgbcol[i];
	float c = Intensity - c1Plusc2 / 3.f;
	for (int i = 0; i < 3; i++)
		rgb[i] = rgbcol[i] + c;
	return true;
}

bool n_ColPlate::getRGBColoredFromHueV(const s_ColWheel& cw, const s_2pt& hueV, float rgb[]) {
	for (int ii = 0; ii < 3; ii++)
		rgb[ii] = 0.f;
	if (vecMath::len(hueV) <= 0.5f) {
		rgb[0] = 1.f;
		return false;
	}
	/*find which of the 3 sectors the hue vector points into, Ur is -gb, Ug is -rb, etc...*/
	float bi_len[3];
	bi_len[0] = -1.f * vecMath::dot(hueV, cw.Ur);
	bi_len[1] = -1.f * vecMath::dot(hueV, cw.Ug);
	bi_len[2] = -1.f * vecMath::dot(hueV, cw.Ub);
	float biggest_bi=-1.f;
	int i_bi = 0;
	for (int ii = 0; ii < 3; ii++) {
		if (bi_len[ii] > biggest_bi) {
			biggest_bi = bi_len[ii];
			i_bi = ii;
		}
	}
	/*set the two component vectors*/
	s_2pt Ua;
	s_2pt Ub;
	if (i_bi == 0) {
		Ua = cw.Ug;
		Ub = cw.Ub;
	}
	else if (i_bi == 1) {
		Ua = cw.Ub;
		Ub = cw.Ur;
	}
	else if (i_bi == 2) {
		Ua = cw.Ur;
		Ub = cw.Ug;
	}
	s_2pt colhue = { 0.f, 0.f };
	if (!getColoredHueCompFromHue(hueV, Ua, Ub, colhue))
		return false;
	float a = colhue.x0;
	float b = colhue.x1;
	if (i_bi == 0) {
		rgb[1] = a;/*g*/
		rgb[2] = b;/*b*/
	}
	else if (i_bi == 1) {
		rgb[2] = a;/*b*/
		rgb[0] = b;/*r*/
	}
	else if (i_bi == 2) {
		rgb[0] = a;/*r*/
		rgb[1] = b;/*g*/
	}
	return true;
}
bool n_ColPlate::getColoredHueCompFromHue(const s_2pt& vh, const s_2pt& U1, const s_2pt& U2, s_2pt& colhue) {
	/* vh.x = h1 * U1.x + h2*U2.x
	   vh.y = h1 * U1.y + h2* U2.y */

	/* U*.x is always nonzero */
	/* vh.x/U1.x = h1 + h2*U2.x/U1.x
	*  h1 = vh.x/U1.x - h2*U2.x/U1.x  */

	/* vh.y = (vh.x/U1.x - h2*U2.x/U1.x)*U1.y + h2*U2.y
	 *      = vh.x*U1.y/U1.x - h2*U2.x*U1.y/U1.x + h2*U2.y
	 *      = vh.x*U1.y/U1.x + h2(-U2.x*U1.y/U1.x + U2.y) 
	 * vh.y - vh.x*U1.y/U1.x = h2(U2.y - U2.x*U1.y/U1.x)
	 * (vh.y - vh.x*U1.y/U1.x)/(U2.y - U2.x*U1.y/U1.x) = h2
	 * (vh.y*U1.x - vh.x*U1.y)/(U2.y*U1.x - U2.x*U1.y)=h2
	 * h2 = (vh.y*U1.x - vh.x*U1.y)/(U2.y*U1.x - U2.x*U1.y)
	 */
	if (U1.x0 == 0.f)
		return false;
	colhue.x1 = (vh.x1 * U1.x0 - vh.x0 * U1.x1 ) / (U2.x1 * U1.x0 - U2.x0 * U1.x1);
	colhue.x0 = vh.x0 / U1.x0 - colhue.x1 * U2.x0 / U1.x0;
	/*triangle from adding the U1 and U2 is narrow so some vectors of 1 in  the hue space may have sides from the 2 vectors added that are greater than 1*/
	float max_component = (colhue.x1 > colhue.x0) ? colhue.x1 : colhue.x0;
	if (max_component > 1.f) {
		colhue.x0 /= max_component;
		colhue.x1 /= max_component;
	}
	return true;
}