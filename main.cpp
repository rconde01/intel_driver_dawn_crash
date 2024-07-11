// Taken from https://gist.github.com/d7samurai/261c69490cce0620d0bfc93003cd1052 to quick
// start dx11
// WIN32 window taken from http://www.directxtutorial.com/Lesson.aspx?lessonid=9-1-3
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include <windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
   switch (msg) {
      case WM_DESTROY:
         PostQuitMessage(0);
         return 0;
   }
   return DefWindowProc(hWnd, msg, wParam, lParam);
}

char const* TITLE      = "too-many-indirect-calls";
int const   WIDTH      = 800;
int const   HEIGHT     = 600;
int const   DRAW_COUNT = 10000;

#define CHECK(X) assert(SUCCEEDED(X))

std::string const SHADER_SRC = R"(
float4 VSMain(uint vertex_index : SV_VertexID) : SV_POSITION {
   const float2 verts[3] = {float2(0.0f, 0.5f), (-0.5f).xx, float2(0.5f, -0.5f)};

   return float4(verts[vertex_index], 0.0, 1.0);
}

float4 PSMain() : SV_TARGET {
   return float4(1, 0, 0, 1);
}
)";

int main(int argc, char** argv) {
   // the handle for the window, filled by a function
   HWND       hWnd;
   // this struct holds information for the window class
   WNDCLASSEX wc{};

   // clear out the window class for use
   // ZeroMemory(&wc, sizeof(WNDCLASSEX));

   auto hInstance   = GetModuleHandle(nullptr);
   // fill in the struct with the needed information
   wc.cbSize        = sizeof(WNDCLASSEX);
   wc.style         = CS_HREDRAW | CS_VREDRAW;
   wc.lpfnWndProc   = WndProc;
   wc.hInstance     = hInstance;
   wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
   wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
   wc.lpszClassName = "WindowClass1";

   // register the window class
   RegisterClassEx(&wc);

   // create the window and use the result as the handle
   hWnd = CreateWindowEx(
      NULL,
      "WindowClass1",                 // name of the window class
      "Our First Windowed Program",   // title of the window
      WS_OVERLAPPEDWINDOW,            // window style
      300,                            // x-position of the window
      300,                            // y-position of the window
      WIDTH,                          // width of the window
      HEIGHT,                         // height of the window
      NULL,                           // we have no parent window, NULL
      NULL,                           // we aren't using menus, NULL
      hInstance,                      // application handle
      NULL);                          // used with multiple windows, NULL

   // display the window on the screen
   ShowWindow(hWnd, SW_SHOW);

   ///////////////////////////////////////////////////////////////////////////////////////////////

   D3D_FEATURE_LEVEL featurelevels[] = {D3D_FEATURE_LEVEL_11_0};

   DXGI_SWAP_CHAIN_DESC swapchaindesc = {};
   swapchaindesc.BufferDesc.Width     = 0;   // use window width
   swapchaindesc.BufferDesc.Height    = 0;   // use window height
   swapchaindesc.BufferDesc.Format    = DXGI_FORMAT_B8G8R8A8_UNORM;

   swapchaindesc.SampleDesc.Count = 1;
   swapchaindesc.BufferUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT;
   swapchaindesc.BufferCount      = 2;
   swapchaindesc.OutputWindow     = hWnd;
   swapchaindesc.Windowed         = TRUE;
   swapchaindesc.SwapEffect       = DXGI_SWAP_EFFECT_FLIP_DISCARD;

   IDXGISwapChain* swapchain;

   ID3D11Device*        device;
   ID3D11DeviceContext* context;

   D3D11CreateDeviceAndSwapChain(
      nullptr,
      D3D_DRIVER_TYPE_HARDWARE,
      nullptr,
      D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
      featurelevels,
      ARRAYSIZE(featurelevels),
      D3D11_SDK_VERSION,
      &swapchaindesc,
      &swapchain,
      &device,
      nullptr,
      &context);   // D3D11_CREATE_DEVICE_DEBUG is optional, but provides useful
                   // d3d11 debug output

   swapchain->GetDesc(&swapchaindesc);   // update swapchaindesc with actual window size

   ///////////////////////////////////////////////////////////////////////////////////////////////

   ID3D11Texture2D* framebuffer;

   swapchain->GetBuffer(
      0,
      __uuidof(ID3D11Texture2D),
      (void**)&framebuffer);   // grab framebuffer from swapchain

   D3D11_RENDER_TARGET_VIEW_DESC framebufferRTVdesc = {};
   framebufferRTVdesc.Format                        = DXGI_FORMAT_B8G8R8A8_UNORM;
   framebufferRTVdesc.ViewDimension                 = D3D11_RTV_DIMENSION_TEXTURE2D;

   ID3D11RenderTargetView* framebufferRTV;

   device->CreateRenderTargetView(framebuffer, &framebufferRTVdesc, &framebufferRTV);

   ///////////////////////////////////////////////////////////////////////////////////////////////

   D3D11_TEXTURE2D_DESC depthbufferdesc;

   framebuffer->GetDesc(
      &depthbufferdesc);   // copy framebuffer properties; they're mostly the same

   depthbufferdesc.Format    = DXGI_FORMAT_D16_UNORM;
   depthbufferdesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

   ID3D11Texture2D* depthbuffer;

   device->CreateTexture2D(&depthbufferdesc, nullptr, &depthbuffer);

   ID3D11DepthStencilView* depthbufferDSV;

   device->CreateDepthStencilView(depthbuffer, nullptr, &depthbufferDSV);

   ///////////////////////////////////////////////////////////////////////////////////////////////

   ID3DBlob* vertexshaderCSO;
   ID3DBlob* vs_errors;

   if (FAILED(D3DCompile(
          SHADER_SRC.data(),
          SHADER_SRC.size(),
          nullptr,
          nullptr,
          nullptr,
          "VSMain",
          "vs_5_0",
          0,
          0,
          &vertexshaderCSO,
          &vs_errors)))
   {
      std::cout << "VS errors: " << (char const*)vs_errors->GetBufferPointer() << "\n";
   }
   ID3DBlob* pixelshader_cso;
   ID3DBlob* ps_errors;
   if (FAILED(D3DCompile(
          SHADER_SRC.data(),
          SHADER_SRC.size(),
          nullptr,
          nullptr,
          nullptr,
          "PSMain",
          "ps_5_0",
          0,
          0,
          &pixelshader_cso,
          &ps_errors)))
   {
      std::cout << "PS errors: " << (char const*)ps_errors->GetBufferPointer() << "\n";
   }

   ID3D11VertexShader* vs;
   ID3D11PixelShader*  ps;

   device->CreateVertexShader(
      vertexshaderCSO->GetBufferPointer(),
      vertexshaderCSO->GetBufferSize(),
      nullptr,
      &vs);

   device->CreatePixelShader(
      pixelshader_cso->GetBufferPointer(),
      pixelshader_cso->GetBufferSize(),
      nullptr,
      &ps);

   D3D11_VIEWPORT viewport = {
      .TopLeftX = 0.0f,
      .TopLeftY = 0.0f,
      .Width    = (float)WIDTH,
      .Height   = (float)HEIGHT,
      .MinDepth = 0.0f,
      .MaxDepth = 1.0f,
   };

   D3D11_RASTERIZER_DESC rasterizer_desc = {};
   rasterizer_desc.FillMode              = D3D11_FILL_SOLID;
   rasterizer_desc.CullMode              = D3D11_CULL_BACK;
   rasterizer_desc.FrontCounterClockwise = true;

   ID3D11RasterizerState* rasterizer_state;
   device->CreateRasterizerState(&rasterizer_desc, &rasterizer_state);

   D3D11_DEPTH_STENCIL_DESC depthstencildesc = {};
   depthstencildesc.DepthEnable              = TRUE;
   depthstencildesc.DepthWriteMask           = D3D11_DEPTH_WRITE_MASK_ALL;
   depthstencildesc.DepthFunc                = D3D11_COMPARISON_LESS;

   ID3D11DepthStencilState* depthstencilstate;

   device->CreateDepthStencilState(&depthstencildesc, &depthstencilstate);

   ID3D11Buffer* indirect_draw_buffer;

   std::vector<D3D11_DRAW_INSTANCED_INDIRECT_ARGS> draws(DRAW_COUNT);

   for (auto& arg : draws) {
      arg = {
         .VertexCountPerInstance = 3,
         .InstanceCount          = 1,
         .StartVertexLocation    = 0,
         .StartInstanceLocation  = 0,
      };
   }

   D3D11_BUFFER_DESC indirect_desc = {
      .ByteWidth      = (UINT)draws.size() * sizeof(D3D11_DRAW_INSTANCED_INDIRECT_ARGS),
      .Usage          = D3D11_USAGE_DEFAULT,
      .BindFlags      = 0,
      .CPUAccessFlags = 0,
      .MiscFlags      = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS,
      .StructureByteStride = 0,
   };

   D3D11_SUBRESOURCE_DATA indirect_data = {
      .pSysMem          = draws.data(),
      .SysMemPitch      = 0,
      .SysMemSlicePitch = 0,
   };

   device->CreateBuffer(&indirect_desc, &indirect_data, &indirect_draw_buffer);

   context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

   context->VSSetShader(vs, nullptr, 0);

   context->RSSetViewports(1, &viewport);
   context->RSSetState(rasterizer_state);

   context->PSSetShader(ps, nullptr, 0);

   context->OMSetRenderTargets(1, &framebufferRTV, depthbufferDSV);
   context->OMSetDepthStencilState(depthstencilstate, 0);
   context->OMSetBlendState(nullptr, nullptr, 0xffffffff);

   MSG msg = {0};

   size_t i = 0;

   while (WM_QUIT != msg.message) {
      if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
      else {
         FLOAT clear_color[4] = {(sinf(i / float(120)) + 1.0f)/2.0f, 0.025f, 0.025f, 1.0f};

         context->OMSetRenderTargets(1, &framebufferRTV, depthbufferDSV);
         context->ClearRenderTargetView(framebufferRTV, clear_color);
         context->ClearDepthStencilView(depthbufferDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

         for (int i = 0; i < DRAW_COUNT; i++) {
            context->DrawInstancedIndirect(
               indirect_draw_buffer, i * sizeof(D3D11_DRAW_INSTANCED_INDIRECT_ARGS));
         }
         swapchain->Present(1, 0);

         i++;
      }
   }

   return 0;
}