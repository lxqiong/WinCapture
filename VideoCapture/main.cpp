#include <iostream>
#include <vector>
#include <Objbase.h>
#include <dshow.h>
#include <strsafe.h>
#include "qedit.h"
#include "VideoGrabberSample.h"



typedef struct _device_name
{
	std::wstring device_name;
	IMoniker* pDev;
}CustomName;

typedef std::vector<CustomName> VECDEVICES;

HRESULT AddFilterByCLSID(IGraphBuilder *pGraph, const GUID& clsid, LPCWSTR wszName, IBaseFilter **ppF);

HRESULT ConnectFilters(IGraphBuilder* pGraph,IBaseFilter* pSrc,IBaseFilter* pDest);
HRESULT GetUnconnectPin(IBaseFilter* pFilter,PIN_DIRECTION PinDir,IPin** pPin);

void EnumAllInputDevices(VECDEVICES& alldevice)
{
	alldevice.clear();
	ICreateDevEnum* pSysDevEnum = nullptr;
	HRESULT hr=S_FALSE;
	hr =CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (LPVOID*)&pSysDevEnum);
	if (FAILED(hr))
	{
		printf("system error\n");
		return;
	}
	IEnumMoniker* pEnumDev = nullptr;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,&pEnumDev,0);
	if (FAILED(hr))
	{
		printf("enumerator error\n");
		return;
	}
	IMoniker* pDev;
	ULONG ufetched=0;
	
	while (pEnumDev->Next(1,&pDev, &ufetched)==S_OK)
	{
		IPropertyBag* pProperty = nullptr;
		hr = pDev->BindToStorage(NULL, NULL, IID_IPropertyBag, (void**)&pProperty);
		if (SUCCEEDED(hr))
		{
			CustomName name;
			wchar_t moniker_name[128] = {0};
			wchar_t friend_name[128] = { 0 };
			VARIANT varName;
			VariantInit(&varName);
			pProperty->Read(L"FriendlyName", &varName, NULL);
			StringCchCopy(friend_name, 128, varName.bstrVal);
			name.device_name = friend_name;
			wprintf(L"device name :%s\n", friend_name);
			name.pDev = pDev;
			alldevice.push_back(name);
			pProperty->Release();
		}
	}
	pEnumDev->Release();
}

IBaseFilter* GetDevice(IMoniker* pDev)
{
	IBaseFilter* device_filter = nullptr;
	HRESULT hr = pDev->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&device_filter);
	if (SUCCEEDED(hr))
	{
		return device_filter;
	}
	return nullptr;
}

int main()
{
	::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	IGraphBuilder* pGraph = NULL;
	HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraph);
	IBaseFilter* pSrc = NULL;
	VECDEVICES video_devices;
	EnumAllInputDevices(video_devices);
	if (video_devices.empty())
	{
		return 0;
	}
	pSrc =GetDevice(video_devices[0].pDev);
	pGraph->AddFilter(pSrc, L"video_caputre");
	IBaseFilter *pDest = NULL;
	ISampleGrabber *pGrabber = NULL;
	hr=CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pDest);
	if (FAILED(hr))
	{
		return -1;
	}
	hr = pDest->QueryInterface(IID_ISampleGrabber, (void**)&pGrabber);
	if (FAILED(hr))
	{
		return -1;
	}
	AM_MEDIA_TYPE media_type;
	memset(&media_type, 0, sizeof(AM_MEDIA_TYPE));

	/*VIDEOINFOHEADER vih;
	vih.bmiHeader.biWidth = 320;
	vih.bmiHeader.biHeight = 180;
	vih.bmiHeader.biSizeImage = vih.bmiHeader.biWidth * vih.bmiHeader.biHeight * 3;
	media_type.pbFormat = (BYTE *)(&vih);
	media_type.cbFormat = sizeof(VIDEOINFOHEADER);*/
	media_type.subtype = MEDIASUBTYPE_RGB24;
	media_type.majortype = MEDIATYPE_Video;
	media_type.formattype = FORMAT_VideoInfo;
	hr = pGrabber->SetMediaType(&media_type);
	hr=pGraph->AddFilter(pDest, L"video_grabber");


	hr= ConnectFilters(pGraph, pSrc, pDest);
	if (FAILED(hr))
	{
		return -1;
	}
	ISampleGrabberCB* sample_cb = new AudioGrabberSample();
	pGrabber->SetBufferSamples(FALSE);
	pGrabber->SetCallback(sample_cb,1);
	pGrabber->SetOneShot(FALSE);
	IMediaControl *pControl = NULL;
	hr =pGraph->QueryInterface(IID_IMediaControl, (void**)&pControl);
	
	if (SUCCEEDED(hr))
	{
		hr = pControl->Run();
		while (SUCCEEDED(hr))
		{
			::Sleep(10);
		}
	}
	::CoUninitialize();
	return 0;
}

HRESULT AddFilterByCLSID(IGraphBuilder *pGraph, const GUID& clsid, LPCWSTR wszName, IBaseFilter **ppF)
{
	if (!pGraph || !ppF)
	{
		return E_POINTER;
	}
	IBaseFilter* pf = nullptr;
	HRESULT hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pf);
	if (SUCCEEDED(hr))
	{
		hr = pGraph->AddFilter(pf, wszName);
		if (SUCCEEDED(hr))
		{
			*ppF = pf;
		}
		else
		{
			pf->Release();
		}
	}
	return hr;
}

HRESULT GetUnconnectPin(IBaseFilter* pFilter, PIN_DIRECTION PinDir, IPin** pPin)
{
	*pPin = nullptr;
	IEnumPins* pins=nullptr;
	HRESULT hr = pFilter->EnumPins(&pins);
	if (FAILED(hr))
	{
		return S_FALSE;
	}
	IPin* tmppin;
	while (pins->Next(1, &tmppin,NULL)==S_OK)
	{
		PIN_DIRECTION tmp_direction;
		tmppin->QueryDirection(&tmp_direction);
		if (tmp_direction == PinDir)
		{
			IPin* try_pin=nullptr;
			hr =tmppin->ConnectedTo(&try_pin);
			if (SUCCEEDED(hr))
			{
				try_pin->Release();
			}
			else
			{
				pins->Release();
				*pPin = tmppin;
				return S_OK;
			}
		}
		tmppin->Release();
	}
	pins->Release();
	return S_FALSE;
}

HRESULT ConnectFilters(IGraphBuilder* pGraph, IBaseFilter* pSrc, IBaseFilter* pDest)
{
	if (pGraph == NULL || pSrc == NULL || pDest == NULL)
	{
		return E_POINTER;
	}
	IPin* pOut=nullptr;
	HRESULT hr = GetUnconnectPin(pSrc, PINDIR_OUTPUT, &pOut);
	if (FAILED(hr))
	{
		return hr;
	}

	IAMStreamConfig* pCfg=nullptr;
	hr = pOut->QueryInterface(IID_IAMStreamConfig, (void**)&pCfg);
	if (FAILED(hr))
	{
		return hr;
	}
	AM_MEDIA_TYPE* pmt = nullptr;
	hr = pCfg->GetFormat(&pmt);
	if (FAILED(hr))
	{
		return hr;
	}
	int iCount = 0;
	int iSize = 0;
	pCfg->GetNumberOfCapabilities(&iCount, &iSize);

	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		// Use the video capabilities structure.
		for (int iFormat = 0; iFormat < iCount; iFormat++)
		{
			VIDEO_STREAM_CONFIG_CAPS scc;
				AM_MEDIA_TYPE *pmtConfig;
				hr = pCfg->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
				if (SUCCEEDED(hr))
				{
					/* Examine the format, and possibly use it. */
					// Delete the media type when you are done.
					
					printf("wdith:%d,height:%d\n", scc.InputSize.cx, scc.InputSize.cy);
					hr = pCfg->SetFormat(pmtConfig);//重新设置视频格式
					break;
				}
		}
	}
	pCfg->Release();
	IPin* pIn = nullptr;
	hr = GetUnconnectPin(pDest, PINDIR_INPUT, &pIn);
	if (FAILED(hr))
	{
		return hr;
	}

	hr =pGraph->Connect(pOut, pIn);

	return hr;
}