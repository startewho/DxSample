#pragma once

#include <turbojpeg.h>
#include "Common.h"

class ImageConvertClass
{
public:
	bool ConvertDIBToJPG(unsigned char* buffer, int nWidth, int nHeight, wchar_t* outFile);
	ImageConvertClass();
	~ImageConvertClass();
};
