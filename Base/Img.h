#pragma once
#ifndef IMG_H
#define IMG_H
#ifndef BASE_H
#include "Base.h"
#endif


class Img {
public:
	Img();
	~Img();

	virtual unsigned char init(const Img& other);
	virtual unsigned char init(const unsigned char img[], long width, long height, long colorMode = 4);/*owns the image*/
	unsigned char initNoOwn(unsigned char img[], long width, long height, long colorMode = 4);
	virtual unsigned char init(long width, long height, long colorMode = 4);
	void   clearToChar(unsigned char cval);
	void release();

	inline bool sameKind(const Img& other) { return (this->m_width == other.getWidth() && this->m_height == other.getHeight() && this->getColorMode() == other.getColorMode()); }

	s_rgba GetRGBA(long x_i, long y_i) const;
	s_rgb  GetRGB(long x_i, long y_i) const;
	s_rgba GetRGBA(long index) const;
	s_rgb  GetRGB(long index) const;
	uint32_t GetCol(long index);
	uint32_t GetColRGBA(long index);
	uint32_t GetColRGB(long index);
	uint32_t GetColRGB(long x_i, long y_i);
	void   SetRGBA(long x_i, long y_i, const s_rgba& rgba);
	void   SetRGB(long x_i, long y_i, const s_rgb& rgb);
	void   SetRGBA(long index, const s_rgba& rgba);
	void   SetRGB(long index, const s_rgb& rgb);
	void   AddRGBA(long index, const s_rgba& rgba);
	void   AddRGB(long index, const s_rgb& rgb, float alpha=0.5f);
	void   SetCol(long index, uint32_t col);
	void   SetColRGBA(long index, uint32_t col);
	void   SetColRGB(long index, uint32_t col);
	void   SetColRGB(long x_i, long y_i, uint32_t col);
	void   AddCol(long index, uint32_t col);

	unsigned char rotate(float ang);
	unsigned char translate(const s_2pt& dr);
	unsigned char PrintMaskedImg(long x_i, long y_j, const Img& pImg, const s_rgb& rgb);
	unsigned char PrintSquare(long x_i, long y_j, int size, s_rgba& rgba);
	unsigned char DrawLine(s_2pt_i& pt0, s_2pt_i& pt1, s_rgb& rgba);
	unsigned char DrawLineGrad(s_2pt_i& pt0, s_2pt_i& pt1, s_rgb& rgb1, s_rgb& rgb2);

	inline s_2pt_i getCenter() { s_2pt_i center = { m_width / 2L, m_height / 2L }; return center; }
	inline long getWidth() const { return m_width; }
	inline long getHeight() const { return m_height; }
	inline unsigned char* getImg() { return m_img; }
	inline long getIndex(long i, long j) const{ long index = j * m_width + i; return (index < m_pixSize) ? index : -1; }
	unsigned char setChar(long x, long y, unsigned char pt);
	inline unsigned char setChar(long index, unsigned char pt) { m_img[index] = pt; return ECODE_OK; }
	inline unsigned char getChar(long index) const { return m_img[index]; }
	inline long getColorMode() const { return m_colorMode; }
	inline long getMaxIndex() const { return m_colorMode * m_pixSize; }

	bool inImg(long x_i, long y_i);

protected:
	bool m_notOwned;
	unsigned char* m_img;
	long m_colorMode;
	long m_width;
	long m_height;
	long m_pixSize;

	uint32_t rgbaToUint(s_rgba& rgba);
	uint32_t rgbToUint(s_rgb& rgb);
	s_rgba UintToRGBA(uint32_t col);
	s_rgb  UintToRGB(uint32_t col);


};
#endif
