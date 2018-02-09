#include "VAReaderWriterTranscoder.h"



static GUID intermediateVideoFormats1[] =
{
	MFVideoFormat_RGB32,
	MFVideoFormat_NV12,
	MFVideoFormat_YV12,
	MFVideoFormat_YUY2,
	
};
int nIntermediateVideoFormats1 = 4;


// audio stream formats that every audio decoder and encoder should
// be able to agree on - uncompressed audio data
static GUID intermediateAudioFormats1[] =
{
	MFAudioFormat_Float,
	MFAudioFormat_PCM,
};
int nIntermediateAudioFormats1 = 2;



 VACReaderWriterTranscoder::VACReaderWriterTranscoder()
{
	
	HRESULT hr = S_OK;
    // Start up Media Foundation platform.
    MFStartup(MF_VERSION);
	hr= GetDevicesOfCat(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID, m_pCameraActiveate);
	hr= GetDevicesOfCat(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID, m_pAudioActiveate);
	UINT length;

	WCHAR *szFriendlyName = NULL;

	m_pCameraActiveate->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &szFriendlyName, &length);

	m_pAudioActiveate->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &szFriendlyName, &length);


	

	CComPtr<IMFMediaSource> videoSource;
	CComPtr<IMFMediaSource> aiduoSource;
	
	CComPtr<IMFCollection> pCollection;
	m_pCameraActiveate->ActivateObject(IID_PPV_ARGS(&videoSource));
	m_pAudioActiveate->ActivateObject(IID_PPV_ARGS(&aiduoSource));

	MFCreateCollection(&pCollection); 

	pCollection->AddElement(videoSource);
	pCollection->AddElement(aiduoSource);
	MFCreateAggregateSource(pCollection,  &m_pMixSource);


}


VACReaderWriterTranscoder::~VACReaderWriterTranscoder(void)
{
    // release any COM objects before calling MFShutdown() - otherwise MFShutdown() will cause them
    // to go away, and the CComPtr destructor will AV
	
  
	m_pCameraActiveate = NULL;
	m_pAudioActiveate = NULL;
	m_pMixSource = NULL;		
    m_pSourceReader = NULL;
    m_pSinkWriter = NULL;
	
    // Shutdown the Media Foundation platform
    MFShutdown();

	
}

HRESULT VACReaderWriterTranscoder::StatrtCapture(LPCWSTR savePath, HWND  hwnd)
{
	    HRESULT hr = S_OK;

	    CComPtr<IMFAttributes> pConfigAttrs;

		hr = MFCreateAttributes(&pConfigAttrs, 3);
	
		// set MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS property in the store

		
		//hr = pConfigAttrs->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, callBack);
		hr = pConfigAttrs->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);

		hr=pConfigAttrs->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);

		

		// create a source reader
		hr = MFCreateSourceReaderFromMediaSource(m_pMixSource, pConfigAttrs, &m_pSourceReader);
	

		IMFMediaType *pType = NULL;
		if (SUCCEEDED(hr))
			hr = m_pSourceReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pType);
		

		CComPtr<IMFMediaSink> pSink;
	
		CComPtr<IMFStreamSink> pStream;
		CComPtr<IMFGetService> pService;
		DWORD count;
		DWORD pdwCharacteristics;
		CComPtr<IMFPresentationClock> pClock;
		
		hr = MFCreateVideoRenderer(__uuidof(IMFMediaSink),(void**)(&pSink));

		hr = pSink->GetStreamSinkCount(&count);
		hr = pSink->GetCharacteristics(&pdwCharacteristics);
		hr = pSink->AddStreamSink(2, pType, &pStream);
		hr = pSink->GetPresentationClock(&pClock);
		//§³§à§Ó§ã§Ö§Þ §ß§Ö §á§à§Û§Þ§å §é§ä§à §Þ§ß§Ö §ã§Õ§Ö§Ý§Ñ§ä§î §ã IMFPresentationClock

		hr = pSink->QueryInterface(_uuidof(IMFGetService),(void**)&pService);
		hr = pService->GetService(MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl,(void**)&m_pVideoDisplay);
		hr = m_pVideoDisplay->SetVideoWindow(hwnd);




		// create a sink writer
		hr = MFCreateSinkWriterFromURL(savePath, NULL, pConfigAttrs, &m_pSinkWriter);





	

		// map the streams found in the source file from the source reader to the
		// sink writer, while negotiating media types
		hr = MapStreams();
		
		//Request all source.
		
		m_pSinkWriter->BeginWriting();
		
		m_bFirstSample = true;

		if (Capting)
		{
		  hr = StartReadSample();

		}

		return hr;
		
		// run the transcode loop
		
	

	return hr;
}

HRESULT VACReaderWriterTranscoder::StopCature(void)
{
	HRESULT hr = S_OK;
	m.lock();
	
		if (m_pSourceReader != NULL)
		{
			m_pSourceReader->Flush(-2);
			
			m_pSourceReader = NULL;
		}

		if (m_pSinkWriter != NULL)
		{

			hr = m_pSinkWriter->Finalize();
			m_pSinkWriter = NULL;
		}

		m.unlock();

	return hr;
}

HRESULT VACReaderWriterTranscoder::GetDevicesOfCat(GUID FilterCategory, CComPtr<IMFActivate>& pMFActivate)
{

	IMFActivate **ppDevices;

	UINT32 count;


	HRESULT hr = S_OK;
	CComPtr<IMFAttributes> pAttributes;

	// Initialize an attribute store. We will use this to 
	// specify the enumeration parameters.

	hr = MFCreateAttributes(&pAttributes, 1);


	UINT32 widthOfFrame = 0;
	UINT32 heightOfFrame = 0;


	// Ask for source type = video capture devices
	if (hr >= 0)
	{
		hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, FilterCategory);

		if (pAttributes != NULL)
		{
			MFGetAttributeSize(pAttributes, MF_MT_FRAME_SIZE, &widthOfFrame, &heightOfFrame);

		}
	}


	if (hr >= 0)
	{
		MFEnumDeviceSources(pAttributes, &ppDevices, &count);
		if (count>0)
		{
			pMFActivate = ppDevices[0];
		}
	}

	return hr;
	// Enumerate devices.
}




//
// Main transcoding function that triggers the transcode process:
// 1.  Create stream reader and sink writer objects.
// 2.  Map the streams found in the source file to the sink.
// 3.  Run the transcode operation.
//
HRESULT VACReaderWriterTranscoder::Transcode(LPCWSTR source, LPCWSTR target)
{
    HRESULT hr = S_OK;

	return hr;
  
}

IMFMediaSource * VACReaderWriterTranscoder::GetMixSource()
{
	return m_pMixSource;
}

HRESULT VACReaderWriterTranscoder::Repaint()
{
	HRESULT hr = S_OK;

	if (m_pVideoDisplay)
	{
		hr = m_pVideoDisplay->RepaintVideo();
	}

	return hr;
}

HRESULT VACReaderWriterTranscoder::Resize(LPRECT desRect)
{
	HRESULT hr = S_OK;

	if (m_pVideoDisplay)
	{
		hr = m_pVideoDisplay->SetVideoPosition(NULL, desRect);
	}

	return hr;
}




HRESULT VACReaderWriterTranscoder::StartReadSample(void)
{
	HRESULT hr = S_OK;

	MF_SOURCE_READER_CONTROL_FLAG flag = MF_SOURCE_READER_CONTROLF_DRAIN;
	

	DWORD dwStreamIndex =0;
		
	DWORD  pdwStreamFlag =0;

	LONGLONG  llTimestamp =0;

	IMFSample* pSample;

	

	while(hr==S_OK&&Capting)
	{

		for (int i = 0; i < m_nStreams; i++)
		{

			hr = m_pSourceReader->ReadSample(i, 0, &dwStreamIndex, &pdwStreamFlag, &llTimestamp, &pSample);

		
			if (pSample !=NULL)
			{

				m.lock();

				if (m_bFirstSample)
					{
						m_llBaseTime = llTimestamp;
						m_bFirstSample = false;
					
					}

				if (PrtScn)
				{
					hr = m_pSourceReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &dwStreamIndex, &pdwStreamFlag, &llTimestamp, &pSample);

					SnapReadSample(pSample,_imageType);

					PrtScn = false;

				}

					// rebase the time stamp
					llTimestamp -= m_llBaseTime;

					hr = pSample->SetSampleTime(llTimestamp);


					hr = m_pSinkWriter->WriteSample(dwStreamIndex, pSample);

					
					pSample->Release();
					pSample = NULL;

				m.unlock();
			}
		
			
		}
	
	}

	return hr;


}

HRESULT  VACReaderWriterTranscoder::SnapReadSample(IMFSample* pSample, ImageType _imageType)
{
	HRESULT hr = S_OK;

	BYTE* data;
	IMFMediaBuffer* buffer;
	DWORD max, current;

	CComPtr<IMFMediaType> pStreamMediaType;

	UINT32 widthOfFrame = 0;
	UINT32 heightOfFrame = 0;

	
	pSample->ConvertToContiguousBuffer(&buffer);

	buffer->Lock(&data, &max, &current);

	

	ImageConvertClass * convert = new ImageConvertClass();
	
	wchar_t * fileName=new wchar_t[20];

	if (width==0)
	{
		width = 640;
	}
	if (height==0)
	{
		height = 480;
	}
	
	time_t now = time(0);

	switch (_imageType)
	{
		default:
			break;
	 case BMP:
			swprintf(fileName, 20, L".\\%d.bmp", now);
			break;
	 case PNG:
			swprintf(fileName, 20, L".\\%d.png", now);
			break;
	 case JPG:
			swprintf(fileName, 20, L".\\%d.jpg", now);
			break;

	}
	
	convert->ConvertDIBToImage(_imageType,data, width, height, fileName);

	

	delete convert;

	delete[] fileName;


	buffer->Unlock();

	buffer->Release();

	buffer = NULL;

	
	return hr;


	
}





HRESULT VACReaderWriterTranscoder::GetActivate(void)
{
	HRESULT hr = S_OK;



	return hr;
}

//
// Map each source reader output stream to an input stream of the sink writer, deciding
// on the target format.  Audio and video stream formats are modified to AAC and H.264 
// respectively, but other target formats remain unchanged.
//
HRESULT VACReaderWriterTranscoder::MapStreams(void)
{
	HRESULT hr = S_OK;
	BOOL isStreamSelected = FALSE;
	DWORD sourceStreamIndex = 0;
	DWORD sinkStreamIndex = 0;
	GUID streamMajorType;
	CComPtr<IMFMediaType> pStreamMediaType;


	do
	{
		m_nStreams = 0;

		while (SUCCEEDED(hr))
		{
			// check whether you have a stream with the right index - if you don't, the 
			// IMFSourceReader::GetStreamSelection() function will fail, and you will drop
			// out of the while loop
			hr = m_pSourceReader->GetStreamSelection(sourceStreamIndex, &isStreamSelected);
			if (FAILED(hr))
			{
				hr = S_OK;
				break;
			}

			// count the total number of streams for later
			m_nStreams++;

			// get the source media type of the stream
			hr = m_pSourceReader->GetNativeMediaType(
				sourceStreamIndex,           // index of the stream you are interested in
				0,                           // index of the media type exposed by the 
											 //    stream decoder
				&pStreamMediaType);          // media type
			BREAK_ON_FAIL(hr);

			// extract the major type of the source stream from the media type
			hr = pStreamMediaType->GetMajorType(&streamMajorType);
			BREAK_ON_FAIL(hr);

			// select a stream, indicating that the source should send out its data instead
			// of dropping all of the samples
			hr = m_pSourceReader->SetStreamSelection(sourceStreamIndex, TRUE);
			BREAK_ON_FAIL(hr);

			// if this is a video or audio stream, transcode it and negotiate the media type
			// between the source reader stream and the corresponding sink writer stream.  
			// If this is a some other stream format (e.g. subtitles), just pass the media 
			// type unchanged.
			if (streamMajorType == MFMediaType_Audio || streamMajorType == MFMediaType_Video)
			{
				// get the target media type - the media type into which you will transcode
				// the data of the current source stream
				hr = GetTranscodeMediaType(pStreamMediaType);
				BREAK_ON_FAIL(hr);

				// add the stream to the sink writer - i.e. tell the sink writer that a 
				// stream with the specified index will have the target media type
				hr = m_pSinkWriter->AddStream(pStreamMediaType, &sinkStreamIndex);
				BREAK_ON_FAIL(hr);

				// hook up the source and sink streams - i.e. get them to agree on an
				// intermediate media type that will be used to pass data between source 
				// and sink
				hr = ConnectStream(sourceStreamIndex, streamMajorType);
				BREAK_ON_FAIL(hr);
			}
			else
			{
				// add the stream to the sink writer with the exact same media type as the
				// source stream
				hr = m_pSinkWriter->AddStream(pStreamMediaType, &sinkStreamIndex);
				BREAK_ON_FAIL(hr);
			}

			// make sure that the source stream index is equal to the sink stream index
			if (sourceStreamIndex != sinkStreamIndex)
			{
				hr = E_UNEXPECTED;
				break;
			}

			// increment the source stream index, so that on the next loop you are analyzing
			// the next stream
			sourceStreamIndex++;

			// release the media type
			pStreamMediaType = NULL;
		}

		BREAK_ON_FAIL(hr);

	} while (false);

	return hr;
}



//
// Set the target target audio and video media types to hard-coded values.  In this case you
// are setting audio to AAC, and video to 720p H.264
//
HRESULT VACReaderWriterTranscoder::GetTranscodeMediaType(
    CComPtr<IMFMediaType>& pStreamMediaType)
{
    HRESULT hr = S_OK;
    GUID streamMajorType;

    do
    {
        // extract the major type of the source stream from the media type
        hr = pStreamMediaType->GetMajorType(&streamMajorType);
        BREAK_ON_FAIL(hr);

        // if this is an audio stream, configure a hard-coded AAC profile.  If this is a
        // video stream, configure an H.264 profile
        if(streamMajorType == MFMediaType_Audio)
        {
            hr = GetTranscodeAudioType(pStreamMediaType);
        }
        else if(streamMajorType == MFMediaType_Video)
        {
            hr = GetTranscodeVideoType(pStreamMediaType);
        }
    }
    while(false);

    return hr;
}



//
// Get the target audio media type - use the AAC media format.
//
HRESULT VACReaderWriterTranscoder::GetTranscodeAudioType(
    CComPtr<IMFMediaType>& pStreamMediaType)
{
    HRESULT hr = S_OK;

    do
    {
        BREAK_ON_NULL(pStreamMediaType, E_POINTER);

        // wipe out existing data from the media type
        hr = pStreamMediaType->DeleteAllItems();
        BREAK_ON_FAIL(hr);

        // reset the major type to audio since we just wiped everything out
        pStreamMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
        BREAK_ON_FAIL(hr);

        // set the audio subtype
        hr = pStreamMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);
        BREAK_ON_FAIL(hr);

        // set the number of audio bits per sample
        hr = pStreamMediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
        BREAK_ON_FAIL(hr);

        // set the number of audio samples per second
        hr = pStreamMediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, 44100);
        BREAK_ON_FAIL(hr);

        // set the number of audio channels
        hr = pStreamMediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, 2);
        BREAK_ON_FAIL(hr);

        // set the Bps of the audio stream
        hr = pStreamMediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 16000);
        BREAK_ON_FAIL(hr);

        // set the block alignment of the samples
        hr = pStreamMediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 1);
        BREAK_ON_FAIL(hr);
    }
    while(false);

    return hr;
}



//
// Get the target video media type - use the H.264 media format.
//
HRESULT VACReaderWriterTranscoder::GetTranscodeVideoType(
    CComPtr<IMFMediaType>& pStreamMediaType)
{
    HRESULT hr = S_OK;

    do
    {

		MFGetAttributeSize(pStreamMediaType, MF_MT_FRAME_SIZE, &width, &height);

        BREAK_ON_NULL(pStreamMediaType, E_POINTER);

        // wipe out existing data from the media type
        hr = pStreamMediaType->DeleteAllItems();
		
        BREAK_ON_FAIL(hr);

        // reset the major type to video since we just wiped everything out
        pStreamMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        BREAK_ON_FAIL(hr);

        // set the video subtype
        hr = pStreamMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
        BREAK_ON_FAIL(hr);

        // set the frame size to 720p as a 64-bit packed value
        hr = MFSetAttributeSize(
            pStreamMediaType,           // attribute store on which to set the value
            MF_MT_FRAME_SIZE,           // value ID GUID
            1280, 720);                 // frame width and height
        BREAK_ON_FAIL(hr);

        // Set the frame rate to 30/1.001 - the standard frame rate of NTSC television - as 
        // a 64-bit packed value consisting of a fraction of two integers
        hr = MFSetAttributeRatio(
            pStreamMediaType,           // attribute store on which to set the value
            MF_MT_FRAME_RATE,           // value
            30000, 1001);               // frame rate ratio
        BREAK_ON_FAIL(hr);

        // set the average bitrate of the video in bits per second - in this case 10 Mbps
        hr = pStreamMediaType->SetUINT32(MF_MT_AVG_BITRATE, 10000000 );
        BREAK_ON_FAIL(hr);

        // set the interlace mode to progressive
        hr = pStreamMediaType->SetUINT32(MF_MT_INTERLACE_MODE, 
            MFVideoInterlace_Progressive );
        BREAK_ON_FAIL(hr);

        // set the pixel aspect ratio to 1x1 - square pixels
        hr = MFSetAttributeSize(pStreamMediaType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1 );
        BREAK_ON_FAIL(hr);
    }
    while(false);

    return hr;
}



//
// Attempt to find an uncompressed media type for the specified stream that both the source 
// and sink can agree on
//
HRESULT VACReaderWriterTranscoder::ConnectStream(DWORD dwStreamIndex, 
    const GUID& streamMajorType)
{
    HRESULT hr = S_OK;

    CComPtr<IMFMediaType> pPartialMediaType;
    CComPtr<IMFMediaType> pFullMediaType;

    BOOL fConfigured = FALSE;
    GUID* intermediateFormats = NULL;
    int nFormats = 0;

    do
    {
        // create a media type container object that will be used to match stream input
        // and output media types
        hr = MFCreateMediaType( &pPartialMediaType );
        BREAK_ON_FAIL(hr);

        // set the major type of the partial match media type container
        hr = pPartialMediaType->SetGUID( MF_MT_MAJOR_TYPE, streamMajorType );
        BREAK_ON_FAIL(hr);

        // Get the appropriate list of intermediate formats - formats that every decoder and
        // encoder of that type should agree on.  Essentially these are the uncompressed 
        // formats that correspond to decoded frames for video, and uncompressed audio 
        // formats
        if(streamMajorType == MFMediaType_Video)
        {
            intermediateFormats = intermediateVideoFormats1;
            nFormats = nIntermediateVideoFormats1;
        }
        else if(streamMajorType == MFMediaType_Audio)
        {
            intermediateFormats = intermediateAudioFormats1;
            nFormats = nIntermediateAudioFormats1;
        } 
        else
        {
            hr = E_UNEXPECTED;
            break;
        }

        
        // loop through every intermediate format that you have for this major type, and
        // try to find one on which both the source stream and sink stream can agree on
        for( int x = 0; x < nFormats; x++ )
        {
            // set the format of the partial media type
            hr = pPartialMediaType->SetGUID( MF_MT_SUBTYPE, intermediateFormats[x] );
            BREAK_ON_FAIL(hr);

            // set the partial media type on the source stream
            hr = m_pSourceReader->SetCurrentMediaType( 
                dwStreamIndex,                      // stream index
                NULL,                               // reserved - always NULL
                pPartialMediaType );                // media type to try to set

            // if the source stream (i.e. the decoder) is not happy with this media type -
            // if it cannot decode the data into this media type, restart the loop in order 
            // to try the next format on the list
            if( FAILED(hr) )
            {
                hr = S_OK;
                continue;
            }

            pFullMediaType = NULL;

            // if you got here, the source stream is happy with the partial media type you set
            // - extract the full media type for this stream (with all internal fields 
            // filled in)
            hr = m_pSourceReader->GetCurrentMediaType( dwStreamIndex, &pFullMediaType );

            // Now try to match the full media type to the corresponding sink stream
            hr = m_pSinkWriter->SetInputMediaType( 
                dwStreamIndex,             // stream index
                pFullMediaType,            // media type to match
                NULL );                    // configuration attributes for the encoder

            // if the sink stream cannot accept this media type - i.e. if no encoder was
            // found that would accept this media type - restart the loop and try the next
            // format on the list
            if( FAILED(hr) )
            {
                hr = S_OK;
                continue;
            }

            // you found a media type that both the source and sink could agree on - no need
            // to try any other formats
            fConfigured = TRUE;

		
            break;
        }
        BREAK_ON_FAIL(hr);

        // if you didn't match any formats return an error code
        if( !fConfigured )
        {
            hr = MF_E_INVALIDMEDIATYPE;
            break;
        }

    }
    while(false);

    return hr;
}
