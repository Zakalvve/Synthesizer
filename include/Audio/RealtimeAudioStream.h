#include <windows.h>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <propvarutil.h>
#include <propidl.h>
#include <ks.h>
#include <ksmedia.h>
#include <iostream>
#include <cmath>

int test() {
    
    // Define PKEY_Device_FriendlyName manually
    static const PROPERTYKEY PKEY_Device_FriendlyName = { { 0xa45c254e, 0xdf1c, 0x4efd, { 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0 } }, 14 };
    HRESULT hr;
    
    // 1. Initialize COM library
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM library: " << std::hex << hr << std::endl;
        return -1;
    }
    
    // 2. Create device enumerator
    IMMDeviceEnumerator* pEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, 
                          IID_PPV_ARGS(&pEnumerator));
    if (FAILED(hr)) {
        std::cerr << "Failed to create device enumerator: " << std::hex << hr << std::endl;
        CoUninitialize();
        return -1;
    }
    
    // 3. Get default audio endpoint
    IMMDevice* pDevice = nullptr;
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (FAILED(hr)) {
        std::cerr << "Failed to get default audio endpoint: " << std::hex << hr << std::endl;
        pEnumerator->Release();
        CoUninitialize();
        return -1;
    }
    
    // 4. Get device friendly name
    IPropertyStore* pProps = nullptr;
    hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
    if (FAILED(hr)) {
        std::cerr << "Failed to open property store: " << std::hex << hr << std::endl;
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return -1;
    }
    
    PROPVARIANT varName;
    PropVariantInit(&varName);
    hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
    if (SUCCEEDED(hr)) {
        std::wcout << L"Default audio device: " << varName.pwszVal << std::endl;
    } else {
        std::cerr << "Failed to get device friendly name: " << std::hex << hr << std::endl;
    }
    PropVariantClear(&varName);
    pProps->Release();
    
    // 5. Activate audio client
    IAudioClient* pAudioClient = nullptr;
    hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, 
                           (void**)&pAudioClient);
    if (FAILED(hr)) {
        std::cerr << "Failed to activate audio client: " << std::hex << hr << std::endl;
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return -1;
    }
    
    // 6. Get mix format
    WAVEFORMATEX* pwfx = nullptr;
    hr = pAudioClient->GetMixFormat(&pwfx);
    if (FAILED(hr)) {
        std::cerr << "Failed to get mix format: " << std::hex << hr << std::endl;
        pAudioClient->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return -1;
    }
    
    // 7. Initialize audio client
    REFERENCE_TIME hnsRequestedDuration = 10000000; // 1 second
    hr = pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        0,
        hnsRequestedDuration,
        0,
        pwfx,
        NULL
    );
    
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize audio client: " << std::hex << hr << std::endl;
        CoTaskMemFree(pwfx);
        pAudioClient->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return -1;
    }
    
    // 8. Get buffer size
    UINT32 bufferFrameCount;
    hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    if (FAILED(hr)) {
        std::cerr << "Failed to get buffer size: " << std::hex << hr << std::endl;
        CoTaskMemFree(pwfx);
        pAudioClient->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return -1;
    }
    
    // 9. Get render client
    IAudioRenderClient* pRenderClient = nullptr;
    hr = pAudioClient->GetService(__uuidof(IAudioRenderClient), 
                                  (void**)&pRenderClient);
    if (FAILED(hr)) {
        std::cerr << "Failed to get render client: " << std::hex << hr << std::endl;
        CoTaskMemFree(pwfx);
        pAudioClient->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return -1;
    }
    
    // 10. Generate sine wave and fill buffer
    BYTE* pData;
    hr = pRenderClient->GetBuffer(bufferFrameCount, &pData);
    if (FAILED(hr)) {
        std::cerr << "Failed to get buffer: " << std::hex << hr << std::endl;
        pRenderClient->Release();
        CoTaskMemFree(pwfx);
        pAudioClient->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return -1;
    }
    
    // Generate sine wave
    const double frequency = 440.0; // A4 note
    const double amplitude = 0.5;   // Amplitude [0.0, 1.0]
    const double twoPi = 2 * M_PI;
    const double sampleRate = pwfx->nSamplesPerSec;
    const double delta = frequency * twoPi / sampleRate;
    
    if (pwfx->wBitsPerSample == 16) {
        short* pSample = (short*)pData;
        for (UINT32 i = 0; i < bufferFrameCount; ++i) {
            short value = (short)(amplitude * 32767 * sin(delta * i));
            for (UINT32 ch = 0; ch < pwfx->nChannels; ++ch) {
                *pSample++ = value;
            }
        }
    } else if (pwfx->wBitsPerSample == 32) {
        int* pSample = (int*)pData;
        for (UINT32 i = 0; i < bufferFrameCount; ++i) {
            int value = (int)(amplitude * 2147483647 * sin(delta * i));
            for (UINT32 ch = 0; ch < pwfx->nChannels; ++ch) {
                *pSample++ = value;
            }
        }
    } else {
        std::cerr << "Unsupported audio format" << std::endl;
        pRenderClient->ReleaseBuffer(0, AUDCLNT_BUFFERFLAGS_SILENT);
        pRenderClient->Release();
        CoTaskMemFree(pwfx);
        pAudioClient->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return -1;
    }
    
    hr = pRenderClient->ReleaseBuffer(bufferFrameCount, 0);
    if (FAILED(hr)) {
        std::cerr << "Failed to release buffer: " << std::hex << hr << std::endl;
        pRenderClient->Release();
        CoTaskMemFree(pwfx);
        pAudioClient->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return -1;
    }
    
    // 11. Start audio client
    hr = pAudioClient->Start();
    if (FAILED(hr)) {
        std::cerr << "Failed to start audio client: " << std::hex << hr << std::endl;
        pRenderClient->Release();
        CoTaskMemFree(pwfx);
        pAudioClient->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return -1;
    }
    
    // 12. Play for the duration of the buffer
    Sleep((DWORD)(hnsRequestedDuration / 10000));
    
    // 13. Stop audio client
    hr = pAudioClient->Stop();
    if (FAILED(hr)) {
        std::cerr << "Failed to stop audio client: " << std::hex << hr << std::endl;
    }
    
    // 14. Clean up
    pRenderClient->Release();
    CoTaskMemFree(pwfx);
    pAudioClient->Release();
    pDevice->Release();
    pEnumerator->Release();
    CoUninitialize();
}