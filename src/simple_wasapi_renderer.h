/*
simple_wasapi_renderer.h - audio renderer based on Microsoft's example program
https://github.com/microsoft/Windows-classic-samples/tree/main/Samples/WASAPIRendering
https://learn.microsoft.com/en-us/windows/win32/coreaudio/rendering-a-stream
2023/08/22, peshqa
*/
#pragma once
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <cmath>

#pragma comment (lib, "Ole32.lib")

# define PI_DOUBLE           3.14159265358979323846

enum WASAPIRenderType : int
{
	AUDIO_PCM_16_BIT = 0,
	AUDIO_FLOAT
};

int GenerateSineSamples(BYTE* Buffer, size_t BufferLength, DWORD Frequency, WORD ChannelCount, DWORD SamplesPerSecond, double* InitialTheta)
{
    double sampleIncrement = (Frequency * (PI_DOUBLE * 2)) / (double)SamplesPerSecond;
    float* dataBuffer = (float*)Buffer;
    double theta = (InitialTheta != NULL ? *InitialTheta : 0);

    for (size_t i = 0; i < BufferLength / sizeof(float); i += ChannelCount)
    {
        //float sinValue = ((int)(theta/4) %2) ? 0.5f: -0.5f;
		float sinValue = sin(theta);
        for (size_t j = 0; j < ChannelCount; j++)
        {
            dataBuffer[i + j] = sinValue;
        }
        theta += sampleIncrement;
    }

    if (InitialTheta != NULL)
    {
        *InitialTheta = theta;
    }
	
	return 0;
}

int InitWASAPIRenderer()
{
	int TARGET_LATENCY = 30;
	
	//OutputDebugStringW(L"Initializing WASAPI Renderer\n");
	
	// Step 1: Initialize the COM library
	if (FAILED(CoInitializeEx(0, COINIT_APARTMENTTHREADED)))
	{
		return 1;
	}
	
	
	// Step 2: Pick a device
	IMMDevice *endpoint{};
	IMMDeviceEnumerator *deviceEnumerator{};
	HRESULT res = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&deviceEnumerator));
	if (res != S_OK)
	{
		return 2;
	}
	
	ERole deviceRole = eConsole;
	res = deviceEnumerator->GetDefaultAudioEndpoint(eRender, deviceRole, &endpoint);
	if (res != S_OK)
	{
		return 3;
	}
	
	
	// Step 3: Initialize a renderer
	IAudioClient *audioClient{};
	res = endpoint->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void**>(&audioClient));
	if (res != S_OK)
	{
		return 4;
	}
	
	
	// Step 4: Load Mix format since we are using shared mode
	WAVEFORMATEX *mix_format{};
	res = audioClient->GetMixFormat(&mix_format);
	if (res != S_OK)
	{
		return 5;
	}
	
	int render_type = AUDIO_PCM_16_BIT;
	if (mix_format->wFormatTag == WAVE_FORMAT_PCM || mix_format->wFormatTag == WAVE_FORMAT_EXTENSIBLE && 
		((WAVEFORMATEXTENSIBLE*)mix_format)->SubFormat == KSDATAFORMAT_SUBTYPE_PCM)
	{
		if (mix_format->wBitsPerSample != 16)
		{
			// unexpected format (unknown integer pcm type)
			// TODO: if this situation ever occurs, check if there is only one channel
			return 6;
		}
		
	} else if (mix_format->wFormatTag == WAVE_FORMAT_IEEE_FLOAT || mix_format->wFormatTag == WAVE_FORMAT_EXTENSIBLE && 
		((WAVEFORMATEXTENSIBLE*)mix_format)->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
			
		render_type = AUDIO_FLOAT;
		
	} else {
		// unexpected format
		return 7;
	}
	
	
	// Step 5: Initialize WASAPI in event driven mode
	if (FAILED(audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
        0, //AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST, // <- enables event driven system
        1000 * (long long)10000,
        0,
        mix_format,
        NULL)))
	{
		return 8;
	}
	
	// Get the actual size of the allocated buffer.
	UINT32 bufferFrameCount{};
    if (FAILED(audioClient->GetBufferSize(&bufferFrameCount)))
	{
		return 9;
	}
	
	IAudioRenderClient *pRenderClient{};
	if (FAILED(audioClient->GetService(IID_PPV_ARGS(&pRenderClient))))
	{
		return 10;
	}
	
	BYTE *pData{};
	// Grab the entire buffer for the initial fill operation.
    if (FAILED(pRenderClient->GetBuffer(bufferFrameCount, &pData)))
	{
		return 11;
	}
	
	// Load the initial data into the shared buffer.
	DWORD flags = 0;
	double theta{};
    //hr = pMySource->LoadData(bufferFrameCount, pData, &flags);
	GenerateSineSamples(pData, bufferFrameCount, 440,2, 48000, &theta);

    if (FAILED(pRenderClient->ReleaseBuffer(bufferFrameCount, flags)))
	{
		return 12;
	}
	
	
	// Calculate the actual duration of the allocated buffer.
	REFERENCE_TIME hnsActualDuration{};
	REFERENCE_TIME REFTIMES_PER_MILLISEC = 10000;
    hnsActualDuration = 10000000.0 * bufferFrameCount / mix_format->nSamplesPerSec;
	
	if (FAILED(audioClient->Start()))
	{
		return 13;
	}
	
	int max_cycles = 2;
	// Each loop fills about half of the shared buffer.
    while (flags != AUDCLNT_BUFFERFLAGS_SILENT)
    {
        // Sleep for half the buffer duration.
        Sleep((DWORD)(hnsActualDuration/REFTIMES_PER_MILLISEC/2));

        // See how much buffer space is available.
		UINT32 numFramesPadding{};
        if (FAILED(audioClient->GetCurrentPadding(&numFramesPadding)))
		{
			return 14;
		}

        UINT32 numFramesAvailable = bufferFrameCount - numFramesPadding;

        // Grab all the available space in the shared buffer.
        if (FAILED(pRenderClient->GetBuffer(numFramesAvailable, &pData)))
		{
			return 15;
		}

        // Get next 1/2-second of data from the audio source.
        //hr = pMySource->LoadData(numFramesAvailable, pData, &flags);
		GenerateSineSamples(pData, numFramesAvailable, 440,2, 48000, &theta);

        if (FAILED(pRenderClient->ReleaseBuffer(numFramesAvailable, flags)))
		{
			return 16;
		}
		
		if (max_cycles-- <= 0)
			break;
    }

    // Wait for last data in buffer to play before stopping.
    Sleep((DWORD)(hnsActualDuration/REFTIMES_PER_MILLISEC/2));

    if (FAILED(audioClient->Stop()))  // Stop playing.
	{
		return 17;
	}
	
	return 0;
}