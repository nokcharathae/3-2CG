// tutorial.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "tutorial.h"

#include <windowsx.h>
#include <stdio.h>

#include <d3d11_1.h>
#include <directxcolors.h> // directX helper
#include <d3dcompiler.h> // shader compile

#include <directxmath.h>
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;
// DirectX::SimpleMath::Vector3 test; �̷��� �����ؾ������� namespace�� �����������ν�
// Vector3 test; �̷��� �����ϰ� �ۼ� ����

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE g_hInst;                                // current instance
WCHAR g_szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR g_szWindowClass[MAX_LOADSTRING];            // the main window class name

// for D3D setting
HWND g_hWnd;

// D3D Variables
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device* g_pd3dDevice = nullptr; // NULL�� �ᵵ ������ pointer��� ���� ����ϱ� ����
ID3D11DeviceContext* g_pImmediateContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;

ID3D11VertexShader* g_pVertexShader = nullptr;
ID3D11PixelShader* g_pPixelShader = nullptr;
ID3D11InputLayout* g_pVertexLayout = nullptr; 
ID3D11Buffer* g_pVertexBuffer = nullptr;
ID3D11Buffer* g_pIndexBuffer = nullptr;
ID3D11Buffer* g_pTransformCBuffer = nullptr;
ID3D11Buffer* g_pLightCBuffer = nullptr;

// Forward declarations of functions included in this code module:
HRESULT InitWindow(HINSTANCE hInstance, int nCmdshow);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// GPU programming
HRESULT InitDevice();
void CleanupDevice();
void Render();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, g_szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TUTORIAL, g_szWindowClass, MAX_LOADSTRING);

    // Perform application initialization:
    if (FAILED (InitWindow(hInstance, nCmdShow)))
    {
        return 0;
    }

    if (FAILED(InitDevice()))
    {
        CleanupDevice();
        return 0;
    }

    // �������� ȣ���ϴ� ��Ŀ��� �ΰ����� ���� 
    // Always calling at every frame VS. Only calling when user events occur
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TUTORIAL));
    // Only calling when user events occur
    // Main message loop:
    /*
    MSG msg;

        while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    */

    // Always calling at every frame 
    MSG msg = { 0 };
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            Render();
        }
    }

    CleanupDevice(); // ���� �޼����� ȣ��Ǳ� ���� �ʱ�ȭ
    return (int) msg.wParam;
}

int main()
{
    // Project subsystem���� console�� ����, console�� �����ϱ� ���ؼ� �ش� main function ���
    // wWinMain : windows programming�� ���� function
    return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOWNORMAL);
}


//  FUNCTION: MyRegisterClass() : Registers the window class.
//  FUNCTION: InitInstance(HINSTANCE, int) : Saves instance handle and creates main window
//            In this function, we save the instance handle in a global variable and
//            create and display the main program window.
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    //  PURPOSE: Registers the window class.
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TUTORIAL));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_TUTORIAL);
    wcex.lpszClassName = g_szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    if (!RegisterClassEx(&wcex)) // �ش� window property�� ��Ͽ� �������� ���� ��
        return E_FAIL;

    g_hInst = hInstance; // Store instance handle in our global variable

    // https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindoww
    RECT rc = { 0, 0, 800, 600 };
    // window�� ȭ���� �߰� ��ҿ� ���� ũ�Ⱑ �ٲ�� ���� ����Ͽ� ũ�� ����
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    // void CreateWindowW(lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    // CW_USEDEFAULT�� ������ ����� ��ġ
    // device�� ������ �� handler�� �����ϰ� �Ǵµ� g_hWnd�� ���
    g_hWnd = CreateWindowW(g_szWindowClass, g_szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);

    if (!g_hWnd)
    {
        return E_FAIL;
    }

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    return S_OK;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_MOUSEMOVE:
        // WndProc mouse move
        // https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-mousemove
    {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);
        printf("%d, %d\n", xPos, yPos);
    }
    break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DCompile
//
// With VS 11, we could load up prebuilt .cso files instead...
//--------------------------------------------------------------------------------------
// ��Ÿ�� �߿� szFileName ������ �о �������ϰ� �̸� shader�� ���
HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;

    // Disable optimizations to further improve shader debugging
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // ID3DBlob : IUnknown �������̽��κ��� ��ӵ� ������ temperer �ڷᱸ��
    // �ּ��� GPU�� ����Ǵ��� GPU�� ���������� ������ ������ CPU memory �� ����
    ID3DBlob* pErrorBlob = nullptr;
    // Direct3D SDK function
    // szEntryPoint : shader ���� function
    // szShaderModel : � shader model�� 
    // dwShaderFlags : � ������� ������ ����
    // ppBlobOut : CPU���� ������ �� ���
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
            pErrorBlob->Release();
        }
        return hr;
    }
    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
    HRESULT hr = S_OK;

#pragma region Previous class
    RECT rc;
    // window handler g_hWnd���� rc variable�� �޾ƿ� window�� ũ��� ��ġ ������ �޾ƿ�
    GetClientRect(g_hWnd, &rc); 
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG; // Debug ��忡���� D3D11_CREATE_DEVICE_DEBUG�� ����
    // application�� ����Ǹ鼭 ����� Log message�� console�̳� output�� Ȯ���� �� �ֵ��� ��
#endif

    // �⺻������ GPU�� ����ϱ� ���� API�� ���
    // �׷� API�� �⺻������ OS�� �����ϴ� Driver�� ������� �����ȴ�.
    // ���� Direct3D feature�� �������ִ� GPU�� ����ϸ� OS�� hardware diriver�� ����� �� �ִ�. D3D_DRIVER_TYPE_HARDWARE
    // ������ Direct3D feature�� ���������� �ʴ� GPU�� ����ϸ� OS software driver�� ����Ѵ�. D3D_DRIVER_TYPE_REFERENCE
    D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE; 
    // featureLevels ��������� ���
    // 
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    // HRESULT D3D11CreateDevice(
    // [in, optional]  IDXGIAdapter* pAdapter,
    // D3D_DRIVER_TYPE DriverType,
    // HMODULE         Software,
    // UINT            Flags,
    // [in, optional]  const D3D_FEATURE_LEVEL* pFeatureLevels,
    // UINT            FeatureLevels,
    // UINT            SDKVersion,
    // [out, optional] ID3D11Device** ppDevice, // ���������� �����Ǵ� device�� pointer
    // [out, optional] D3D_FEATURE_LEVEL* pFeatureLevel, // ���õ� featureLevels
    // [out, optional] ID3D11DeviceContext** ppImmediateContext ); // device�� ���������� ����ϱ� ���� Context device
    //  featureLevels�� array�� �Է��Ͽ� �տ������� �ش� ������ �����ϴ��� �����غ�
    // g_pImmediateContext : background�� �ƴ� forground���� GPU ����� �����ϴ� pointer
    hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
        D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);

    // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
    // DXGI ���� �ڵ鸵 �� ���� ���� ����
#pragma region DXGI
    IDXGIFactory1* dxgiFactory = nullptr; // interface�� �θ�
    {
        IDXGIDevice* dxgiDevice = nullptr;
        hr = g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
        if (SUCCEEDED(hr))
        {
            IDXGIAdapter* adapter = nullptr;
            hr = dxgiDevice->GetAdapter(&adapter);
            if (SUCCEEDED(hr))
            {
                hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
                adapter->Release();
            }
            dxgiDevice->Release();
        }
    }
    if (FAILED(hr))
        return hr;
#pragma endregion DXGI

#pragma region Swap Chain
    // Create swap chain
    // DirectX 11.0 systems
    // �� DXGI�� ���� interface�� �ҷ����� swap chain ����
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1; // Backbuffer�� 1���� ��� 
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60; // 60frame���� ȭ���� ����(�ֻ���)
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    // g_pSwapChain���� ������� swap chain�� g_pSwapChain pointer�� ��ȯ
    hr = dxgiFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain); 

    // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
    dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER); // ��üȭ���� �������� ���� DXGI_MWA_NO_ALT_ENTER

    // swap chain�� ���� setting�� �Ϸ�Ǹ� �ݵ�� release!!
    // �׷��� ������ memory rick(����) �߻�
    // �Ҵ�� memory�� DirectX������ refernt count ��ҿ��� ������ ��
    dxgiFactory->Release(); 
#pragma endregion Swap Chain

#pragma region Backbuffer
    // Create a render target view (Backbuffer)
    // �������� Ư�� buffer�� �ϰ� �Ǹ� Backbuffer -> screenbuffer
    // Backbuffer�� �⺻������ texture type resource�� �Ҵ�
    // Texture : sample, Buffer : memory index
    ID3D11Texture2D* pBackBuffer = nullptr; // ID3D11Buffer�� ����
    // swap chain�� ����
    hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
    if (FAILED(hr))
        return hr;
    // swap chain�� ����� backbuffer�� Direct3D API�� �ش� backbuffer�� �׸��� �׸� �� �ְ� ������ ���� 
    // backbuffer�� �׷��Ƚ� ���������ο� ������� �ִ� interface
    // GPU�� ���������� �Ҵ�Ǵ� resource�� �׷��Ƚ� ���������ο� ����� �� ���� ���� ���
    // �츮�� pBackBuffer ���ҽ��� ���� �Ҵ��ϴ� ���� �ƴ� ���ҽ��� binding�� view�� �׷��Ƚ� ���������ο� �Ҵ���
    hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr))
        return hr;

    // �׷��Ƚ� ���������ο� ���� 
    // OMSetRenderTargets : Ouput Merge Render Target
    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);
#pragma endregion Backbuffer
#pragma endregion Previous class

    // Setup the viewport
    // ������ �� ���� ȭ�鿡 �׷����� ���� viewport�� ����
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    // RS : Rasterizer stage
    g_pImmediateContext->RSSetViewports(1, &vp);

#pragma region Create Shader
#pragma region vertex shader
    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    // �����ϵ� ���빰�� �ϳ��� ���ҽ��� ���
    // �����ϵ� �� ���� -> Vertex shader
    // "VS_TEST" " �Է� �޴� shader �̸��� ����
    // "vs_4_0" : Shaders.hlsl"�� vertex shader�� ������, � profile�� compile
    hr = CompileShaderFromFile(L"Shaders.hlsl", "VS_TEST", "vs_4_0", &pVSBlob);
    if (FAILED(hr))
    {
        // L : project�� multi byte�� �̷���� �־ ��� multi byte string���� ��Ÿ�� 
        MessageBox(nullptr, L"Vertex Shader Compiler Error!!", L"Error!!", MB_OK);
        return hr;
    }

    // �����ϵ� vertex shader�� ������ GPU memory�� shader resource�� ����� �̸� vertex shader�� �����ؾ� ��
    // Create the vertex shader
    hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
    if (FAILED(hr))
    {
        pVSBlob->Release();
        return hr;
    }

    // Define the input layout
    // �ش� vertex shader�� ���� ���� � ������ ��������� ����  
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12���� <- �� ä�� 1byte
        { "NORMAL", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // 0, 16
    };
    UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
    // input-assembler�� ���� vertex buffer�� vertex shader�� input���� �� �� ��� ���� ����
    // �������� ä�η� ����� vertex buffer�� �־ �̸� ���ǵ� layout�� ���� input���� ���
    hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &g_pVertexLayout);
    pVSBlob->Release();
    if (FAILED(hr))
        return hr;

    // Set the input layout
    // ������ ������ �� �� ���ƺ���
    g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
#pragma endregion vertex shader

#pragma region pixel shader
    // Compile the pixel shader
    ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"Shaders.hlsl", "PS", "ps_4_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Pixel Shader Compiler Error!!", L"Error", MB_OK);
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
    pPSBlob->Release();
    if (FAILED(hr))
        return hr;
#pragma endregion pixel shader
#pragma endregion Create Shader

#pragma region Create a triangle
    struct SimpleVertex
    {
        Vector3 Pos;
    };

    SimpleVertex vertices[] =
    {
        Vector3(0.0f, 0.5f, 0.5f),
        Vector3(0.5f, -0.5f, 0.5f),
        Vector3(-0.5f, -0.5f, 0.5f), 
    };

    // �Ʒ� vertex�� �����ϱ� ���ؼ� 3D device�� ������ ��
    // � resource�� ����� ���ؼ��� �ش� resource description�� ��������� ��
    //typedef struct D3D11_BUFFER_DESC {
    //    UINT        ByteWidth; // ������ ũ��
    //    D3D11_USAGE Usage; // ���ۿ��� �а� �� ������ ����Ǵ� ����� �ĺ�
    //    UINT        BindFlags; // ���۰� ���������ο� ���ε��Ǵ� ���
    //    UINT        CPUAccessFlags; // CPU �׼����� �ʿ����� ���� ��� 0
    //    UINT        MiscFlags; // ������� �ʴ� ��� 0
    //    UINT        StructureByteStride; // ���۰� ����ȭ�� ���۸� ��Ÿ���� ��� ���� ������ �� ��� ũ��
    //} D3D11_BUFFER_DESC;
    D3D11_BUFFER_DESC bd = {}; // https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_buffer_desc
    bd.Usage = D3D11_USAGE_IMMUTABLE; // ������ GPU�� ���ؼ� ������ �� ����
    // D3D11_USAGE_DEFAULT : GPU�� ���� �������� ������ �� ����(�Ϲ���) 
    // D3D11_USAGE_DYNAMIC : GPU(�б� ����)�� CPU(���� ����) ��ο��� �׼���
    bd.ByteWidth = sizeof(SimpleVertex) * ARRAYSIZE(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    // IMMUTABLE�̸� vertex buffer�� ������ �� initialization buffer�� ���� ��
    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = vertices;
    // HRESULT CreateBuffer(
    //    [in]            const D3D11_BUFFER_DESC* pDesc, 
    //    [in, optional]  const D3D11_SUBRESOURCE_DATA* pInitialData, // ó�� ���ۿ� �� �� ����
    //    [out, optional] ID3D11Buffer** ppBuffer ); // ������ ���� ��ü�� ���� ID3D11Buffer �������̽��� ���� ������ �ּ�
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
    if (FAILED(hr))
        return hr;

    // Set vertex buffer
    // GPU �׷��� ���������ο� ����
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    // Bind an array of vertex buffers to the input-assembler stage.
    // IA : Input Assembler
    // void IASetVertexBuffers(
    //    [in]           UINT         StartSlot,
    //    [in]           UINT         NumBuffers,
    //    [in, optional] ID3D11Buffer* const* ppVertexBuffers,
    //    [in, optional] const UINT* pStrides,
    //    [in, optional] const UINT* pOffsets );
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

    // Set primitive topology
    // Bind information about the primitive type, and data order that describes input data for the input assembler stage.
    // input assembler stage���� �Է� �����͸� �����ϴ� �⺻ ���� �� ������ ������ ���� ���ε� ����
    // void IASetPrimitiveTopology( [in] D3D11_PRIMITIVE_TOPOLOGY Topology );
    // D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST : non �������� �ﰢ���� �迭
    g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

#pragma region Create a triangle
    return hr;
}

//--------------------------------------------------------------------------------------
// Render the frame
//--------------------------------------------------------------------------------------
void Render()
{	
    // Just clear the backbuffer
    // backbuffer�� MidnightBlue color�� �־��
    // DirectX helper
    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, DirectX::Colors::MistyRose);

    // Render a triangle
    g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
    g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
    g_pImmediateContext->Draw(3, 0);

    // Present the information rendered to the back buffer to the front buffer (the screen)
    // �������� �̹����� ����ڿ��� ����
    g_pSwapChain->Present(0, 0);
}

// handler ���� curve ������ �Ǿ� �ִ� : �ش� application�� ����Ǿ memory�� �Ҵ�� �͵��� ���� ����
// ��������� ���α׷��� ����� �� ����� API���� GPU memory�� �Ҵ��� �͵��� �����ؾ��Ѵ�.
//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
    // �̴� �� �����ϴ� ���� �ʿ�!
    if (g_pImmediateContext) g_pImmediateContext->ClearState();

    if (g_pVertexBuffer) g_pVertexBuffer->Release();
    if (g_pVertexLayout) g_pVertexLayout->Release();
    if (g_pVertexShader) g_pVertexShader->Release();
    if (g_pPixelShader) g_pPixelShader->Release();

    if (g_pRenderTargetView) g_pRenderTargetView->Release();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pImmediateContext) g_pImmediateContext->Release();
    if (g_pd3dDevice) g_pd3dDevice->Release();
}