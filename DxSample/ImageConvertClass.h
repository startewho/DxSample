#pragma once

#include <turbojpeg.h>
#include "Common.h"

class ImageConvertClass
{
public:
	bool ConvertDIBToJPG(unsigned char* buffer, int nWidth, int nHeight, wchar_t* outFile);
	bool ConvertDIBToPNG(unsigned char* buffer, int nWidth, int nHeight, wchar_t* outFile);
	bool ConvertDIBToBMP(unsigned char* buffer, int nWidth, int nHeight, wchar_t* outFile);
	ImageConvertClass();
	~ImageConvertClass();
};

