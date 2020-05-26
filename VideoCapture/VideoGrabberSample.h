#pragma once
#include "qedit.h"
#include <stdio.h>

class AudioGrabberSample : public ISampleGrabberCB 
{

public:
	AudioGrabberSample();
	~AudioGrabberSample();
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();

	virtual HRESULT STDMETHODCALLTYPE SampleCB(
		double SampleTime,
		IMediaSample *pSample);
		virtual HRESULT STDMETHODCALLTYPE BufferCB(
			double SampleTime,
			BYTE *pBuffer,
			long BufferLen);
private:
	FILE* f_aac_;
};