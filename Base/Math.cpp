#include "Math.h"
#include "Img.h"
void n_gaussianInt::clear(s_gaussianInt& gI) {
	gI.sigma = 0.f;
	gI.max = 0.f;
	gI.N = 0;
	gI.I = NULL;
	gI.X = NULL;
}
bool n_gaussianInt::init(s_gaussianInt& gI, float sigma, float max, int N) {
	gI.sigma = fabsf(sigma);
	if (sigma == 0.f)
		return false;
	if (max > 0.f)
		gI.max = max;
	else
		gI.max = 2.f * sigma;
	gI.N=(float)N;
	if (N < 1)
		return false;
	gI.I = new float[N];
	gI.X = new float[N];
	for (int i = 0; i < gI.N; i++) {
		gI.I[i] = 0.f;
		gI.X[i] = 0.f;/*this will be set when the integral values are set*/
	}
	return true;
}
void n_gaussianInt::release(s_gaussianInt& gI) {
	if (gI.X != NULL) {
		delete[]gI.X;
	}
	gI.X = NULL;
	if (gI.I != NULL) {
		delete[] gI.I;
	}
	gI.I = NULL;
	gI.N = 0;
}

namespace Math {
	float Ang2PI(float ang) {
		float _2PI = 2.f * PI;
		while (ang < 0.f)
			ang += _2PI;
		while (ang >= _2PI)
			ang -= _2PI;
		if (ang < 0.f)/*fix rounding err*/
			ang = 0.f;
		return ang;
	}
	float StepFunc(float x) {
		/*1/(1+exp(-x))*/
		float expofn = expf(-x);
		//if (expofn <= 0.f)/*shouldn't happen*/
		//	return 0.f;
		float denom = 1.f + expofn;
		return 1.f / denom;
	}
	int loop(int i, int n) {
		int finali = i;
		while (finali >= n)
			finali -= n;
		while (finali < 0)
			finali += n;
		return finali;
	}
	float powerXseries(float x, int n) {
		if (n == 0)
			return 1.f;
		/* 1+x+x^2 + .. x^n = S
		   xS=1+x+x^2+x^3+...x^(n+1) - 1
		   xS=S+x^(n+1)-1
		   xS-S = x^(n+1)-1
		   S=(x^(n+1)-1)/x-1 */
		float X_N1 = power(x, (n + 1));
		float X_N1_1 = X_N1 - 1.f;
		float X_1 = x - 1.f;
		if (X_1 == 0.f) {
			return 1.f + ((float)n);
		}
		return X_N1_1 / X_1;
	}
	float power(float x, int y) {
		float res = 1.f;
		for (int i = 0; i < y; i++)
			res *= x;
		return res;
	}
	float Gaussian(float pt, float norm_const, float center) {
		float Norm = 1.f / sqrtf(2.f * PI * norm_const * norm_const);
		float sqrt2_ExpConst = sqrtf(2.f) * norm_const;
		return GaussianFast(pt, sqrt2_ExpConst, Norm, center);
	}
	float GaussianOneMax(float pt, float norm_const) {/*used for art is 1 at pt=0*/
		float sqrt2_ExpConst = sqrtf(2.f) * norm_const;
		float expval = pt / sqrt2_ExpConst;
		expval *= expval;
		return expf(-expval);
	}
	float GaussianFast(float pt, float sqrt2_ExpConst, float Norm, float center)
	{
		float dist = pt-center;
		float expval = dist / sqrt2_ExpConst;
		expval = expval * expval;
		float val = expf(-expval);
		return val * Norm;
	}
	bool GaussianNumInt(float norm_const, float max, float N, float I[], float x[]) {
		if (I == NULL || x==NULL)
			return false;
		float numDiv = N - 1.f;
		if (numDiv < 1.f)
			return false;
		float dX = max / numDiv;
		if (dX <= 0.f)
			return false;
		int max_cnt = (int)roundf(numDiv);
		float X = 0;
		float gauss = Gaussian(X, norm_const, 0.f);
		float prev_gauss = gauss;
		I[0] = 0.f;
		x[0] = X;
		for (int i = 1; i < max_cnt; i++) {
			X += dX;
			x[i] = X;
			gauss = Gaussian(X, norm_const, 0.f);
			float ave_gauss = (gauss + prev_gauss) / 2.f;
			float dI = ave_gauss * dX;
			I[i] = I[i - 1] + dI;
			prev_gauss = gauss;
		}
		return true;
	}
	bool GaussianNumInt(s_gaussianInt& Int) {
		if (Int.N < 2)
			return false;
		if (Int.I == NULL || Int.N == NULL)
			return false;
		return GaussianNumInt(Int.sigma, Int.max, Int.N, Int.I, Int.X);
	}
	void timeSeed() {
		std::srand((unsigned int)std::time(0));
	}
	float randAng(float startRad, float endRad) {
		int randFullRange = rand() % MATH_RANDMOD;
		float rand_ = (float)randFullRange;
		float angRange = endRad - startRad;
		float _2PI = 2.f * PI;
		if (fabsf(angRange) < _2PI)
			angRange = Ang2PI(angRange);
		else
			angRange = _2PI;
		float randmodrange = (float)MATH_RANDMOD;
		float randdang = rand_ * angRange / randmodrange;
		return Ang2PI(startRad + randdang);
	}
	float randGausPt(const s_gaussianInt& gI) {
		/*assumes I has already been filled*/
		if (gI.I == NULL || gI.X == NULL || gI.N < 2)
			return 0.f;
		int rand200 = rand() % MATH_RANDMOD;
		int randModHalf = MATH_RANDMOD / 2;
		int rand100 = rand200;
		bool signpos = false;
		if (rand200 >= randModHalf) {
			signpos = true;
			rand100 = rand200 - randModHalf;
		}
		/*this will not be exact*/
		float integratedProb = ((float)rand100) / ((float)randModHalf);
		int max_i_index = (int)roundf(gI.N) - 1;
		float fixGaussFac = gI.I[ max_i_index ];
		if (fixGaussFac <= 0.f)
			return 0.f;
		float retPt = 0.f;
		for (int i_int = 0; i_int < gI.N; i_int++) {
			float fixedGaussInt = gI.I[i_int] / fixGaussFac;
			if (fixedGaussInt >= integratedProb) {
				/*value loc match*/
				retPt = gI.X[i_int];
			}
		}
		return signpos ? retPt : -retPt;
	}
	s_2pt randGaus2D(const s_2pt& center, const s_gaussianInt& gI) {
		/*compute a random angle in the range [0,2PI)*/
		float ang = randAng();
		/*comput a random gaussian offset*/
		float dr = randGausPt(gI);
		/*convert these values to a vector*/
		s_2pt X = { dr * sinf(ang), dr * cosf(ang) };
		s_2pt offset = vecMath::add(X, center);
		return offset;
	}
	float randGausJitterAng(const s_gaussianInt& gI, float center_ang) {
		float randGauss = randGausPt(gI);/*this could be positive or negative*/
		/*the range must already be set in gI*/
		float new_ang = center_ang + randGauss;
		return Ang2PI(new_ang);
	}
	long ftol(float f) {
		return lroundf(f);
	}
	float ltof(long l) {
		return (float)l;
	}
}
namespace arrMath {
	void put(float v, int i, float ar[], int n) {
		if (i >= n)
			return;
		for (int ar_i = n-2; ar_i >= i; ar_i--) {
			ar[ar_i + 1] = ar[ar_i];
		}
		ar[i] = v;
	}
	void put(int v, int i, int ar[], int n) {
		if (i >= n)
			return;
		for (int ar_i = n - 2; ar_i >= i; ar_i--) {
			ar[ar_i + 1] = ar[ar_i];
		}
		ar[i] = v;
	}
	int getFirstBelow(float v, float ar[], int n, int i_start) {
		int found_i = -1;
		for (int i = i_start; i < n; i++) {
			if (ar[i] > v) {
				found_i = i;
				break;
			}
		}
		return (found_i >= 0) ? found_i : n;
	}
}
namespace vecMath {
	s_2pt add(const s_2pt& v1, const s_2pt& v2) {
		s_2pt v = { v1.x0 + v2.x0, v1.x1 + v2.x1 };
		return v;
	}
	s_2pt_i add(const s_2pt_i& v1, const s_2pt_i& v2) {
		s_2pt_i v = { v1.x0 + v2.x0, v1.x1 + v2.x1 };
		return v;
	}
	s_2pt add(const s_2pt& v1, const s_2pt& v2, const s_2pt& v3) {
		s_2pt v = add(v1, v2);
		v.x0 += v3.x0;
		v.x1 += v3.x1;
		return v;
	}
	s_2pt v12(const s_2pt& v1, const s_2pt& v2) {
		s_2pt v = { v2.x0 - v1.x0, v2.x1 - v1.x1 };
		return v;
	}

	Tup3 v12(const Tup3& v1, const Tup3& v2) {
		Tup3 v = { v2.x - v1.x, v2.y - v1.y, v2.z - v1.z };
		return v;
	}

	float len(const s_2pt& v) {
		return sqrtf(v.x0*v.x0 + v.x1*v.x1);
	}
	float len(const Tup3& v) {
		return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
	}
	float dist(const s_2pt& v1, const s_2pt& v2) {
		s_2pt v = v12(v1, v2);
		return len(v);
	}
	float dist(const Tup3& v1, const Tup3& v2) {
		Tup3 v = v12(v1, v2);
		return len(v);
	}
	float dot(const s_2pt& v1, const s_2pt& v2) {
		return (v1.x0*v2.x0 + v1.x1*v2.x1);
	}
	s_2pt mul(float a, const s_2pt& vec) {
		s_2pt v1 = { a * vec.x0, a * vec.x1 };
		return v1;
	}
	s_2pt perpUR(s_2pt& vec) {
		s_2pt vperp = { -vec.x1, vec.x0 };
		return vperp;
	}
	s_2pt perpUL(s_2pt& vec) {
		s_2pt vperp = { vec.x1, -vec.x0 };
		return vperp;
	}
	void  setBasis(float angRad, s_2pt& basisU0, s_2pt& basisU1) {
		basisU0.x0 = cosf(angRad);
		basisU0.x1 = sinf(angRad);
		basisU1.x0 = -sinf(angRad);
		basisU1.x1 = cosf(angRad);
	}
	void revBasis(const s_2pt& basisU0, const s_2pt& basisU1, s_2pt& revBasisU0, s_2pt& revBasisU1) {
		revBasisU0.x0 = basisU1.x1;
		revBasisU0.x1 = -basisU0.x1;

		revBasisU1.x0 = -basisU1.x0;
		revBasisU1.x1 = basisU0.x0;
	}
	s_2pt convBasis(const s_2pt& basisU0, const s_2pt& basisU1, const s_2pt& vecInBasis) {
		s_2pt v0 = mul(vecInBasis.x0, basisU0);
		s_2pt v1 = mul(vecInBasis.x1, basisU1);
		return add(v0, v1);
	}
	bool inCircle(float r, const s_2pt& center, const s_2pt& pt) {
		float d = dist(center, pt);
		return d <= r;
	}
	float distCircleArc(float r, const s_2pt& center, const s_2pt& pt) {
		float distance = dist(center, pt);
		return distance-r;
	}
	float distLineGivenPerp(const s_2pt& line_pt, const s_2pt& line_perp, const s_2pt& pt) {
		s_2pt vec = v12(line_pt, pt);
		return dot(vec, line_perp);
	}
}

namespace imgMath {
	unsigned char add(unsigned char c0, unsigned char c1) {
		int i0 = (int)c0;
		int i1 = (int)c1;
		int isum = i0 + i1;
		if (isum > 0xFF)
			isum = 0xFF;
		return static_cast<unsigned char>(isum);
	}
	s_rgba convToRGBA(float r, float g, float b) {
		if (r > 255.f)
			r = 255.f;
		if (r < 0.f)
			r = 0.f;
		if (g > 255.f)
			g = 255.f;
		if (g < 0.f)
			g = 0.f;
		if (b > 255.f)
			b = 255.f;
		if (b < 0.f)
			b = 0.f;
		s_rgba rgba = { (unsigned char)r, (unsigned char)g, (unsigned char)b, 0xFF };
		return rgba;
	}
	s_rgb convToRGB(float r, float g, float b) {
		if (r > 255.f)
			r = 255.f;
		if (r < 0.f)
			r = 0.f;
		if (g > 255.f)
			g = 255.f;
		if (g < 0.f)
			g = 0.f;
		if (b > 255.f)
			b = 255.f;
		if (b < 0.f)
			b = 0.f;
		s_rgb rgb = { (unsigned char)r, (unsigned char)g, (unsigned char)b };
		return rgb;
	}
	s_rgb mulIntensity(const s_rgb& rgb, float intensity) {
		Tup3 frgb = rgbToTup3(rgb);
		frgb *= intensity;
		return tup3ToRgb(frgb);
	}
	void IncRGB(s_rgb& rgb0, const s_rgb& rgb1) {
		rgb0.r = add(rgb0.r, rgb1.r);
		rgb0.g = add(rgb0.g, rgb1.g);
		rgb0.b = add(rgb0.b, rgb1.b);
	}
	s_rgb gradRGB(const s_rgb& rgb0, const s_rgb& rgb1, float relI) {
		s_rgb rgb_end = mulIntensity(rgb1, relI);
		float rev_relI = 1.f - relI;
		s_rgb rgb_start = mulIntensity(rgb0, rev_relI);
		s_rgb col;
		col.r = add(rgb_start.r,rgb_end.r);
		col.g = add(rgb_start.g,rgb_end.g);
		col.b = add(rgb_start.b,rgb_end.b);
		return col;
	}
	void limitRGB(s_rgb_f& rgb) {
		if (rgb.r > 255.f)
			rgb.r = 255.f;
		if (rgb.r < 0.f)
			rgb.r = 0.f;
		if (rgb.g > 255.f)
			rgb.g = 255.f;
		if (rgb.g < 0.f)
			rgb.g = 0.f;
		if (rgb.b > 255.f)
			rgb.b = 255.f;
		if (rgb.b < 0.f)
			rgb.b = 0.f;
	}
	s_2pt_i convToVint(const s_2pt& vec) {
		s_2pt_i vi;
		vi.x0 = (long)floorf(vec.x0);
		vi.x1 = (long)floorf(vec.x1);
		return vi;
	}
	s_2pt convToVfloat(const s_2pt_i& vi) {
		s_2pt vec;
		vec.x0 = (float)vi.x0;
		vec.x1 = (float)vi.x1;
		return vec;
	}
	Tup3 rgbToTup3(const s_rgb& rgb) {
		Tup3 tup;
		tup.x = (float)rgb.r;
		tup.y = (float)rgb.g;
		tup.z = (float)rgb.b;
		return tup;
	}
	s_rgb tup3ToRgb(const Tup3& tup) {
		s_rgb rgb = convToRGB(tup.x, tup.y, tup.z);
		return rgb;
	}
	void RGBToFloats(const s_rgb& rgb_c, float rgb_f[]) {
		rgb_f[0] = (float)rgb_c.r;
		rgb_f[1] = (float)rgb_c.g;
		rgb_f[2] = (float)rgb_c.b;
	}
	void FloatsToRGB(const float rgb_f[], s_rgb& rgb_c) {
		rgb_c = convToRGB(rgb_f[0], rgb_f[1], rgb_f[2]);
	}
	s_2pt perpUR(s_2pt& vec) {
		s_2pt perp = { -vec.x1, vec.x0 };
		return perp;
	}
	void drawPoint(long i, long j, const s_rgba& col, Img* canvas) {
		if (i < 0 || j < 0)
			return;
		if (i >= canvas->getWidth() || j >= canvas->getHeight())
			return;
		canvas->SetRGBA(i, j, col);
	}
	void drawLine(long i_start, long j_start, long i_end, long j_end, const s_rgba& col, float thickness, Img* canvas) {
		s_2pt v = { (float)(i_end - i_start), (float)(j_end - j_start) };
		drawV(v, i_start, j_start, col, thickness, canvas);
	}
	void drawV(const s_2pt& v, long i, long j, const s_rgba& col, float thickness, Img* canvas)
	{
		long width = canvas->getWidth();
		long height = canvas->getHeight();
		float x = v.x0;
		float y = v.x1;
		if (fabs(x) <= 1 && fabs(y) <= 1) {
			return;
		}

		long x_max = (long)roundf(x);
		long x_absmax = abs(x_max);
		long y_max = (long)roundf(y);
		long y_absmax = abs(y_max);
		if (fabs(x) >= fabs(y)) {
			long sign_val = (x >= 0.f) ? 1 : -1;
			float slope = y / x;
			long x_i = 0;
			while (x_i <= x_absmax) {
				long x_i_ = x_i * sign_val;
				float y_cur = slope * ((float)x_i_);
				long y_j = (long)roundf(y_cur);
				drawPoint(x_i_ + i, y_j + j, col, canvas);
				x_i++;
			}
		}
		else {
			long sign_val = (y >= 0.f) ? 1 : -1;
			float slope = x / y;
			long y_j = 0;
			while (y_j <= y_absmax) {
				long y_j_ = sign_val * y_j;
				float x_cur = slope * ((float)y_j_);
				long x_i = (long)roundf(x_cur);
				drawPoint(x_i + i, y_j_ + j, col, canvas);
				y_j++;
			}
		}
	}
	void nineAveFromPtOffset(const s_2pt& R, const s_2pt& center, s_2pt_i& img_pt, float perc[]) {
		for (int i_perc = 0; i_perc < 9; i_perc++)
			perc[i_perc] = 0.f;
		s_2pt img_xy = vecMath::add(R, center);
		float pix_x = floorf(img_xy.x0);
		float pix_y = floorf(img_xy.x1);
		img_pt.x0 = (long)pix_x;
		img_pt.x1 = (long)pix_y;
		float in_square_x = img_xy.x0 - pix_x;
		float in_square_y = img_xy.x1 - pix_y;
		/*find distances to adjoining centers*/
		float dists[9];
		int d_i = 0;
		for (float y_d = -0.5f; y_d <= 1.5f; y_d += 1.f) {
			for (float x_d = -0.5f; x_d <= 1.5f; x_d += 1.f) {
				float dx = in_square_x - x_d;
				float dy = in_square_y - y_d;
				float max_dx = 0.5f - x_d;
				float max_dy = 0.5f - y_d;
				float max_dist = max_dx * max_dx + max_dy * max_dy;
				dists[d_i] = (max_dist > 0.00001f) ? (dx * dx + dy * dy) / max_dist : (dx * dx + dy * dy);/*4 is special*/
				d_i++;
			}
		}
		float max_center_offset_dist = 0.5f * 0.5f + 0.5f * 0.5f;
		dists[4] /= max_center_offset_dist;
		perc[4] = dists[4];
		float other_squares_perc_fact = 1.f - perc[4];
		float other_ave_denom = 0.f;
		for (int i = 0; i < 9; i++) {
			if (i != 4) {
				if (dists[i] > 1.f)
					perc[i] = 0.f;
				else {
					other_ave_denom += dists[i];
				}
			}
		}
		for (int i = 0; i < 9; i++) {
			if (i != 4) {
				perc[i] = dists[i] / other_ave_denom;
				perc[i] *= other_squares_perc_fact;
			}
			if (perc[i] > 1.f)
				perc[i] = 1.f;
			if (perc[i] < 0.f)
				perc[i] = 0.f;
		}
	}
	bool nineAveRGB(const s_2pt& R, const s_2pt& center, const Img* img, s_2pt_i& img_pt, s_rgb& retrgb) {
		retrgb.r = IMG_IMGBAK;
		retrgb.g = IMG_IMGBAK;
		retrgb.b = IMG_IMGBAK;
		float perc[9];
		nineAveFromPtOffset(R, center, img_pt, perc);
		if (img_pt.x0 < 0 || img_pt.x1 < 0)
			return false;
		int perc_i = 0;
		for (long i_y = -1; i_y < 3; i_y++) {
			for (long i_x = -1; i_x < 3; i_x++) {
				long img_indx = img->getIndex(img_pt.x0 - i_x, img_pt.x1 - i_y);
				if (img_indx >= 0) {
					s_rgb rgb = img->GetRGB(img_indx);
					s_rgb rgb_scaled = mulIntensity(rgb, perc[perc_i]);
					IncRGB(retrgb, rgb_scaled);
				}
				perc_i++;
			}
		}
		return true;
	}
}
namespace hexMath {
	bool inHex(float hexR, float hexRS, const s_2pt hexU[], const s_2pt& center, const s_2pt& pt, float padding) {
		float xdiff = pt.x0 - center.x0;
		float ydiff = pt.x1 - center.x1;
		float diff = sqrtf(xdiff * xdiff + ydiff * ydiff);
		if (diff > hexR)
			return false;
		s_2pt vpt = { xdiff, ydiff };
		float max_proj = 0.f;
		for (int i = 0; i < 6; i++) {
			float proj = vecMath::dot(vpt, hexU[i]);
			if (proj > max_proj)
				max_proj = proj;
		}
		bool inside = false;
		if (max_proj <= (hexRS + padding))
			inside = true;
		return inside;
	}
}
