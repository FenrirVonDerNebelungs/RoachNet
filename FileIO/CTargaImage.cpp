#include "CTargaImage.h"
using namespace std;

void n_tgaheader::headerToChararray(const tgaheader_t& th, char ar[]) {
	ar[0] = static_cast<char>(th.idLength);
	ar[1] = static_cast<char>(th.colorMapType);
	ar[2] = static_cast<char>(th.imageTypeCode);
	for (int i = 0; i < 5; i++)
		ar[3 + i] = static_cast<char>(th.colorMapSpec[i]);
	shortToChars(th.xOrigin, &(ar[8]));
	shortToChars(th.yOrigin, &(ar[10]));
	shortToChars(th.width, &(ar[12]));
	shortToChars(th.height, &(ar[14]));
	ar[16] = static_cast<char>(th.bpp);
	ar[17] = static_cast<char>(th.imageDesc);
}
void n_tgaheader::chararrayToHeader(const char ar[], tgaheader_t& th) {
	th.idLength = static_cast<unsigned char>(ar[0]);
	th.colorMapType = static_cast<unsigned char>(ar[1]);
	th.imageTypeCode = static_cast<unsigned char>(ar[2]);
	for (int i = 0; i < 5; i++)
		th.colorMapSpec[i] = static_cast<unsigned char>(ar[3 + i]);
	th.xOrigin = charsToShort(&(ar[8]));
	th.yOrigin = charsToShort(&(ar[10]));
	th.width = charsToShort(&(ar[12]));
	th.height = charsToShort(&(ar[14]));
	th.bpp = static_cast<unsigned char>(ar[16]);
	th.imageDesc = static_cast<unsigned char>(ar[17]);
}
void n_tgaheader::shortToChars(unsigned short s, char ar[]) {
	unsigned short lo = s & 0xFF;
	unsigned short hi = s & 0xFF00;
	unsigned short his = hi >> 8;
	ar[0] = static_cast<char>(lo);
	ar[1] = static_cast<char>(his);
}
unsigned short n_tgaheader::charsToShort(const char ar[]) {
	unsigned char lo_c = static_cast<unsigned char>(ar[0]);/*git rid of the sign before changing to short*/
	unsigned char hi_c = static_cast<unsigned char>(ar[1]);
	unsigned short lo = static_cast<unsigned short>(lo_c);
	unsigned short his = static_cast<unsigned short>(hi_c);
	unsigned short hi = his << 8;
	unsigned short s = hi | lo;
	return s;
}
CTargaImage::CTargaImage() :m_ImageDataIsOwned(false), pStream_in(NULL), pStream_out(NULL),
m_xOrigin(0), m_yOrigin(0), m_colorDepth(0x00), m_imageDataType(0x00), m_imageDataFormat(0x00),
m_imageDesc(0x00), m_pImageData(NULL),
m_width(0), m_height(0), m_colorMode(0), m_imageSize(0),
m_filename_len(0), m_IsBGR(false)
{
	m_filename[0] = '\0';
}
CTargaImage::~CTargaImage()
{
	;
}
unsigned char CTargaImage::Init()
{
	if (!IsStateNew())
		return ECODE_FAIL;
	SetStateInitialized();
	return ECODE_OK;
}
void CTargaImage::Release()
{
	if (IsStateOpen())
		Close();
	SetStateNew();
	return;
}
unsigned char CTargaImage::Open(unsigned char* pImageData,
	unsigned short width,
	unsigned short height,
	bool          IsBGR,
	unsigned char imageDataFormat,
	const char* filename)
{
	if (!IsStateInitialized())
		return ECODE_FAIL;
	if (IsStateOpen())
		return ECODE_ABORT;
	if (m_pImageData != NULL)
		return ECODE_ABORT;
	if (pImageData == NULL)
		return ECODE_FAIL;
	if (!(IsGoodImageDim(width)))
		return ECODE_FAIL;
	if (!(IsGoodImageDim(height)))
		return ECODE_FAIL;
	if (width < 2 || height < 2)
		return ECODE_FAIL;
	if (!(imageDataFormat == IMAGE_RGB || imageDataFormat == IMAGE_RGBA))
		return ECODE_FAIL;
	m_pImageData = pImageData;
	m_ImageDataIsOwned = false;
	m_width = width;
	m_height = height;
	m_IsBGR = IsBGR;
	m_imageDataFormat = imageDataFormat;
	if (filename != NULL) {
		for (unsigned short i = 0; i < FILEIO_MAXFILENAMELEN; i++) {
			m_filename[i] = filename[i];
			if (filename[i] == '\0')
				break;
			m_filename_len++;
		}
	}
	SetStateOpen();
	return ECODE_OK;
}
unsigned char CTargaImage::Open(const char* filename)
{
	if (IsStateOpen())
		return ECODE_ABORT;
	if (!IsStateInitialized())
		return ECODE_ABORT;
	std::string inFile(filename);
	//pStream_in = fopen(filename, "rb");
	pStream_in = new ifstream(inFile, ios::in|ios::binary|ios::ate);/*ate puts pointer at end of file which makes tellg give the size of the file when called*/
	if (pStream_in == NULL)
		return ECODE_USERERR_ABORT;
	tgaheader_t tgaHeader;

	// read the TGA header
	int tgaheader_size = (int)sizeof(tgaheader_t);
	int infile_size = (int)pStream_in->tellg();
	if (tgaheader_size >= infile_size) {
		cout << "\n ERROR file too small, empty image or lack of tga header\n";
		pStream_in->close();
		return ECODE_FAIL;
	}
	char* tgaheader_cArray = new char[tgaheader_size];
	pStream_in->seekg(0, ios::beg);
	pStream_in->read(tgaheader_cArray, tgaheader_size);/*assume this advances the location 'g'  could check with tellg*/
	//fread(&tgaHeader, 1, sizeof(tgaheader_t), pStream_in);
	n_tgaheader::chararrayToHeader(tgaheader_cArray, tgaHeader);
	delete[] tgaheader_cArray;
	// see if the image type is one that we support (RGB, RGB GRAYSCALE,)  cut out the RLE stuff for now
	if (((tgaHeader.imageTypeCode != TGA_RGB) && (tgaHeader.imageTypeCode != TGA_GRAYSCALE)) ||
		tgaHeader.colorMapType != 0)
	{
		if (pStream_in) {
			pStream_in->close();
			//fclose(pStream_in);
		}
		return ECODE_FAIL;
	}

	// get image width and height
	m_width = tgaHeader.width;
	m_height = tgaHeader.height;
	// colormode -> 3 = BGR, 4 = BGRA
	int colorMode = tgaHeader.bpp / 8;
	m_colorMode = colorMode;
	// we don't handle less than 24 bit or currently >32 bit
	// also don't try for images if they are too big
	if (((colorMode < 3) || (colorMode > 4)) || (!IsGoodImageDim(m_width)) || (!IsGoodImageDim(m_height)))
	{
		if (pStream_in) {
			pStream_in->close();
			//fclose(pStream_in);
		}
		return ECODE_FAIL;
	}

	m_imageSize = m_width * m_height * colorMode;

	// allocate memory for TGA image data
	m_pImageData = new unsigned char[m_imageSize];
	if (m_pImageData == NULL) {
		return ECODE_FAIL;
	}
	m_ImageDataIsOwned = true;

	// skip past the id if there is one
	if (tgaHeader.idLength > 0) {
		int header_idLength = static_cast<int>(tgaHeader.idLength);
		pStream_in->seekg(header_idLength, ios::cur);
		//fseek(pStream_in, SEEK_CUR, tgaHeader.idLength);
	}

	// read image data
	char* signed_pImageDate = reinterpret_cast<char*>(m_pImageData);//both should be 1 byte so this shouldn't make a difference
	int _imageSize = static_cast<int>(m_imageSize);/*read uses int*/
	pStream_in->read(signed_pImageDate, _imageSize);
	//fread(m_pImageData, 1, m_imageSize, pStream_in);
	//IGNORE IF RLE compressed image for now to correct if needed see code in book

	if (pStream_in) {
		pStream_in->close();
		delete pStream_in;
		pStream_in = NULL;
		//int err = fclose(pStream_in);
	}
	switch (tgaHeader.imageTypeCode)
	{
	case TGA_RGB:
	case TGA_RGB_RLE:
		if (3 == colorMode)
		{
			m_imageDataFormat = IMAGE_RGB;
			m_imageDataType = IMAGE_DATA_UNSIGNED_BYTE;
			m_colorDepth = 24;
		}
		else
		{
			m_imageDataFormat = IMAGE_RGBA;
			m_imageDataType = IMAGE_DATA_UNSIGNED_BYTE;
			m_colorDepth = 32;
		}
		break;

	case TGA_GRAYSCALE:
	case TGA_GRAYSCALE_RLE:
		m_imageDataFormat = IMAGE_LUMINANCE;
		m_imageDataType = IMAGE_DATA_UNSIGNED_BYTE;
		m_colorDepth = 8;
		break;
	}
	m_imageDesc = tgaHeader.imageDesc;

	// swap the red and blue components in the image data
	//SwapRedBlue(); //microsoft libraries for D2D1 use bgr instead of rgb
	m_IsBGR = true;
	if (m_pImageData == NULL)
		return ECODE_FAIL;
	SetStateOpen();
	return ECODE_OK;
}
void CTargaImage::Close()
{
	m_filename_len = 0;
	if ((m_pImageData != NULL) && m_ImageDataIsOwned) {
		delete[] m_pImageData;
	}
	m_pImageData = NULL;
	m_width = 0;
	m_height = 0;
	m_imageSize = 0;
	m_IsBGR = false;
	SetStateClosed();
}
bool CTargaImage::Load(const char* filename) {
	return (RetOk(Open(filename)));
}
unsigned char CTargaImage::Write(const char* filename, unsigned char bpp) {
	if (!IsStateOpen())
		return ECODE_ABORT;
	const char* cur_filename = filename;
	if ((cur_filename == NULL) && (m_filename_len > 0))
		cur_filename = m_filename;

	if (bpp == 0) {
		if (m_imageDataFormat == IMAGE_RGB)
			bpp = 3;
		else if (m_imageDataFormat == IMAGE_RGBA)
			bpp = 4;
	}
	if (bpp != 3 && bpp != 4)
		return ECODE_FAIL;
	if (m_IsBGR) {
		if (Err(SwapRedBlue()))
			return ECODE_FAIL;
		m_IsBGR = false;
	}
	//set the size of the image 
	m_imageSize = bpp * m_width * m_height; //this is done before the multiplication by 8

	pStream_out = new ofstream(cur_filename, ios::binary);
	//pStream_out = fopen(cur_filename, "wxb");
	if (pStream_out==NULL) {
		SetFailErr(FILEIO_ERR_FOPENFAIL);
		return m_obj_err;
	}

	tgaheader_t tgaHeader;
	Zero_Memory(&tgaHeader, sizeof(tgaheader_t));
	//fill the tga header
	tgaHeader.idLength = 0;
	tgaHeader.colorMapType = 0;
	tgaHeader.imageTypeCode = TGA_RGB;
	//shouldnt need the color map spec ignored when color map type is zero
	tgaHeader.xOrigin = m_xOrigin;
	tgaHeader.yOrigin = m_yOrigin;
	tgaHeader.width = m_width;
	tgaHeader.height = m_height;
	tgaHeader.bpp = bpp * 8; //3*8 for rgb , 4*8 for rgba
	tgaHeader.imageDesc = m_imageDesc; //just to be quick same and return the same values as was originally here
	int tgaheader_size = (int)sizeof(tgaheader_t);
	char* tgaheader_cArray = new char[tgaheader_size];
	n_tgaheader::headerToChararray(tgaHeader, tgaheader_cArray);
	std::cout << "start pos:" << pStream_out->tellp() << '\n';
	pStream_out->write(tgaheader_cArray, tgaheader_size);
	std::cout << "after write of header: " << pStream_out->tellp() << '\n';
	//size_t n_written = fwrite(&tgaHeader, 1, sizeof(tgaheader_t), pStream_out);
	char* signed_pImageData = reinterpret_cast<char*>(m_pImageData);
	//assume the write advances the file g position
	pStream_out->write(signed_pImageData, static_cast<int>(m_imageSize));
	std::cout << "after write of image data: " << pStream_out->tellp() << '\n';
	//n_written = fwrite(m_pImageData, 1, m_imageSize, pStream_out);
	pStream_out->close();
	delete pStream_out;
	pStream_out = NULL;
	//fclose(pStream_out);
	return ECODE_OK;
}
bool CTargaImage::IsGoodImageDim(unsigned short dim)
{
	if (dim < FILEIO_DEF_MINIMAGEDIM)
		return false;
	if (dim > FILEIO_DEF_MAXIMAGEDIM)
		return false;
	return true;
}
unsigned char CTargaImage::SwapRedBlue()
{
	if (m_pImageData == NULL)
		return ECODE_FAIL;
	unsigned long colorMode = static_cast<unsigned long>(m_colorMode);
	unsigned long maxI = m_imageSize - colorMode;
	for (unsigned long i = 0; i <= maxI; i += colorMode) {
		unsigned char bHold = m_pImageData[i];
		m_pImageData[i] = m_pImageData[i + 2];
		m_pImageData[i + 2] = bHold;
	}
	return ECODE_OK;
}
unsigned char CTargaImage::ConvertRGBAtoRGB()
{
	if (m_imageDataFormat == IMAGE_RGB)
		return ECODE_OK;
	if (m_pImageData == NULL || m_imageDataFormat != IMAGE_RGBA || !m_ImageDataIsOwned)
		return ECODE_FAIL;
	unsigned long imageSize_rgb = m_height * m_width * 3;
	unsigned char* pimage_rgb = new unsigned char[imageSize_rgb];
	Zero_Memory(pimage_rgb, imageSize_rgb);

	for (unsigned short row = 0; row < m_height; row++) {
		for (unsigned short col = 0; col < m_width; col++) {
			unsigned long i_rgba = (row * m_width + col) * 4;
			unsigned long i_rgb = (row * m_width + col) * 3;
			pimage_rgb[i_rgb] = m_pImageData[i_rgba];
			pimage_rgb[i_rgb + 1] = m_pImageData[i_rgba + 1];
			pimage_rgb[i_rgb + 2] = m_pImageData[i_rgba + 1];
		}
	}

	delete[] m_pImageData;
	m_pImageData = pimage_rgb;
	m_imageSize = imageSize_rgb;
	m_colorDepth = 24;
	m_colorMode = 3;
	return true;
}