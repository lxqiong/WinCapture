
#include "WSAudioCapture.h"

int main()
{
	::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	CWSAudioCapture* audio_capture = new CWSAudioCapture();
	audio_capture->InitAudioCapture();
	audio_capture->StartCapure();

	::CoUninitialize();

	return 0;
}