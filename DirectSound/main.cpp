
#include "DSAudioCapure.h"
#include <functional>
#pragma comment(lib,"Dsound.lib")

BOOL CALLBACK CustomDSEnumCallback(
	LPGUID lpGuid,
	LPCWSTR lpcstrDescription,
	LPCWSTR lpcstrModule,
	LPVOID lpContext
)
{
	CDSAudioCapure* audio_capture = (CDSAudioCapure*)lpContext;
	audio_capture->OnAudioCapture(lpGuid, lpcstrDescription, lpcstrModule,nullptr);
	return TRUE;
}

int main()
{
	::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	CDSAudioCapure* audio_capture_ = new CDSAudioCapure();
	HRESULT hr = DirectSoundCaptureEnumerate(CustomDSEnumCallback, audio_capture_);
	if (hr != DS_OK)
	{
		return -1;
	}
	if (audio_capture_->CreateCapture() == false)
	{
		return -1;
	}

	audio_capture_->StartCapture();

	delete audio_capture_;
	audio_capture_ = nullptr;
	::CoUninitialize();
	return 0;
}