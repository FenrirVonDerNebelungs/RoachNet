#pragma once
#ifndef PARSETXT_H
#define PARSETXT_H
#ifndef BASE_H
#include "../Base/Base.h"
#endif

#define PARSETXT_MAXAR 1800
#define PARSETXT_FILENUMEXTLEN 4
using namespace std;

/*struct s_datLine {
	float v[PARSETXT_MAXAR];
	int n;
};*/
class s_datLine {
public:
	s_datLine();
	s_datLine(int i_maxar);
	~s_datLine();
	float* v;
	int n;
	int maxar;
};
namespace n_datLine {
	void clear(s_datLine& dl);
}

class ParseTxt : public Base {
public:
	ParseTxt();
	~ParseTxt();

	unsigned char init(
		const string& inFile, 
		const string& outFile);
	unsigned char init();
	void release();
	void setInFile(const string& inFile);
	void setOutFile(const string& outFile);

	int readCSV(s_datLine dat[], int maxSize=5000);
	unsigned char writeCSVHeader(std::string& headerStr);
	unsigned char writeCSVwithSpacer(int marker_i, const s_datLine dat[], int dat_size);
	unsigned char writeCSV(const s_datLine dat[], int dat_size);
protected:
	string m_inFile;
	string m_outFile;
	std::string m_header;


	string dumpFloatLine(const float* ar, int len);

	int readFloatLine(const string& str, float* ar);
};

namespace n_ParseTxt {
	unsigned char intToFixedLenStr(int val, int target_len, string& strout);
	inline unsigned char intToFileNameExt(int val, string& strout) {
		return intToFixedLenStr(val, PARSETXT_FILENUMEXTLEN, strout);
	}
	bool stripStringBeforeSuf(const std::string& sufstr, const std::string& instr, std::string& resstr);
}
#endif