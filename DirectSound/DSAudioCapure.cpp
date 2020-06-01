#include "DSAudioCapure.h"

CDSAudioCapure::CDSAudioCapure()
{
	f_aac_ = fopen("./audio.pcm", "wb");
}

CDSAudioCapure::~CDSAudioCapure()
{

}

BOOL CDSAudioCapure::OnAudioCapture(LPGUID lpGuid, LPCWSTR lpcstrDescription, LPCWSTR lpcstrModule, LPVOID lpContext)
{
	if (lpGuid == nullptr)
	{
		return FALSE;
	}
	audio_device device;
	device.device_guid = lpGuid;
	device.device_name = lpcstrDescription;
	m_all_devices.push_back(device);
	return TRUE;
}

bool CDSAudioCapure::CreateCapture()
{
	if (m_all_devices.empty())
	{
		return false;
	}
	audio_device device = m_all_devices.front();
	
	HRESULT hr = DirectSoundCaptureCreate8(device.device_guid, &m_audio_capture, NULL);
	if (hr != DS_OK)
	{
		return false;
	}
	
	WAVEFORMATEX wfx;
	memset(&wfx, 0, sizeof(WAVEFORMATEX));
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nSamplesPerSec = m_isampeles;
	wfx.nChannels = m_ichannles;
	wfx.wBitsPerSample = m_ibitsample;
	wfx.nBlockAlign = 4;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * 4;
	m_iNeedBufferSzie = 3 * wfx.nSamplesPerSec * m_ichannles * (m_ibitsample / 8); //采集缓冲3秒数据;
	DSCBUFFERDESC buffer_desc;
	memset(&buffer_desc, 0, sizeof(DSCBUFFERDESC));
	buffer_desc.dwSize = sizeof(DSCBUFFERDESC);
	buffer_desc.dwBufferBytes = m_iNeedBufferSzie;
	buffer_desc.lpwfxFormat = &wfx;
	hr= m_audio_capture->CreateCaptureBuffer(&buffer_desc, &m_audio_buffer, nullptr);

	return hr==DS_OK;
}

bool CDSAudioCapure::StartCapture()
{
	HRESULT hr = m_audio_buffer->Start(DSCBSTART_LOOPING);
	if (hr != DS_OK)
	{
		return false;
	}
	int nbufferlen = (m_isampeles * m_ichannles*m_ibitsample / 8)/1000*40;

	DWORD dwCapture=0;
	DWORD dwReadPtr=0;
	DWORD nPreReadPtr = 0; //上一次读取数据的位置
	DWORD nReadLen = 0;	//本次读取的数据大小
	while (true)
	{
		m_audio_buffer->GetCurrentPosition(&dwCapture, &dwReadPtr);
	//	printf("buffer pos:%ld\n", dwReadPtr);
		if (dwReadPtr < nPreReadPtr) //循环了
		{
			
			nReadLen = m_iNeedBufferSzie - (nPreReadPtr-dwReadPtr);
		}
		else
		{
			nReadLen = dwReadPtr - nPreReadPtr;
		}
		if (nReadLen < nbufferlen)
		{
			continue;
		}
		void *ptr1 = 0;
		void *ptr2 = 0;
		DWORD len1 = 0;
		DWORD len2 = 0;
		hr =m_audio_buffer->Lock(nPreReadPtr, nbufferlen,&ptr1,&len1,&ptr2,&len2,0);
		if (FAILED(hr))
		{
			return false;
		}
		int nbuffer = len1 + len2;
		unsigned char *pBuffer = new unsigned char[nbuffer + 1];
		if (len1)
		{
			memcpy(pBuffer, ptr1, len1);
		}
		if (len2)
		{
			memcpy(pBuffer+len1, ptr2, len2);
		}
		nPreReadPtr += nbuffer;
		nPreReadPtr %= m_iNeedBufferSzie;
	//	printf("pre buffer pos:%ld\n", nPreReadPtr);
		m_audio_buffer->Unlock(ptr1, len1, ptr2, len2);
		fwrite(pBuffer, 1, nbuffer, f_aac_);
		delete[] pBuffer;

	}

	return false;
}

