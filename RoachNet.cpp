// RoachNet.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#include <iostream>
#ifndef TESTHEXEYE_C_H
#include "Test/TestHexEye_C.h"
#endif
#ifndef TESTHEXEYEIMG_C_H
#include "Test/TestHexEyeImg_C.h"
#endif
#ifndef CTARGAIMAGE_H
#include "FileIO/CTargaImage.h"
#endif
#ifndef TESTCOLOR_C_H
#include "Test/TestColor_C.h"
#endif
#ifndef TESTLUNA_C_H
#include "Test/TestLuna_C.h"
#endif
#ifndef TRAINL0_H
#include "Train/TrainL0.h"
#endif

int testHexEye();
int testHexEyeImg();
int testColor();
int testLuna();

void writeTGA(CTargaImage& tga_IO, std::string& f1, Img* img_);
int main()
{
    TrainL0 trainL;
    unsigned char errc = trainL.init();
    if (errc != ECODE_OK)
        return 1;
    errc=trainL.run();
    if (errc != ECODE_OK)
        return 1;
    trainL.release();
    return 0;
}
void writeTGA(CTargaImage& tga_IO, std::string& f1, Img* img_) {
    tga_IO.Open(img_->getImg(), (unsigned short)img_->getWidth(), (unsigned short)img_->getHeight(), false, IMAGE_RGB);
    tga_IO.Write(f1.c_str());
    tga_IO.Close();

}
int testLuna() {
    CTargaImage tga_imgIn;
    tga_imgIn.Init();
    std::string fIn("Test/Dat/baseStamp0.tga");
    tga_imgIn.Open(fIn.c_str());
    Img imgIn;
    imgIn.init(tga_imgIn.GetImage(), tga_imgIn.GetWidth(), tga_imgIn.GetHeight(), tga_imgIn.GetColorMode());

    CTargaImage tga_img;
    tga_img.Init();

    TestLuna_C testLuna;
    testLuna.init(&imgIn, 3.f);

    Img lunasImg;
    Img hexBaseImg;
    Img colImg;
    int arlen = 6;
    Img** img_luna = new Img * [arlen];
    for (int i = 0; i < arlen; i++)
        img_luna[i] = new Img;
    testLuna.render(&colImg, &lunasImg, img_luna);
    testLuna.renderBase(&hexBaseImg);
    Img* rawImgPtr = testLuna.getBaseImg();

    std::string f_raw("Test/Dat/Luna/testlunaRaw.tga");
    writeTGA(tga_img, f_raw, rawImgPtr);
    std::string f_h("Test/Dat/Luna/testlunaHexed.tga");
    writeTGA(tga_img, f_h, &hexBaseImg);
    std::string f_c("Test/Dat/Luna/testlunaCol.tga");
    writeTGA(tga_img, f_c, &colImg);
    std::string f0("Test/Dat/Luna/testlunasImg.tga");
    writeTGA(tga_img, f0, &lunasImg);
    std::string f1("Test/Dat/Luna/testlunaImg_1.tga");
    writeTGA(tga_img, f1, img_luna[0]);
    std::string f2("Test/Dat/Luna/testlunaImg_2.tga");
    writeTGA(tga_img, f2, img_luna[1]);
    std::string f3("Test/Dat/Luna/testlunaImg_3.tga");
    writeTGA(tga_img, f3, img_luna[2]);
    std::string f4("Test/Dat/Luna/testlunaImg_4.tga");
    writeTGA(tga_img, f4, img_luna[3]);
    std::string f5("Test/Dat/Luna/testlunaImg_5.tga");
    writeTGA(tga_img, f5, img_luna[4]);
    std::string f6("Test/Dat/Luna/testlunaImg_6.tga");
    writeTGA(tga_img, f6, img_luna[5]);





    tga_img.Release();

    for (int i = 0; i < arlen; i++) {
        if (img_luna[i] != NULL) {
            img_luna[i]->release();
            delete img_luna[i];
        }
        img_luna[i] = NULL;
    }
    delete[] img_luna;
    img_luna = NULL;
    lunasImg.release();

    testLuna.release();


    tga_imgIn.Close();
    tga_imgIn.Release();
    std::cout << "Finished generating Test/Dat/ files...\n";
    return 0;
}
int testColor() {
    CTargaImage tga_imgIn;
    tga_imgIn.Init();
    std::string fIn("Test/Dat/baseImg0.tga");
    tga_imgIn.Open(fIn.c_str());
    Img imgIn;
    imgIn.init(tga_imgIn.GetImage(), tga_imgIn.GetWidth(), tga_imgIn.GetHeight(), tga_imgIn.GetColorMode());

    CTargaImage tga_img;
    tga_img.Init();

    int num_cols = 6;
    TestColor_C testCol;
    testCol.init(&imgIn, 7.f, 6, num_cols);

    int arlen = num_cols + 1;
    Img** img_cols = new Img * [arlen];
    for (int i = 0; i < arlen; i++)
        img_cols[i] = new Img;
    testCol.render(img_cols);
    std::string f1("Test/Dat/testColImg_0.tga");
    writeTGA(tga_img, f1, img_cols[0]);
    std::string f2("Test/Dat/testColImg_1.tga");
    writeTGA(tga_img, f2, img_cols[1]);
    std::string f3("Test/Dat/testColImg_2.tga");
    writeTGA(tga_img, f3, img_cols[2]);
    std::string f4("Test/Dat/testColImg_3.tga");
    writeTGA(tga_img, f4, img_cols[3]);
    std::string f5("Test/Dat/testColImg_4.tga");
    writeTGA(tga_img, f5, img_cols[4]);
    std::string f6("Test/Dat/testColImg_5.tga");
    writeTGA(tga_img, f6, img_cols[5]);
    std::string f7("test/Dat/testColImg_6.tga");
    writeTGA(tga_img, f7, img_cols[6]);
    tga_img.Release();

    for (int i = 0; i < arlen; i++) {
        if (img_cols[i] != NULL) {
            img_cols[i]->release();
            delete img_cols[i];
        }
        img_cols[i] = NULL;
    }
    delete[] img_cols;
    img_cols = NULL;

    testCol.release();


    tga_imgIn.Close();
    tga_imgIn.Release();
    std::cout << "Finished generating Test/Dat/ files...\n";
    return 0;
}
int testHexEyeImg() {
    CTargaImage tga_imgIn;
    tga_imgIn.Init();
    std::string fIn("Test/Dat/baseImg0.tga");
    tga_imgIn.Open(fIn.c_str());
    Img imgIn;
    imgIn.init(tga_imgIn.GetImage(), tga_imgIn.GetWidth(), tga_imgIn.GetHeight(), tga_imgIn.GetColorMode());

    CTargaImage tga_img;
    tga_img.Init();

    TestHexEyeImg_C testHexEyeImg;
    testHexEyeImg.init(&imgIn, 7.f, 6);
    Img* img_eye0 = new Img;
    testHexEyeImg.genImg(img_eye0, 1);
    std::string f1("Test/Dat/testHexEyeImg_l1.tga");
    writeTGA(tga_img, f1, img_eye0);
    img_eye0->release();
    delete img_eye0;
    testHexEyeImg.release();

    tga_img.Release();

    tga_imgIn.Close();
    tga_imgIn.Release();
    return 0;
}
int testHexEye() 
{
    CTargaImage tga_img;
    tga_img.Init();
    TestHexEye_C testHexEye;
    testHexEye.init(28.f, 3);
    Img* img_levels = new Img;
    Img* img_web = new Img;
    Img* img_lower = new Img;
    testHexEye.run(img_levels, img_web, img_lower);
    std::string f1("Test/Dat/testHexEye_levels.tga");
    std::string f2("Test/Dat/testHexEye_web.tga");
    std::string f3("Test/Dat/testHexEye_lower.tga");
    tga_img.Open(img_levels->getImg(), (unsigned short)img_levels->getWidth(), (unsigned short)img_levels->getHeight(), false, IMAGE_RGB);
    tga_img.Write(f1.c_str());
    tga_img.Close();
    tga_img.Open(img_web->getImg(), (unsigned short)img_web->getWidth(), (unsigned short)img_web->getHeight(), false, IMAGE_RGB);
    tga_img.Write(f2.c_str());
    tga_img.Close();
    tga_img.Open(img_lower->getImg(), (unsigned short)img_lower->getWidth(), (unsigned short)img_lower->getHeight(), false, IMAGE_RGB);
    tga_img.Write(f3.c_str());
    tga_img.Close();

    img_web->release();
    delete img_web;
    img_levels->release();
    delete img_levels;
    img_lower->release();
    delete img_lower;
    testHexEye.release();
    tga_img.Release();
    std::cout << "Finished generating Test/Dat/ files...\n";
    return 0;
}
// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
