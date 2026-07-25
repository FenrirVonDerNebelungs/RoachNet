#pragma once
#ifndef TEST_H
#define TEST_H

#ifndef TESTHEXEYE_C_H
#include "TestHexEye.h"
#endif

class Test : public Base {
public:
	Test();
	~Test();

	unsigned char init();

private:
	TestHexEye* m_testHexEye;
};
#endif