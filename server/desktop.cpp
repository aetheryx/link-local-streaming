#pragma comment(lib,"d3d11.lib")

#include <iostream>
#include <dxgi1_2.h>
#include <d3d11.h>

const D3D_FEATURE_LEVEL FEATURE_LEVELS[] = { D3D_FEATURE_LEVEL_11_0 };

class Desktop {
  public:
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* deviceCtx = nullptr;
    IDXGIOutputDuplication* duplication = nullptr;

    HRESULT init();
};


HRESULT Desktop::init() {
  HRESULT hr = S_OK;
  #define ASSERT_HR(msg) \
    if (FAILED(hr)) { \
      printf("failed to %s (hr: %X)\n", msg, hr); \
      return hr; \
    }

  D3D_FEATURE_LEVEL featureLevel;

  hr = D3D11CreateDevice(
      nullptr,
      D3D_DRIVER_TYPE_HARDWARE,
      nullptr,
      0,
      FEATURE_LEVELS, ARRAYSIZE(FEATURE_LEVELS),
      D3D11_SDK_VERSION,
      &device,
      &featureLevel,
      &deviceCtx
  );
  ASSERT_HR("create device");

  IDXGIDevice* dxgiDevice = nullptr;
  hr = device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
  ASSERT_HR("query IDXGIDevice");

  IDXGIAdapter* dxgiAdapter = nullptr;
  hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&dxgiAdapter));
  ASSERT_HR("query IDXGIAdapter");

  IDXGIOutput* dxgiOutput = nullptr;
  hr = dxgiAdapter->EnumOutputs(0, &dxgiOutput);
  ASSERT_HR("enumerate IDXGIOutput");

  IDXGIOutput1* dxgiOutputInner = nullptr;
  hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&dxgiOutputInner));
  ASSERT_HR("query IDXGIOutput1");

  hr = dxgiOutputInner->DuplicateOutput(device, &duplication);
  ASSERT_HR("duplicate");

  dxgiDevice->Release();
  dxgiAdapter->Release();
  dxgiOutput->Release();
  dxgiOutputInner->Release();

  #undef ASSERT_HR

  return hr;
}