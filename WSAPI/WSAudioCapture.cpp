
#include "WSAudioCapture.h"
#include <AudioClient.h>

#define FALSE_ON_ERROR(hres)  \
              if (FAILED(hres)) { return false; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID   IID_IAudioClient = __uuidof(IAudioClient);
const IID   IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

CWSAudioCapture::CWSAudioCapture()
{
	f_aac_ = fopen("./audio.pcm", "wb");
}

CWSAudioCapture::~CWSAudioCapture()
{

}

bool CWSAudioCapture::InitAudioCapture()
{
	HRESULT hr = S_OK;
	IMMDeviceEnumerator* pAudioEnum=nullptr;
	IMMDeviceCollection* pCollection = nullptr;
	
	hr=CoCreateInstance(CLSID_MMDeviceEnumerator, nullptr, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pAudioEnum);
	FALSE_ON_ERROR(hr);
	hr = pAudioEnum->EnumAudioEndpoints(eCapture,DEVICE_STATE_ACTIVE,&pCollection);
	FALSE_ON_ERROR(hr);
	
	UINT ncount=0;
	hr=pCollection->GetCount(&ncount);
	FALSE_ON_ERROR(hr);
	IMMDevice* pDevice = nullptr;
	IPropertyStore* pProps = nullptr;
	LPWSTR pwszid = nullptr;
	for (size_t i = 0; i < ncount; i++)
	{
		hr = pCollection->Item(i, &pDevice);
		FALSE_ON_ERROR(hr);
		hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
		FALSE_ON_ERROR(hr);
		PROPVARIANT varName;
		PropVariantInit(&varName);
		hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
		FALSE_ON_ERROR(hr);
		
		hr = pDevice->GetId(&pwszid);
		FALSE_ON_ERROR(hr);
		if (i == 0)
		{
			m_audio_device = pDevice;
		}
	}

	return true;
}

bool CWSAudioCapture::StartCapure()
{
	IAudioClient* pClient;
	HRESULT hr = m_audio_device->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pClient);
	FALSE_ON_ERROR(hr);
	WAVEFORMATEX        *pwfx = NULL;
	hr = pClient->GetMixFormat(&pwfx);
	FALSE_ON_ERROR(hr);
	hr=pClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
		AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST,
		1000000,
		0,
		pwfx,
		NULL);
	FALSE_ON_ERROR(hr);
	IAudioCaptureClient *pCaptureClient = NULL;
	hr = pClient->GetService(IID_IAudioCaptureClient, (void**)&pCaptureClient);
	FALSE_ON_ERROR(hr);
	HANDLE hAudioSamplesReadyEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
	if (hAudioSamplesReadyEvent == NULL)
	{
		return false;
	}
	hr=pClient->SetEventHandle(hAudioSamplesReadyEvent);
	FALSE_ON_ERROR(hr);

	pClient->Start();
	int nCaptureSize = 8 * 1024 * 1024;
	int nFrameSize = (pwfx->wBitsPerSample/8)*(pwfx->nChannels);
	BYTE* pCaptureBuffer = new BYTE[nCaptureSize];
	memset(pCaptureBuffer, 0, nCaptureSize);
	HANDLE waitArray[2];
	waitArray[0] = hAudioSamplesReadyEvent;
	while (true)
	{
		DWORD waitResult = WaitForMultipleObjects(1, waitArray, FALSE, INFINITE);
		switch (waitResult)
		{
		case WAIT_OBJECT_0+0:
		{
			UINT32 pNumberPacket = 0;
			UINT32 pNumberFrames = 0;
			DWORD flags;
			hr = pCaptureClient->GetNextPacketSize(&pNumberPacket);
			FALSE_ON_ERROR(hr);
			while (pNumberPacket != 0)
			{
				hr = pCaptureClient->GetBuffer(&pCaptureBuffer, &pNumberFrames, &flags, 0, 0);
				int nBufferlen = pNumberFrames * nFrameSize;
				fwrite(pCaptureBuffer, 1, nBufferlen, f_aac_);
				FALSE_ON_ERROR(hr);
				pCaptureClient->ReleaseBuffer(pNumberFrames);
				pCaptureClient->GetNextPacketSize(&pNumberPacket);
			}
			if (pNumberPacket == 0)
			{
				int i = 0;
				i++;
				break;
			}
		}
		break;

		default:
			break;
		}
	}
	return true;
}