#ifndef TEST_H
#include "Test/Test.h"
#endif
// RoachNet.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#include <iostream>

//void writeTGA(CTargaImage& tga_IO, std::string& f1, Img* img_);
int main()
{
	Test test;
    test.init();
    test.runWrite();
    test.release();
	std::cout << "Hello World!\n";
    return 0;
}
/*void writeTGA(CTargaImage& tga_IO, std::string& f1, Img* img_) {
    tga_IO.Open(img_->getImg(), (unsigned short)img_->getWidth(), (unsigned short)img_->getHeight(), false, IMAGE_RGB);
    tga_IO.Write(f1.c_str());
    tga_IO.Close();

}*/

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
