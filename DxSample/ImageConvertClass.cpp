#include "ImageConvertClass.h"




bool DeleteExsitFile(wchar_t * &outFile)
{
	if (PathFileExists(outFile))
	{
		DeleteFile(outFile);
	}

	return true;
}


bool ImageConvertClass::ConvertDIBToJPG(unsigned char * buffer, int nWidth, int nHeight, wchar_t * outFile)
{
	jpeg_compress_struct jpegInfo;

	memset(&jpegInfo, 0, sizeof(jpeg_compress_struct));
	
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

bool ImageConvertClass::ConvertDIBToPNG(unsigned char * buffer, int nWidth, int nHeight, wchar_t * outFile)
{

	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;

	const int pixel_size = 4;
	const int depth = 8;
	const int bytesPerPixel = sizeof(unsigned char) * pixel_size;

	FILE *inputf;

	if (PathFileExists(outFile))
	{
		DeleteFile(outFile);
	}

	 _wfopen_s(&inputf,outFile, L"wb");

	png_byte ** row_pointers = NULL;  

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	png_init_io(png_ptr, inputf);
	
	info_ptr = png_create_info_struct(png_ptr);

	

	png_set_IHDR(png_ptr,
		info_ptr,
		nWidth,
		nHeight,
		depth,
		PNG_COLOR_TYPE_RGB_ALPHA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);

	png_write_info(png_ptr, info_ptr);

	row_pointers = static_cast<png_byte**>(png_malloc(png_ptr, nHeight * sizeof(png_byte *)));
	for (int y = 0; y < nHeight; ++y)
	{
		png_byte *row = static_cast<png_byte*>(png_malloc(png_ptr, bytesPerPixel * nWidth));
		row_pointers[y] = row;

		//row_pointers[y] = buffer + y * nWidth * 4;
		for ( int x = 0; x < nWidth; ++x)
		{
			//pixel_t * pixel = pixel_at (bitmap, x, y);

			unsigned char* b = buffer + y * nWidth * 4 + x * 4 + 0;
			unsigned char* g = buffer + y * nWidth * 4 + x * 4 + 1;
			unsigned char* r = buffer + y * nWidth * 4 + x * 4 + 2;
			unsigned char* a = buffer + y * nWidth * 4 + x * 4 + 3;

			*row++ = *r;
			*row++ = *g;
			*row++ = *b;
			*row++ = *a;



		}
	}

	
	

	png_set_rows(png_ptr, info_ptr, row_pointers);

	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_INVERT_ALPHA, NULL);

	png_write_end(png_ptr, NULL);

	
	for (int y = 0; y < nHeight; y++) {
		png_free(png_ptr, row_pointers[y]);
	}
	png_free(png_ptr, row_pointers);

	png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(inputf);
	

	return false;
}

bool ImageConvertClass::ConvertDIBToBMP(unsigned char * buffer, int nWidth, int nHeight, wchar_t * outFile)
{
	HANDLE file;
	DWORD write;

	if (PathFileExists(outFile))
	{
		DeleteFile(outFile);
	}

	file = CreateFile(outFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);  //Sets up the new bmp to be written to
	int bytesofScanLine, i, j;
	DWORD dwFileSize = nWidth * nHeight * 32;


	bytesofScanLine = (nWidth % 4 == 0) ? nWidth : ((nWidth + 3) / 4 * 4);


	BITMAPFILEHEADER bmfHeader;
	bmfHeader.bfType = 19778;

	bmfHeader.bfReserved1 = 0;
	bmfHeader.bfReserved2 = 0;
	bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);



	// fill the bmp file Infomation Header.
	BITMAPINFOHEADER bmiHeader;
	bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmiHeader.biWidth = nWidth;
	bmiHeader.biHeight = nHeight;
	bmiHeader.biPlanes = 1;
	bmiHeader.biBitCount = 32;
	bmiHeader.biCompression = BI_RGB;
	bmiHeader.biSizeImage = 640 * 480 * 4;
	bmiHeader.biXPelsPerMeter = 3780;
	bmiHeader.biYPelsPerMeter = 3780;
	bmiHeader.biClrUsed = 0;
	bmiHeader.biClrImportant = 0;


	WriteFile(file, &bmfHeader, sizeof(bmfHeader), &write, NULL);
	WriteFile(file, &bmiHeader, sizeof(bmiHeader), &write, NULL);

	/*
	数据是从头开始描述，还是从底部描述。这里要处理
	*/
	for (int i = 0; i < nHeight; i++)
	{
		WriteFile(file, buffer+(nHeight-i-1)*nWidth*4, nWidth*4, &write, NULL);
	}

	//WriteFile(file, buffer, bmiHeader.biSizeImage, &write, NULL);




	DWORD error = GetLastError();

	CloseHandle(file);


	return	 true;
}

bool ImageConvertClass::ConvertDIBToImage(ImageType imageType, unsigned char * buffer, int nWidth, int nHeight, wchar_t * outFile)
{
	bool result=false;
	switch (imageType)
	{
	default:
		break;
	case BMP:
		result=ConvertDIBToBMP(buffer, nWidth, nHeight, outFile);
		break;

	case PNG:
		result = ConvertDIBToPNG(buffer, nWidth, nHeight, outFile);
		break;

	case JPG:
		result = ConvertDIBToJPG(buffer, nWidth, nHeight, outFile);
		break;
	}

	return result;
}




ImageConvertClass::ImageConvertClass()
{
}


ImageConvertClass::~ImageConvertClass()
{
}
