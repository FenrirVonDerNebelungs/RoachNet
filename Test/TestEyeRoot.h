#pragma once
#ifndef TESTEYEROOT_H
#define TESTEYEROOT_H

#ifndef EYE_H
#include "../Eye/Eye.h"
#endif

class TestEyeRoot : public Base {
public:
	TestEyeRoot();
	~TestEyeRoot();

	unsigned char init(
		s_Eye* seye
	);
	void release();

	unsigned char genImg(Img* hexed_img) {
		return genHexedImg(hexed_img);
	}/*hexed_img must exist but not be initalized*/
protected:


	unsigned char genHexedImg(Img* hexed_img);
};
#endif