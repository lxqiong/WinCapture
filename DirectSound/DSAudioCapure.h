#pragma once
#include <dsound.h>
#include <string>
#include <vector>
#include <atlcomcli.h>
class CDSAudioCapure
{
public:
	CDSAudioCapure();
	~CDSAudioCapure();
	BOOL  OnAudioCapture(LPGUID lpGuid,LPCWSTR lpcstrDescription,LPCWSTR lpcstrModule,LPVOID lpContext);
	bool CreateCapture();
	bool StartCapture();
private:
	typedef struct _device
	{
		LPGUID device_guid;
		std::wstring device_name;
	}audio_device;
	typedef std::vector<audio_device> audio_devices;
	audio_devices m_all_devices;
	CComPtr <IDirectSoundCapture8> m_audio_capture;
	CComPtr <IDirectSoundCaptureBuffer> m_audio_buffer;
	int m_isampeles = 16000;
	int m_ichannles = 2;
	int m_ibitsample = 16;
	int m_iNeedBufferSzie = 0;
	FILE* f_aac_;
};
