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
// DirectX::SimpleMath::Vector3 test; 이렇게 선언해야했지만 namespace를 선언해줌으로써
// Vector3 test; 이렇게 간편하게 작성 가능

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE g_hInst;                                // current instance
WCHAR g_szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR g_szWindowClass[MAX_LOADSTRING];            // the main window class name

// for D3D setting
HWND g_hWnd;

// D3D Variables
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device* g_pd3dDevice = nullptr; // NULL을 써도 되지만 pointer라는 것을 명시하기 위해
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

    // 렌더링을 호출하는 방식에는 두가지가 있음 
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

    CleanupDevice(); // 종료 메세지가 호출되기 전에 초기화
    return (int) msg.wParam;
}

int main()
{
    // Project subsystem에서 console을 띄우고, console을 실행하기 위해서 해당 main function 사용
    // wWinMain : windows programming의 시작 function
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

    if (!RegisterClassEx(&wcex)) // 해당 window property가 등록에 성공하지 않을 때
        return E_FAIL;

    g_hInst = hInstance; // Store instance handle in our global variable

    // https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindoww
    RECT rc = { 0, 0, 800, 600 };
    // window가 화면의 추가 요소에 따라 크기가 바뀌는 것을 고려하여 크기 결정
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    // void CreateWindowW(lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    // CW_USEDEFAULT는 이전에 띄워진 위치
    // device를 생성할 때 handler를 생성하게 되는데 g_hWnd를 사용
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
// 런타임 중에 szFileName 파일을 읽어서 컴파일하고 이를 shader에 등록
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

    // ID3DBlob : IUnknown 인터페이스로부터 상속된 임의의 temperer 자료구조
    // 최소한 GPU에 저장되던가 GPU에 직접적으로 접근이 가능한 CPU memory 상에 존재
    ID3DBlob* pErrorBlob = nullptr;
    // Direct3D SDK function
    // szEntryPoint : shader 내의 function
    // szShaderModel : 어떤 shader model로 
    // dwShaderFlags : 어떤 방식으로 컴파일 할지
    // ppBlobOut : CPU에서 컴파일 된 결과
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
    // window handler g_hWnd에서 rc variable을 받아와 window의 크기와 위치 정보를 받아옴
    GetClientRect(g_hWnd, &rc); 
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG; // Debug 모드에서는 D3D11_CREATE_DEVICE_DEBUG로 생성
    // application이 진행되면서 생기는 Log message를 console이나 output에 확인할 수 있도록 함
#endif

    // 기본적으로 GPU를 사용하기 위한 API를 사용
    // 그런 API는 기본적으로 OS가 제공하는 Driver를 기반으로 구현된다.
    // 만약 Direct3D feature를 지원해주는 GPU를 사용하면 OS는 hardware diriver를 사용할 수 있다. D3D_DRIVER_TYPE_HARDWARE
    // 하지만 Direct3D feature를 지원해주지 않는 GPU를 사용하면 OS software driver를 사용한다. D3D_DRIVER_TYPE_REFERENCE
    D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE; 
    // featureLevels 통상적으로 사용
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
    // [out, optional] ID3D11Device** ppDevice, // 최종적으로 생성되는 device의 pointer
    // [out, optional] D3D_FEATURE_LEVEL* pFeatureLevel, // 선택된 featureLevels
    // [out, optional] ID3D11DeviceContext** ppImmediateContext ); // device를 직접적으로 사용하기 위한 Context device
    //  featureLevels은 array로 입력하여 앞에서부터 해당 버전을 지원하는지 실행해봄
    // g_pImmediateContext : background가 아닌 forground에서 GPU 명령을 수행하는 pointer
    hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
        D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);

    // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
    // DXGI 직접 핸들링 할 일은 별로 없다
#pragma region DXGI
    IDXGIFactory1* dxgiFactory = nullptr; // interface를 부름
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
    // 위 DXGI를 통해 interface를 불러오면 swap chain 생성
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1; // Backbuffer로 1개만 사용 
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60; // 60frame으로 화면을 구성(주사율)
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    // g_pSwapChain으로 만들어진 swap chain을 g_pSwapChain pointer로 반환
    hr = dxgiFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain); 

    // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
    dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER); // 전체화면을 지원하지 않음 DXGI_MWA_NO_ALT_ENTER

    // swap chain에 대한 setting이 완료되면 반드시 release!!
    // 그렇지 않으면 memory rick(누수) 발생
    // 할당된 memory는 DirectX에서는 refernt count 요소에서 관리가 됨
    dxgiFactory->Release(); 
#pragma endregion Swap Chain

#pragma region Backbuffer
    // Create a render target view (Backbuffer)
    // 렌더링을 특정 buffer에 하게 되면 Backbuffer -> screenbuffer
    // Backbuffer는 기본적으로 texture type resource로 할당
    // Texture : sample, Buffer : memory index
    ID3D11Texture2D* pBackBuffer = nullptr; // ID3D11Buffer도 가능
    // swap chain에 연결
    hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
    if (FAILED(hr))
        return hr;
    // swap chain에 연결된 backbuffer를 Direct3D API가 해당 backbuffer에 그림을 그릴 수 있게 가지고 오고 
    // backbuffer를 그래픽스 파이프라인에 연결시켜 주는 interface
    // GPU에 직접적으로 할당되는 resource를 그래픽스 파이프라인에 등록할 때 쓰는 구성 요소
    // 우리는 pBackBuffer 리소스를 직접 할당하는 것이 아닌 리소스에 binding된 view를 그래픽스 파이프라인에 할당함
    hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr))
        return hr;

    // 그래픽스 파이프라인에 연결 
    // OMSetRenderTargets : Ouput Merge Render Target
    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);
#pragma endregion Backbuffer
#pragma endregion Previous class

    // Setup the viewport
    // 렌더링 된 것을 화면에 그려지기 위한 viewport를 생성
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
    // 컴파일된 내용물도 하나의 리소스로 취급
    // 컴파일된 값 저장 -> Vertex shader
    // "VS_TEST" " 입력 받는 shader 이름과 동일
    // "vs_4_0" : Shaders.hlsl"를 vertex shader로 컴파일, 어떤 profile로 compile
    hr = CompileShaderFromFile(L"Shaders.hlsl", "VS_TEST", "vs_4_0", &pVSBlob);
    if (FAILED(hr))
    {
        // L : project가 multi byte로 이루어져 있어서 영어를 multi byte string으로 나타냄 
        MessageBox(nullptr, L"Vertex Shader Compiler Error!!", L"Error!!", MB_OK);
        return hr;
    }

    // 컴파일된 vertex shader를 가지고 GPU memory에 shader resource로 만들고 이를 vertex shader에 매핑해야 함
    // Create the vertex shader
    hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
    if (FAILED(hr))
    {
        pVSBlob->Release();
        return hr;
    }

    // Define the input layout
    // 해당 vertex shader에 들어가는 값이 어떤 값인지 명시적으로 정의  
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12가능 <- 각 채널 1byte
        { "NORMAL", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 }, // 0, 16
    };
    UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
    // input-assembler에 넣은 vertex buffer가 vertex shader에 input으로 들어갈 때 어떻게 들어갈지 규정
    // 여러개의 채널로 저장된 vertex buffer가 있어도 미리 정의된 layout의 값만 input으로 사용
    hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &g_pVertexLayout);
    pVSBlob->Release();
    if (FAILED(hr))
        return hr;

    // Set the input layout
    // 별도로 빠지는 게 더 좋아보임
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

    // 아래 vertex를 생성하기 위해선 3D device를 만들어야 함
    // 어떤 resource를 만들기 위해서는 해당 resource description을 지정해줘야 함
    //typedef struct D3D11_BUFFER_DESC {
    //    UINT        ByteWidth; // 버퍼의 크기
    //    D3D11_USAGE Usage; // 버퍼에서 읽고 쓸 것으로 예상되는 방법을 식별
    //    UINT        BindFlags; // 버퍼가 파이프라인에 바인딩되는 방법
    //    UINT        CPUAccessFlags; // CPU 액세스가 필요하지 않은 경우 0
    //    UINT        MiscFlags; // 사용하지 않는 경우 0
    //    UINT        StructureByteStride; // 버퍼가 구조화된 버퍼를 나타내는 경우 버퍼 구조의 각 요소 크기
    //} D3D11_BUFFER_DESC;
    D3D11_BUFFER_DESC bd = {}; // https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_buffer_desc
    bd.Usage = D3D11_USAGE_IMMUTABLE; // 오로지 GPU에 의해서 읽혀질 수 있음
    // D3D11_USAGE_DEFAULT : GPU에 의해 읽혀지고 쓰여질 수 있음(일반적) 
    // D3D11_USAGE_DYNAMIC : GPU(읽기 전용)와 CPU(쓰기 전용) 모두에서 액세스
    bd.ByteWidth = sizeof(SimpleVertex) * ARRAYSIZE(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    // IMMUTABLE이면 vertex buffer가 생성될 때 initialization buffer가 같이 들어감
    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = vertices;
    // HRESULT CreateBuffer(
    //    [in]            const D3D11_BUFFER_DESC* pDesc, 
    //    [in, optional]  const D3D11_SUBRESOURCE_DATA* pInitialData, // 처음 버퍼에 들어갈 때 생성
    //    [out, optional] ID3D11Buffer** ppBuffer ); // 생성된 버퍼 개체에 대한 ID3D11Buffer 인터페이스에 대한 포인터 주소
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
    if (FAILED(hr))
        return hr;

    // Set vertex buffer
    // GPU 그래픽 파이프라인에 매핑
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
    // input assembler stage에서 입력 데이터를 설명하는 기본 유형 및 데이터 순서에 대한 바인딩 정보
    // void IASetPrimitiveTopology( [in] D3D11_PRIMITIVE_TOPOLOGY Topology );
    // D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST : non 연속적인 삼각형의 배열
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
    // backbuffer에 MidnightBlue color를 넣어라
    // DirectX helper
    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, DirectX::Colors::MistyRose);

    // Render a triangle
    g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
    g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
    g_pImmediateContext->Draw(3, 0);

    // Present the information rendered to the back buffer to the front buffer (the screen)
    // 렌더링된 이미지를 사용자에게 제공
    g_pSwapChain->Present(0, 0);
}

// handler 들은 curve 구조로 되어 있다 : 해당 application이 종료되어도 memory가 할당된 것들은 남아 있음
// 명시적으로 프로그램이 종료될 때 사용한 API에서 GPU memory로 할당한 것들을 해제해야한다.
//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
    // 이는 꼭 진행하는 습관 필요!
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