#pragma once
#include <windows.h>
#include <initguid.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <stdio.h>
#include <functiondiscoverykeys_devpkey.h>

class CWSAudioCapture
{
public:
	CWSAudioCapture();
	~CWSAudioCapture();

public:
	bool InitAudioCapture();
	bool StartCapure();

private:
	IMMDevice* m_audio_device;
private:
	FILE* f_aac_;
};
