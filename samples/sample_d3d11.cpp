
// ================================================================================================
// -*- C++ -*-
// File:   sample_d3d11.cpp
// Author: Guilherme R. Lampert
// Brief:  D3D11 Debug Draw sample. Also uses the newer explicit context API.
//
// This software is in the public domain. Where that dedication is not recognized,
// you are granted a perpetual, irrevocable license to copy, distribute, and modify
// this file as you see fit.
// ================================================================================================

#define DEBUG_DRAW_IMPLEMENTATION
#define DEBUG_DRAW_EXPLICIT_CONTEXT
#include "debug_draw.hpp"

#define DD_SAMPLES_NOGL
#include "samples_common.hpp"
using namespace ddSamplesCommon;

#include <cstdlib>
#include <tuple>

#define NOIME
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <ShellScalingAPI.h>
#include <windows.h>
#include <wrl.h>

#include <dxgi.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#pragma comment(lib, "Shcore")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "d3dcompiler")

using Microsoft::WRL::ComPtr;

// ========================================================
// Helper functions
// ========================================================

static void panicF(const char * format, ...)
{
    va_list args;
    char buffer[2048] = {'\0'};

    va_start(args, format);
    std::vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    MessageBoxA(nullptr, buffer, "Fatal Error", MB_OK);
    std::abort();
}

// ========================================================
// Window
// ========================================================

const wchar_t WindowTitle[] = L"Debug Draw - D3D11 Sample";
const wchar_t WindowClassName[] = L"DebugDrawD3D11";

class Window
{
public:

    HINSTANCE hInst   = nullptr;
    HWND      hWindow = nullptr;

    Window(HINSTANCE hInstance, int nCmdShow)
        : hInst(hInstance)
    {
        registerClass();
        createWindow(nCmdShow);
    }

    virtual ~Window()
    {
        if (hWindow != nullptr)
        {
            DestroyWindow(hWindow);
        }
        if (hInst != nullptr)
        {
            UnregisterClass(WindowClassName, hInst);
        }
    }

    void runMessageLoop()
    {
        MSG msg = {0};
        while (msg.message != WM_QUIT)
        {
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            onRender();
        }
    }

    virtual void onRender() { }

    //
    // DPI scaling helpers:
    //

    static const std::tuple<float, float> DpiXY;
    static const int WidthScaled;
    static const int HeightScaled;

    static int GetDpiAdjustedX(int size) { return static_cast<int>(size * std::get<0>(DpiXY) / 96.0f); }
    static int GetDpiAdjustedY(int size) { return static_cast<int>(size * std::get<1>(DpiXY) / 96.0f); }

    static std::tuple<float, float> GetDpiXY()
    {
        const POINT ptZero = {0,0};
        HMONITOR hm = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);

        UINT dpiX = 0, dpiY = 0;
        if (FAILED(GetDpiForMonitor(hm, MDT_EFFECTIVE_DPI, &dpiX, &dpiY)))
        {
            dpiX = dpiY = 192; // Some arbitrary default (2x scaling)
        }

        return {float(dpiX), float(dpiY)};
    }

private:

    void registerClass()
    {
        WNDCLASSEX wcex    = {0};
        wcex.cbSize        = sizeof(WNDCLASSEX);
        wcex.style         = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc   = &Window::WndProc;
        wcex.hInstance     = hInst;
        wcex.lpszClassName = WindowClassName;
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
        wcex.hIconSm       = LoadIcon(nullptr, IDI_APPLICATION);
        wcex.hCursor       = LoadCursor(nullptr, IDC_ARROW);
        RegisterClassEx(&wcex);
    }

    void createWindow(int nCmdShow)
    {
        hWindow = CreateWindow(WindowClassName, WindowTitle, WS_OVERLAPPEDWINDOW,
                               0, 0, WidthScaled, HeightScaled,
                               nullptr, nullptr, hInst, nullptr);
        if (hWindow == nullptr)
        {
            panicF("Failed to create application window!");
        }
        ShowWindow(hWindow, nCmdShow);
        UpdateWindow(hWindow);
    }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_KEYDOWN:
            if (wParam == VK_RETURN) { keys.showGrid   = !keys.showGrid;   }
            if (wParam == VK_SPACE)  { keys.showLabels = !keys.showLabels; }
            return 0;

        case WM_MOUSEMOVE:
            {
                int mx = LOWORD(lParam); 
                int my = HIWORD(lParam); 

                // Clamp to window bounds:
                if      (mx > WidthScaled)  { mx = WidthScaled;  }
                else if (mx < 0)            { mx = 0;            }
                if      (my > HeightScaled) { my = HeightScaled; }
                else if (my < 0)            { my = 0;            }

                mouse.deltaX = mx - mouse.lastPosX;
                mouse.deltaY = my - mouse.lastPosY;
                mouse.lastPosX = mx;
                mouse.lastPosY = my;

                // Clamp between -/+ max delta:
                if      (mouse.deltaX >  Mouse::MaxDelta) { mouse.deltaX =  Mouse::MaxDelta; }
                else if (mouse.deltaX < -Mouse::MaxDelta) { mouse.deltaX = -Mouse::MaxDelta; }
                if      (mouse.deltaY >  Mouse::MaxDelta) { mouse.deltaY =  Mouse::MaxDelta; }
                else if (mouse.deltaY < -Mouse::MaxDelta) { mouse.deltaY = -Mouse::MaxDelta; }
            }
            return 0;

        default:
            break;
        } // switch

        return DefWindowProc(hWnd, message, wParam, lParam);
    }
};

// Caching these for convenience
const std::tuple<float, float> Window::DpiXY = Window::GetDpiXY();
const int Window::WidthScaled  = Window::GetDpiAdjustedX(WindowWidth);
const int Window::HeightScaled = Window::GetDpiAdjustedY(WindowHeight);

// ========================================================
// RenderWindowD3D11
// ========================================================

class RenderWindowD3D11 final
    : public Window
{
public:

    ComPtr<IDXGISwapChain>         swapChain;
    ComPtr<ID3D11Device>           d3dDevice;
    ComPtr<ID3D11DeviceContext>    deviceContext;
    ComPtr<ID3D11RenderTargetView> renderTargetView;
    std::function<void()>          renderCallback;

    RenderWindowD3D11(HINSTANCE hInstance, int nCmdShow)
        : Window(hInstance, nCmdShow)
    {
        initD3D();
    }

    void onRender() override
    {
        const float clearColor[] = {0.2f, 0.2f, 0.2f, 1.0f};
        deviceContext->ClearRenderTargetView(renderTargetView.Get(), clearColor);

        if (renderCallback)
        {
            renderCallback();
        }

        swapChain->Present(0, 0);
    }

private:

    void initD3D()
    {
        UINT createDeviceFlags = 0;
        const UINT width  = Window::WidthScaled;
        const UINT height = Window::HeightScaled;

        // If the project is in a debug build, enable debugging via SDK Layers with this flag.
        #if defined(DEBUG) || defined(_DEBUG)
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        #endif // DEBUG

        // Acceptable driver types.
        const D3D_DRIVER_TYPE driverTypes[] = {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
        const UINT numDriverTypes = ARRAYSIZE(driverTypes);

        // This array defines the ordering of feature levels that D3D should attempt to create.
        const D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        const UINT numFeatureLevels = ARRAYSIZE(featureLevels);

        DXGI_SWAP_CHAIN_DESC sd               = {0};
        sd.BufferCount                        = 2;
        sd.BufferDesc.Width                   = width;
        sd.BufferDesc.Height                  = height;
        sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator   = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow                       = hWindow;
        sd.SampleDesc.Count                   = 1;
        sd.SampleDesc.Quality                 = 0;
        sd.Windowed                           = true;
        sd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;

        HRESULT hr;
        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

        // Try to create the device and swap chain:
        for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; ++driverTypeIndex)
        {
            auto driverType = driverTypes[driverTypeIndex];
            hr = D3D11CreateDeviceAndSwapChain(nullptr, driverType, nullptr, createDeviceFlags,
                                               featureLevels, numFeatureLevels, D3D11_SDK_VERSION,
                                               &sd, swapChain.GetAddressOf(), d3dDevice.GetAddressOf(),
                                               &featureLevel, deviceContext.GetAddressOf());

            if (hr == E_INVALIDARG)
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDeviceAndSwapChain(nullptr, driverType, nullptr, createDeviceFlags,
                                                   &featureLevels[1], numFeatureLevels - 1,
                                                   D3D11_SDK_VERSION, &sd, swapChain.GetAddressOf(),
                                                   d3dDevice.GetAddressOf(), &featureLevel,
                                                   deviceContext.GetAddressOf());
            }

            if (SUCCEEDED(hr))
            {
                break;
            }
        }

        if (FAILED(hr))
        {
            panicF("Failed to create D3D device or swap chain!");
        }

        // Create a render target view for the framebuffer:
        ID3D11Texture2D * backBuffer = nullptr;
        hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
        if (FAILED(hr))
        {
            panicF("Failed to get framebuffer from swap chain!");
        }

        hr = d3dDevice->CreateRenderTargetView(backBuffer, nullptr, renderTargetView.GetAddressOf());
        backBuffer->Release();
        if (FAILED(hr))
        {
            panicF("Failed to create RTV for framebuffer!");
        }

        deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), nullptr);

        // Setup the viewport:
        D3D11_VIEWPORT vp;
        vp.Width    = static_cast<float>(width);
        vp.Height   = static_cast<float>(height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        deviceContext->RSSetViewports(1, &vp);
    }
};

// ========================================================
// ShaderSetD3D11
// ========================================================

struct ShaderSetD3D11 final
{
    using InputLayoutDesc = std::tuple<const D3D11_INPUT_ELEMENT_DESC *, int>;

    ComPtr<ID3D11VertexShader> vs;
    ComPtr<ID3D11PixelShader>  ps;
    ComPtr<ID3D11InputLayout>  vertexLayout;

    ShaderSetD3D11() = default;

    void loadFromFxFile(ID3D11Device * device, const wchar_t * filename,
                        const char * vsEntry, const char * psEntry,
                        const InputLayoutDesc & layout)
    {
        assert(filename != nullptr && filename[0] != L'\0');
        assert(vsEntry  != nullptr && vsEntry[0]  != '\0');
        assert(psEntry  != nullptr && psEntry[0]  != '\0');

        ComPtr<ID3DBlob> vsBlob;
        compileShaderFromFile(filename, vsEntry, "vs_4_0", vsBlob.GetAddressOf());
        assert(vsBlob != nullptr);

        ComPtr<ID3DBlob> psBlob;
        compileShaderFromFile(filename, psEntry, "ps_4_0", psBlob.GetAddressOf());
        assert(psBlob != nullptr);

        HRESULT hr;

        // Create the vertex shader:
        hr = device->CreateVertexShader(vsBlob->GetBufferPointer(),
                                        vsBlob->GetBufferSize(), nullptr,
                                        vs.GetAddressOf());
        if (FAILED(hr))
        {
            panicF("Failed to create vertex shader '%s'", vsEntry);
        }

        // Create the pixel shader:
        hr = device->CreatePixelShader(psBlob->GetBufferPointer(),
                                       psBlob->GetBufferSize(), nullptr,
                                       ps.GetAddressOf());
        if (FAILED(hr))
        {
            panicF("Failed to create pixel shader '%s'", psEntry);
        }

        // Create vertex input layout:
        hr = device->CreateInputLayout(std::get<0>(layout), std::get<1>(layout),
                                       vsBlob->GetBufferPointer(),
                                       vsBlob->GetBufferSize(),
                                       vertexLayout.GetAddressOf());
        if (FAILED(hr))
        {
            panicF("Failed to create vertex layout!");
        }
    }

    static void compileShaderFromFile(const wchar_t * fileName, const char * entryPoint,
                                      const char * shaderModel, ID3DBlob ** ppBlobOut)
    {
        UINT shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

        // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
        // Setting this flag improves the shader debugging experience, but still allows
        // the shaders to be optimized and to run exactly the way they will run in
        // the release configuration.
        #if defined(DEBUG) || defined(_DEBUG)
        shaderFlags |= D3DCOMPILE_DEBUG;
        #endif // DEBUG

        ComPtr<ID3DBlob> pErrorBlob;
        HRESULT hr = D3DCompileFromFile(fileName, nullptr, nullptr, entryPoint, shaderModel,
                                        shaderFlags, 0, ppBlobOut, pErrorBlob.GetAddressOf());
        if (FAILED(hr))
        {
            auto * details = (pErrorBlob ? static_cast<const char *>(pErrorBlob->GetBufferPointer()) : "<no info>");
            panicF("Failed to compile shader! Error info: %s", details);
        }
    }
};

// ========================================================
// RenderInterfaceD3D11
// ========================================================

class RenderInterfaceD3D11 final
    : public dd::RenderInterface
{
public:

    RenderInterfaceD3D11(const ComPtr<ID3D11Device> & pDevice, const ComPtr<ID3D11DeviceContext> & pContext)
        : d3dDevice(pDevice)
        , deviceContext(pContext)
    {
        initShaders();
        initBuffers();
    }

    void setMvpMatrixPtr(const float * const mtx)
    {
        constantBufferData.mvpMatrix = DirectX::XMMATRIX(mtx);
    }

    void setCameraFrame(const Vector3 & up, const Vector3 & right, const Vector3 & origin)
    {
        camUp = up; camRight = right; camOrigin = origin;
    }

    //
    // dd::RenderInterface overrides:
    //

    void beginDraw() override
    {
        // Update and set the constant buffer for this frame
        deviceContext->UpdateSubresource(constantBuffer.Get(), 0, nullptr, &constantBufferData, 0, 0);
        deviceContext->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

        // Disable culling for the screen text
        deviceContext->RSSetState(rasterizerState.Get());
    }

    void endDraw() override
    {
        // No work done here at the moment.
    }

    dd::GlyphTextureHandle createGlyphTexture(int width, int height, const void * pixels) override
    {
        UINT numQualityLevels = 0;
        d3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8_UNORM, 1, &numQualityLevels);

        D3D11_TEXTURE2D_DESC tex2dDesc  = {};
        tex2dDesc.Usage                 = D3D11_USAGE_DEFAULT;
        tex2dDesc.BindFlags             = D3D11_BIND_SHADER_RESOURCE;
        tex2dDesc.Format                = DXGI_FORMAT_R8_UNORM;
        tex2dDesc.Width                 = width;
        tex2dDesc.Height                = height;
        tex2dDesc.MipLevels             = 1;
        tex2dDesc.ArraySize             = 1;
        tex2dDesc.SampleDesc.Count      = 1;
        tex2dDesc.SampleDesc.Quality    = numQualityLevels - 1;

        D3D11_SAMPLER_DESC samplerDesc  = {};
        samplerDesc.Filter              = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU            = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressV            = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressW            = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.ComparisonFunc      = D3D11_COMPARISON_ALWAYS;
        samplerDesc.MaxAnisotropy       = 1;
        samplerDesc.MipLODBias          = 0.0f;
        samplerDesc.MinLOD              = 0.0f;
        samplerDesc.MaxLOD              = D3D11_FLOAT32_MAX;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem                = pixels;
        initData.SysMemPitch            = width;

        auto * texImpl = new TextureImpl{};

        if (FAILED(d3dDevice->CreateTexture2D(&tex2dDesc, &initData, &texImpl->d3dTexPtr)))
        {
            errorF("CreateTexture2D failed!");
            destroyGlyphTexture(texImpl);
            return nullptr;
        }
        if (FAILED(d3dDevice->CreateShaderResourceView(texImpl->d3dTexPtr, nullptr, &texImpl->d3dTexSRV)))
        {
            errorF("CreateShaderResourceView failed!");
            destroyGlyphTexture(texImpl);
            return nullptr;
        }
        if (FAILED(d3dDevice->CreateSamplerState(&samplerDesc, &texImpl->d3dSampler)))
        {
            errorF("CreateSamplerState failed!");
            destroyGlyphTexture(texImpl);
            return nullptr;
        }

        return static_cast<dd::GlyphTextureHandle>(texImpl);
    }

    void destroyGlyphTexture(dd::GlyphTextureHandle glyphTex) override
    {
        auto * texImpl = static_cast<TextureImpl *>(glyphTex);
        if (texImpl)
        {
            if (texImpl->d3dSampler) { texImpl->d3dSampler->Release(); }
            if (texImpl->d3dTexSRV)  { texImpl->d3dTexSRV->Release();  }
            if (texImpl->d3dTexPtr)  { texImpl->d3dTexPtr->Release();  }
            delete texImpl;
        }
    }

    void drawGlyphList(const dd::DrawVertex * glyphs, int count, dd::GlyphTextureHandle glyphTex) override
    {
        assert(glyphs != nullptr);
        assert(count > 0 && count <= DEBUG_DRAW_VERTEX_BUFFER_SIZE);

        auto * texImpl = static_cast<TextureImpl *>(glyphTex);
        assert(texImpl != nullptr);

        // Map the vertex buffer:
        D3D11_MAPPED_SUBRESOURCE mapInfo;
        if (FAILED(deviceContext->Map(glyphVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapInfo)))
        {
            panicF("Failed to map vertex buffer!");
        }

        // Copy into mapped buffer:
        auto * verts = static_cast<Vertex *>(mapInfo.pData);
        for (int v = 0; v < count; ++v)
        {
            verts[v].pos.x = glyphs[v].glyph.x;
            verts[v].pos.y = glyphs[v].glyph.y;
            verts[v].pos.z = 0.0f;
            verts[v].pos.w = 1.0f;

            verts[v].uv.x = glyphs[v].glyph.u;
            verts[v].uv.y = glyphs[v].glyph.v;
            verts[v].uv.z = 0.0f;
            verts[v].uv.w = 0.0f;

            verts[v].color.x = glyphs[v].glyph.r;
            verts[v].color.y = glyphs[v].glyph.g;
            verts[v].color.z = glyphs[v].glyph.b;
            verts[v].color.w = 1.0f;
        }

        // Unmap and draw:
        deviceContext->Unmap(glyphVertexBuffer.Get(), 0);

        // Bind texture & sampler (t0, s0):
        deviceContext->PSSetShaderResources(0, 1, &texImpl->d3dTexSRV);
        deviceContext->PSSetSamplers(0, 1, &texImpl->d3dSampler);

        const float blendFactor[] = {1.0f, 1.0f, 1.0f, 1.0f};
        deviceContext->OMSetBlendState(blendStateText.Get(), blendFactor, 0xFFFFFFFF);

        // Draw with the current buffer:
        drawHelper(count, glyphShaders, glyphVertexBuffer.Get(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Restore default blend state.
        deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    }

    void drawPointList(const dd::DrawVertex * points, int count, bool depthEnabled) override
    {
        (void)depthEnabled; // TODO: not implemented yet - not required by this sample

        // Emulating points as billboarded quads, so each point will use 6 vertexes.
        // D3D11 doesn't support "point sprites" like OpenGL (gl_PointSize).
        const int maxVerts = DEBUG_DRAW_VERTEX_BUFFER_SIZE / 6;

        // OpenGL point size scaling produces gigantic points with the billboarding fallback.
        // This is some arbitrary down-scaling factor to more or less match the OpenGL samples.
        const float D3DPointSpriteScalingFactor = 0.01f;

        assert(points != nullptr);
        assert(count > 0 && count <= maxVerts);

       // Map the vertex buffer:
        D3D11_MAPPED_SUBRESOURCE mapInfo;
        if (FAILED(deviceContext->Map(pointVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapInfo)))
        {
            panicF("Failed to map vertex buffer!");
        }

        const int numVerts = count * 6;
        const int indexes[6] = {0, 1, 2, 2, 3, 0};

        int v = 0;
        auto * verts = static_cast<Vertex *>(mapInfo.pData);

        // Expand each point into a quad:
        for (int p = 0; p < count; ++p)
        {
            const float ptSize      = points[p].point.size * D3DPointSpriteScalingFactor;
            const Vector3 halfWidth = (ptSize * 0.5f) * camRight; // X
            const Vector3 halfHeigh = (ptSize * 0.5f) * camUp;    // Y
            const Vector3 origin    = Vector3{ points[p].point.x, points[p].point.y, points[p].point.z };

            Vector3 corners[4];
            corners[0] = origin + halfWidth + halfHeigh;
            corners[1] = origin - halfWidth + halfHeigh;
            corners[2] = origin - halfWidth - halfHeigh;
            corners[3] = origin + halfWidth - halfHeigh;

            for (int i : indexes)
            {
                verts[v].pos.x = corners[i].getX();
                verts[v].pos.y = corners[i].getY();
                verts[v].pos.z = corners[i].getZ();
                verts[v].pos.w = 1.0f;

                verts[v].color.x = points[p].point.r;
                verts[v].color.y = points[p].point.g;
                verts[v].color.z = points[p].point.b;
                verts[v].color.w = 1.0f;

                ++v;
            }
        }
        assert(v == numVerts);

        // Unmap and draw:
        deviceContext->Unmap(pointVertexBuffer.Get(), 0);

        // Draw with the current buffer:
        drawHelper(numVerts, pointShaders, pointVertexBuffer.Get(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    void drawLineList(const dd::DrawVertex * lines, int count, bool depthEnabled) override
    {
        (void)depthEnabled; // TODO: not implemented yet - not required by this sample

        assert(lines != nullptr);
        assert(count > 0 && count <= DEBUG_DRAW_VERTEX_BUFFER_SIZE);

        // Map the vertex buffer:
        D3D11_MAPPED_SUBRESOURCE mapInfo;
        if (FAILED(deviceContext->Map(lineVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapInfo)))
        {
            panicF("Failed to map vertex buffer!");
        }

        // Copy into mapped buffer:
        auto * verts = static_cast<Vertex *>(mapInfo.pData);
        for (int v = 0; v < count; ++v)
        {
            verts[v].pos.x = lines[v].line.x;
            verts[v].pos.y = lines[v].line.y;
            verts[v].pos.z = lines[v].line.z;
            verts[v].pos.w = 1.0f;

            verts[v].color.x = lines[v].line.r;
            verts[v].color.y = lines[v].line.g;
            verts[v].color.z = lines[v].line.b;
            verts[v].color.w = 1.0f;
        }

        // Unmap and draw:
        deviceContext->Unmap(lineVertexBuffer.Get(), 0);

        // Draw with the current buffer:
        drawHelper(count, lineShaders, lineVertexBuffer.Get(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    }

private:

    //
    // Local types:
    //

    struct ConstantBufferData
    {
        DirectX::XMMATRIX mvpMatrix        = DirectX::XMMatrixIdentity();
        DirectX::XMFLOAT4 screenDimensions = {float(WindowWidth), float(WindowHeight), 0.0f, 0.0f};
    };

    struct Vertex
    {
        DirectX::XMFLOAT4A pos;   // 3D position
        DirectX::XMFLOAT4A uv;    // Texture coordinates
        DirectX::XMFLOAT4A color; // RGBA float
    };

    struct TextureImpl : public dd::OpaqueTextureType
    {
        ID3D11Texture2D          * d3dTexPtr  = nullptr;
        ID3D11ShaderResourceView * d3dTexSRV  = nullptr;
        ID3D11SamplerState       * d3dSampler = nullptr;
    };

    //
    // Members:
    //

    ComPtr<ID3D11Device>          d3dDevice;
    ComPtr<ID3D11DeviceContext>   deviceContext;
    ComPtr<ID3D11RasterizerState> rasterizerState;
    ComPtr<ID3D11BlendState>      blendStateText;

    ComPtr<ID3D11Buffer>          constantBuffer;
    ConstantBufferData            constantBufferData;

    ComPtr<ID3D11Buffer>          lineVertexBuffer;
    ShaderSetD3D11                lineShaders;

    ComPtr<ID3D11Buffer>          pointVertexBuffer;
    ShaderSetD3D11                pointShaders;

    ComPtr<ID3D11Buffer>          glyphVertexBuffer;
    ShaderSetD3D11                glyphShaders;

    // Camera vectors for the emulated point sprites
    Vector3                       camUp     = Vector3{0.0f};
    Vector3                       camRight  = Vector3{0.0f};
    Vector3                       camOrigin = Vector3{0.0f};

    void initShaders()
    {
        // Same vertex format used by all buffers to simplify things.
        const D3D11_INPUT_ELEMENT_DESC layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        const ShaderSetD3D11::InputLayoutDesc inputDesc{ layout, int(ARRAYSIZE(layout)) };

        // 3D lines shader:
        lineShaders.loadFromFxFile(d3dDevice.Get(), L"ddShader.fx",
                                   "VS_LinePoint", "PS_LinePoint", inputDesc);

        // 3D points shader:
        pointShaders.loadFromFxFile(d3dDevice.Get(), L"ddShader.fx",
                                    "VS_LinePoint", "PS_LinePoint", inputDesc);

        // 2D glyphs shader:
        glyphShaders.loadFromFxFile(d3dDevice.Get(), L"ddShader.fx",
                                    "VS_TextGlyph", "PS_TextGlyph", inputDesc);

        // Rasterizer state for the screen text:
        D3D11_RASTERIZER_DESC rsDesc = {};
        rsDesc.FillMode              = D3D11_FILL_SOLID;
        rsDesc.CullMode              = D3D11_CULL_NONE;
        rsDesc.FrontCounterClockwise = true;
        rsDesc.DepthBias             = 0;
        rsDesc.DepthBiasClamp        = 0.0f;
        rsDesc.SlopeScaledDepthBias  = 0.0f;
        rsDesc.DepthClipEnable       = false;
        rsDesc.ScissorEnable         = false;
        rsDesc.MultisampleEnable     = false;
        rsDesc.AntialiasedLineEnable = false;
        if (FAILED(d3dDevice->CreateRasterizerState(&rsDesc, rasterizerState.GetAddressOf())))
        {
            errorF("CreateRasterizerState failed!");
        }

        // Blend state for the screen text:
        D3D11_BLEND_DESC bsDesc                      = {};
        bsDesc.RenderTarget[0].BlendEnable           = true;
        bsDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        bsDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
        bsDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
        bsDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
        bsDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
        bsDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
        bsDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
        if (FAILED(d3dDevice->CreateBlendState(&bsDesc, blendStateText.GetAddressOf())))
        {
            errorF("CreateBlendState failed!");
        }
    }

    void initBuffers()
    {
        D3D11_BUFFER_DESC bd;

        // Create the shared constant buffer:
        bd                = {};
        bd.Usage          = D3D11_USAGE_DEFAULT;
        bd.ByteWidth      = sizeof(ConstantBufferData);
        bd.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0;
        if (FAILED(d3dDevice->CreateBuffer(&bd, nullptr, constantBuffer.GetAddressOf())))
        {
            panicF("Failed to create shader constant buffer!");
        }

        // Create the vertex buffers for lines/points/glyphs:
        bd                = {};
        bd.Usage          = D3D11_USAGE_DYNAMIC;
        bd.ByteWidth      = sizeof(Vertex) * DEBUG_DRAW_VERTEX_BUFFER_SIZE;
        bd.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        if (FAILED(d3dDevice->CreateBuffer(&bd, nullptr, lineVertexBuffer.GetAddressOf())))
        {
            panicF("Failed to create lines vertex buffer!");
        }
        if (FAILED(d3dDevice->CreateBuffer(&bd, nullptr, pointVertexBuffer.GetAddressOf())))
        {
            panicF("Failed to create points vertex buffer!");
        }
        if (FAILED(d3dDevice->CreateBuffer(&bd, nullptr, glyphVertexBuffer.GetAddressOf())))
        {
            panicF("Failed to create glyphs vertex buffer!");
        }
    }

    void drawHelper(const int numVerts, const ShaderSetD3D11 & ss,
                    ID3D11Buffer * vb, const D3D11_PRIMITIVE_TOPOLOGY topology)
    {
        const UINT offset = 0;
        const UINT stride = sizeof(Vertex);
        deviceContext->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        deviceContext->IASetPrimitiveTopology(topology);
        deviceContext->IASetInputLayout(ss.vertexLayout.Get());
        deviceContext->VSSetShader(ss.vs.Get(), nullptr, 0);
        deviceContext->PSSetShader(ss.ps.Get(), nullptr, 0);
        deviceContext->Draw(numVerts, 0);
    }
};

// ========================================================
// Sample drawing
// ========================================================

static void drawGrid(dd::ContextHandle ctx)
{
    if (!keys.showGrid)
    {
        return;
    }

    // Grid from -50 to +50 in both X & Z
    dd::xzSquareGrid(ctx, -50.0f, 50.0f, -1.0f, 1.7f, dd::colors::Green);
}

static void drawLabel(dd::ContextHandle ctx, ddVec3_In pos, const char * name)
{
    if (!keys.showLabels)
    {
        return;
    }

    // Only draw labels inside the camera frustum.
    if (camera.isPointInsideFrustum(pos[0], pos[1], pos[2]))
    {
        const ddVec3 textColor = { 0.8f, 0.8f, 1.0f };
        dd::projectedText(ctx, name, pos, textColor, toFloatPtr(camera.vpMatrix),
                          0, 0, WindowWidth, WindowHeight, 0.5f);
    }
}

static void drawMiscObjects(dd::ContextHandle ctx)
{
    // Start a row of objects at this position:
    ddVec3 origin = { -15.0f, 0.0f, 0.0f };

    // Box with a point at it's center:
    drawLabel(ctx, origin, "box");
    dd::box(ctx, origin, dd::colors::Blue, 1.5f, 1.5f, 1.5f);
    dd::point(ctx, origin, dd::colors::White, 15.0f);
    origin[0] += 3.0f;

    // Sphere with a point at its center
    drawLabel(ctx, origin, "sphere");
    dd::sphere(ctx, origin, dd::colors::Red, 1.0f);
    dd::point(ctx, origin, dd::colors::White, 15.0f);
    origin[0] += 4.0f;

    // Two cones, one open and one closed:
    const ddVec3 condeDir = { 0.0f, 2.5f, 0.0f };
    origin[1] -= 1.0f;

    drawLabel(ctx, origin, "cone (open)");
    dd::cone(ctx, origin, condeDir, dd::colors::Yellow, 1.0f, 2.0f);
    dd::point(ctx, origin, dd::colors::White, 15.0f);
    origin[0] += 4.0f;

    drawLabel(ctx, origin, "cone (closed)");
    dd::cone(ctx, origin, condeDir, dd::colors::Cyan, 0.0f, 1.0f);
    dd::point(ctx, origin, dd::colors::White, 15.0f);
    origin[0] += 4.0f;

    // Axis-aligned bounding box:
    const ddVec3 bbMins = { -1.0f, -0.9f, -1.0f };
    const ddVec3 bbMaxs = {  1.0f,  2.2f,  1.0f };
    const ddVec3 bbCenter = {
        (bbMins[0] + bbMaxs[0]) * 0.5f,
        (bbMins[1] + bbMaxs[1]) * 0.5f,
        (bbMins[2] + bbMaxs[2]) * 0.5f
    };
    drawLabel(ctx, origin, "AABB");
    dd::aabb(ctx, bbMins, bbMaxs, dd::colors::Orange);
    dd::point(ctx, bbCenter, dd::colors::White, 15.0f);

    // Move along the Z for another row:
    origin[0] = -15.0f;
    origin[2] += 5.0f;

    // A big arrow pointing up:
    const ddVec3 arrowFrom = { origin[0], origin[1], origin[2] };
    const ddVec3 arrowTo   = { origin[0], origin[1] + 5.0f, origin[2] };
    drawLabel(ctx, arrowFrom, "arrow");
    dd::arrow(ctx, arrowFrom, arrowTo, dd::colors::Magenta, 1.0f);
    dd::point(ctx, arrowFrom, dd::colors::White, 15.0f);
    dd::point(ctx, arrowTo, dd::colors::White, 15.0f);
    origin[0] += 4.0f;

    // Plane with normal vector:
    const ddVec3 planeNormal = { 0.0f, 1.0f, 0.0f };
    drawLabel(ctx, origin, "plane");
    dd::plane(ctx, origin, planeNormal, dd::colors::Yellow, dd::colors::Blue, 1.5f, 1.0f);
    dd::point(ctx, origin, dd::colors::White, 15.0f);
    origin[0] += 4.0f;

    // Circle on the Y plane:
    drawLabel(ctx, origin, "circle");
    dd::circle(ctx, origin, planeNormal, dd::colors::Orange, 1.5f, 15.0f);
    dd::point(ctx, origin, dd::colors::White, 15.0f);
    origin[0] += 3.2f;

    // Tangent basis vectors:
    const ddVec3 normal    = { 0.0f, 1.0f, 0.0f };
    const ddVec3 tangent   = { 1.0f, 0.0f, 0.0f };
    const ddVec3 bitangent = { 0.0f, 0.0f, 1.0f };
    origin[1] += 0.1f;
    drawLabel(ctx, origin, "tangent basis");
    dd::tangentBasis(ctx, origin, normal, tangent, bitangent, 2.5f);
    dd::point(ctx, origin, dd::colors::White, 15.0f);

    // And a set of intersecting axes:
    origin[0] += 4.0f;
    origin[1] += 1.0f;
    drawLabel(ctx, origin, "cross");
    dd::cross(ctx, origin, 2.0f);
    dd::point(ctx, origin, dd::colors::White, 15.0f);
}

static void drawFrustum(dd::ContextHandle ctx)
{
    const ddVec3 color  = {  0.8f, 0.3f, 1.0f  };
    const ddVec3 origin = { -8.0f, 0.5f, 14.0f };
    drawLabel(ctx, origin, "frustum + axes");

    // The frustum will depict a fake camera:
    const Matrix4 proj = Matrix4::perspective(degToRad(45.0f), 800.0f / 600.0f, 0.5f, 4.0f);
    const Matrix4 view = Matrix4::lookAt(Point3(-8.0f, 0.5f, 14.0f), Point3(-8.0f, 0.5f, -14.0f), Vector3::yAxis());
    const Matrix4 clip = inverse(proj * view);
    dd::frustum(ctx, toFloatPtr(clip), color);

    // A white dot at the eye position:
    dd::point(ctx, origin, dd::colors::White, 15.0f);

    // A set of arrows at the camera's origin/eye:
    const Matrix4 transform = Matrix4::translation(Vector3(-8.0f, 0.5f, 14.0f)) * Matrix4::rotationZ(degToRad(60.0f));
    dd::axisTriad(ctx, toFloatPtr(transform), 0.3f, 2.0f);
}

static void drawText(dd::ContextHandle ctx)
{
    // HUD text:
    const ddVec3 textColor = { 1.0f,  1.0f,  1.0f };
    const ddVec3 textPos2D = { 10.0f, 15.0f, 0.0f };
    dd::screenText(ctx, "Welcome to the D3D11 Debug Draw demo.\n\n"
                        "[SPACE]  to toggle labels on/off\n"
                        "[RETURN] to toggle grid on/off",
                        textPos2D, textColor, 0.55f);
}

static void inputUpdate(const Window & win)
{
    if (GetForegroundWindow() != win.hWindow)
    {
        return; // Don't update if window out of focus.
    }

    auto testKeyPressed = [](int key) -> bool
    {
        return (GetKeyState(key) & 0x80) > 0;
    };

    // Keyboard movement keys:
    keys.wDown = testKeyPressed('W') || testKeyPressed(VK_UP);
    keys.sDown = testKeyPressed('S') || testKeyPressed(VK_DOWN);
    keys.aDown = testKeyPressed('A') || testKeyPressed(VK_LEFT);
    keys.dDown = testKeyPressed('D') || testKeyPressed(VK_RIGHT);

    // Mouse buttons:
    mouse.leftButtonDown  = testKeyPressed(VK_LBUTTON);
    mouse.rightButtonDown = testKeyPressed(VK_RBUTTON);
}

// ========================================================
// WinMain
// ========================================================

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow)
{
    printDDBuildConfig();

    RenderWindowD3D11 renderWindow(hInstance, nCmdShow);
    RenderInterfaceD3D11 renderInterface(renderWindow.d3dDevice, renderWindow.deviceContext);

    dd::ContextHandle ddContext = nullptr;
    dd::initialize(&ddContext, &renderInterface);

    renderWindow.renderCallback = [ddContext, &renderWindow, &renderInterface]()
    {
        const double t0s = getTimeSeconds();

        inputUpdate(renderWindow);
        camera.checkKeyboardMovement();
        camera.checkMouseRotation();
        camera.updateMatrices();

        const Matrix4 mvpMatrix = transpose(camera.vpMatrix);
        renderInterface.setMvpMatrixPtr(toFloatPtr(mvpMatrix));
        renderInterface.setCameraFrame(camera.up, camera.right, camera.eye);

        // Call some DD functions to add stuff to the debug draw queues:
        drawGrid(ddContext);
        drawMiscObjects(ddContext);
        drawFrustum(ddContext);
        drawText(ddContext);

        // Flush the draw queues:
        dd::flush(ddContext);

        const double t1s = getTimeSeconds();

        deltaTime.seconds      = static_cast<float>(t1s - t0s);
        deltaTime.milliseconds = static_cast<std::int64_t>(deltaTime.seconds * 1000.0);
    };

    renderWindow.runMessageLoop();
    dd::shutdown(ddContext);
    return 0;
}

// ========================================================
