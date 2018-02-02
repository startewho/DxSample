#include "ImageConvertClass.h"
#include <jpeglib.h>






bool ImageConvertClass::ConvertDIBToJPG(unsigned char * buffer, int nWidth, int nHeight, wchar_t * outFile)
{
	jpeg_compress_struct jpegInfo;
	jpeg_error_mgr errorMgr;
	unsigned char* pJpegBuffer=NULL;
	unsigned long outSize;
	jpegInfo.err = jpeg_std_error(&errorMgr);
	//注册失败的回调函数
	//toWriteInfo.err->error_exit = error_exit;
	jpeg_create_compress(&jpegInfo);
	//保存压缩后的图片
	//FILE* fp = NULL;
	//_wfopen_s(&fp, L"c:\\output.jpg", L"wb+");
	//jpeg_stdio_dest(&toWriteInfo, fp);
	//确定要用于输出压缩的jpeg的数据空间
	jpeg_mem_dest(&jpegInfo, &pJpegBuffer, &outSize);
	jpegInfo.image_width = nWidth;
	jpegInfo.image_height = nHeight;
	//toWriteInfo.jpeg_width = nWidth / 2;
	//toWriteInfo.jpeg_height = nHeight / 2;
	jpegInfo.input_components = 4;// 在此为1,表示灰度图， 如果是彩色位图，则为4
	jpegInfo.in_color_space = JCS_EXT_BGRA; //JCS_GRAYSCALE表示灰度图，JCS_RGB表示彩色图像 
	jpeg_set_defaults(&jpegInfo);
	jpeg_set_quality(&jpegInfo, 100, TRUE);	//设置压缩质量100表示100%
	jpeg_start_compress(&jpegInfo, TRUE);
	int nRowStride = nWidth * 4;	// 如果不是索引图,此处需要乘以4
	JSAMPROW row_pointer[1];	// 一行位图
	while (jpegInfo.next_scanline < jpegInfo.image_height)
	{
		row_pointer[0] = &buffer[jpegInfo.next_scanline*nRowStride];//
		jpeg_write_scanlines(&jpegInfo, row_pointer, 1);
	}

	jpeg_finish_compress(&jpegInfo);


	HANDLE file;
	DWORD write;

	if (PathFileExists(outFile))
	{
		DeleteFile(outFile);
	}

	file = CreateFile(outFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);  //Sets up the new bmp to be written to


	WriteFile(file, pJpegBuffer, outSize, &write, NULL);


	CloseHandle(file);


	jpeg_destroy_compress(&jpegInfo);
	

	return false;
}

ImageConvertClass::ImageConvertClass()
{
}


ImageConvertClass::~ImageConvertClass()
{
}
