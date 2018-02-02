#include "ImageConvertClass.h"
#include <jpeglib.h>






bool ImageConvertClass::ConvertDIBToJPG(unsigned char * buffer, int nWidth, int nHeight, wchar_t * outFile)
{
	jpeg_compress_struct jpegInfo;
	jpeg_error_mgr errorMgr;
	unsigned char* pJpegBuffer=NULL;
	unsigned long outSize;
	jpegInfo.err = jpeg_std_error(&errorMgr);
	//ע��ʧ�ܵĻص�����
	//toWriteInfo.err->error_exit = error_exit;
	jpeg_create_compress(&jpegInfo);
	//����ѹ�����ͼƬ
	//FILE* fp = NULL;
	//_wfopen_s(&fp, L"c:\\output.jpg", L"wb+");
	//jpeg_stdio_dest(&toWriteInfo, fp);
	//ȷ��Ҫ�������ѹ����jpeg�����ݿռ�
	jpeg_mem_dest(&jpegInfo, &pJpegBuffer, &outSize);
	jpegInfo.image_width = nWidth;
	jpegInfo.image_height = nHeight;
	//toWriteInfo.jpeg_width = nWidth / 2;
	//toWriteInfo.jpeg_height = nHeight / 2;
	jpegInfo.input_components = 4;// �ڴ�Ϊ1,��ʾ�Ҷ�ͼ�� ����ǲ�ɫλͼ����Ϊ4
	jpegInfo.in_color_space = JCS_EXT_BGRA; //JCS_GRAYSCALE��ʾ�Ҷ�ͼ��JCS_RGB��ʾ��ɫͼ�� 
	jpeg_set_defaults(&jpegInfo);
	jpeg_set_quality(&jpegInfo, 100, TRUE);	//����ѹ������100��ʾ100%
	jpeg_start_compress(&jpegInfo, TRUE);
	int nRowStride = nWidth * 4;	// �����������ͼ,�˴���Ҫ����4
	JSAMPROW row_pointer[1];	// һ��λͼ
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
