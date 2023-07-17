#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

#include "desktop.cpp"
#include <iostream>
#include <fstream>

D3D11_MAPPED_SUBRESOURCE subresource;

DWORD WINAPI server(void* data) {
  WSADATA wsaData;
  int iResult;

  SOCKET ListenSocket = INVALID_SOCKET;
  sockaddr_in service;

  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0) {
    printf("WSAStartup failed with error: %d\n", iResult);
    return 1;
  }

  ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (ListenSocket == INVALID_SOCKET) {
    printf("socket failed with error: %ld\n", WSAGetLastError());
    WSACleanup();
    return 1;
  }

  service.sin_family = AF_INET;
  service.sin_addr.s_addr = inet_addr("0.0.0.0");
  service.sin_port = htons(27015);

  iResult = bind(ListenSocket, (SOCKADDR *) &service, sizeof (service));
  if (iResult == SOCKET_ERROR) {
    printf("bind failed with error: %d\n", WSAGetLastError());
    closesocket(ListenSocket);
    WSACleanup();
    return 1;
  }

  iResult = listen(ListenSocket, SOMAXCONN);
  if (iResult == SOCKET_ERROR) {
    printf("listen failed with error: %d\n", WSAGetLastError());
    closesocket(ListenSocket);
    WSACleanup();
    return 1;
  }

  // Accept a client socket
  while (true) {
    printf("creating new socket\n");
    SOCKET ClientSocket = INVALID_SOCKET;
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
      printf("accept failed with error: %d\n", WSAGetLastError());
      closesocket(ListenSocket);
      return 1;
    }

    printf("received client\n");

    int iSendResult;
    printf("Bytes received: %d\n", iResult);

    while (true) {
      printf("starting send\n");
      iSendResult = send(
        ClientSocket, 
        reinterpret_cast<char*>(subresource.pData), 
        subresource.DepthPitch, 
        0
      );
      printf("finishing send\n");

      if (iSendResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        break;
      }
      Sleep(100);
      printf("Bytes sent: %d\n", iSendResult);
    }

    shutdown(ClientSocket, SD_SEND);
    closesocket(ClientSocket);
  }
}

int main() {
  HRESULT hr = S_OK;
  #define ASSERT_HR(msg) \
    if (FAILED(hr)) { \
      printf("%s (hr: %X)\n", msg, hr); \
      return 1; \
    }

  Desktop desktop;
  hr = desktop.init();
  ASSERT_HR("desktop init");

  ID3D11Texture2D* destFrame = nullptr;
  D3D11_TEXTURE2D_DESC destFrameDesc = {
    .Width = 3440,
    .Height = 1440,
    .MipLevels = 1,
    .ArraySize = 1,
    .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
    .SampleDesc = { .Count = 1, .Quality = 0 },
    .Usage = D3D11_USAGE_STAGING,
    .BindFlags = 0,
    .CPUAccessFlags = D3D11_CPU_ACCESS_READ,
    .MiscFlags = 0,
  };
  hr = desktop.device->CreateTexture2D(&destFrameDesc, nullptr, &destFrame);
  ASSERT_HR("create dest frame");

  hr = desktop.deviceCtx->Map(
    destFrame, 0,
    D3D11_MAP_READ, 0,
    &subresource
  );
  ASSERT_HR("create frame mapping");

  CreateThread(NULL, 0, server, NULL, 0, NULL);

  // frame loop
  while (true) {
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    IDXGIResource* desktopResource = nullptr;

    hr = desktop.duplication->AcquireNextFrame(1000, &frameInfo, &desktopResource);
    if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
      continue;
    }

    ASSERT_HR("acquire frame");

    ID3D11Texture2D* srcFrame = nullptr;
    hr = desktopResource->QueryInterface(
      __uuidof(ID3D11Texture2D),
      reinterpret_cast<void**>(&srcFrame));
    ASSERT_HR("extract source texture");

    desktop.deviceCtx->CopyResource(destFrame, srcFrame);

    desktopResource->Release();
    desktop.duplication->ReleaseFrame();
  }

  return 0;
}
