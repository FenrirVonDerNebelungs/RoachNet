#pragma once
#ifndef TRAINL0_H
#define TRAINL0_H

#ifndef TRAINL0STAMP_H
#include "TrainL0Stamp.h"
#endif

const float TRAINL0STAMP_max_train_stamps = 2.f;

class TrainL0 : public Base {
public:
	TrainL0();
	~TrainL0();
	unsigned char init();
	void release();
	unsigned char run();
private:
	/*owned*/
	TrainL0Stamp* m_trainStamp;
	Img* m_baseImg;/*dummy image with correct dimensions*/

	float m_r;
	float m_imgDim;
	long m_imgWH;
	float m_num_sig_stamps;
	long  m_color_mode;

	std::string m_stampDir;
	unsigned char runBakDir();
	unsigned char runStampDir(int stampNum);/*runs over all stamps in the directory and then dumps result in a NetBaseO file in that directory*/

};

#endif
