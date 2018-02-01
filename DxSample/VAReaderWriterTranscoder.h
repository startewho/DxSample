#pragma once
#include "Common.h"
#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <Ks.h>
#include <Codecapi.h>

#include <mutex>

#include <Mfreadwrite.h>

#include <d3d11.h>



 class VACReaderWriterTranscoder
{
    public:
        VACReaderWriterTranscoder(void);
        ~VACReaderWriterTranscoder(void);

		HRESULT	StatrtCapture(LPCWSTR savePath, HWND hwnd); //��ʼ¼��
		HRESULT	StopCature(void);	 //����¼��



		HRESULT GetDevicesOfCat(GUID FilterCategory, CComPtr<IMFActivate>& pMFActivate);
		BOOL Capting;
        // Start the transcode
        HRESULT Transcode(LPCWSTR source, LPCWSTR sink);

    private:
		HWND mainHwnd;
		BOOL m_bFirstSample;
		LONGLONG m_llBaseTime;
		
		std::mutex m;
		volatile long m_nRefCount;                       // COM reference count.
		CComPtr<IMFActivate> m_pCameraActiveate;
		CComPtr<IMFActivate> m_pAudioActiveate;
		CComPtr<IMFMediaSource> m_pMixSource;
		
        CComPtr<IMFSourceReader> m_pSourceReader;
        CComPtr<IMFSinkWriter> m_pSinkWriter;
      
        int m_nStreams;
		
		HRESULT StartReadSample(void);
		// map individual streams
		HRESULT GetActivate(void);

        // map individual streams
        HRESULT MapStreams(void);

        // figure out the target media types
        HRESULT GetTranscodeMediaType(CComPtr<IMFMediaType>& pStreamMediaType);
        HRESULT GetTranscodeVideoType(CComPtr<IMFMediaType>& pStreamMediaType);
        HRESULT GetTranscodeAudioType(CComPtr<IMFMediaType>& pStreamMediaType);

        // connect the streams
        HRESULT ConnectStream(DWORD dwStreamIndex, const GUID& streamMajorType);

		HRESULT SnapReadSample(IMFSample* pSample)

 
};

