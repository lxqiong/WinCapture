#include "VideoGrabberSample.h"
AudioGrabberSample::AudioGrabberSample()
{
	f_aac_ = fopen("./video.rgb", "wb");
}

AudioGrabberSample::~AudioGrabberSample()
{

}

HRESULT AudioGrabberSample::QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	return S_OK;
}

ULONG AudioGrabberSample::AddRef()
{
	return 0;
}

ULONG AudioGrabberSample::Release()
{
	return 0;
}

HRESULT AudioGrabberSample::SampleCB(double SampleTime, IMediaSample *pSample)
{

	return 0;
}

HRESULT AudioGrabberSample::BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen)
{
	fwrite(pBuffer, 1, BufferLen, f_aac_);
	return 0;
}