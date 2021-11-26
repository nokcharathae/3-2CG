// CG_Project1.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "CG_Project1.h"

#include "windowsx.h"
#include "stdio.h"

#include "d3d11_1.h"
#include <directxcolors.h>
#include <d3dcompiler.h>

#include <directxmath.h>
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE g_hInst;                                // current instance
WCHAR g_szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR g_szWindowClass[MAX_LOADSTRING];            // the main window class name

// fir D3D setting
HWND g_hWnd;

// D3D Variables
D3D_FEATURE_LEVEL g_featureLevel = D3D_FEATURE_LEVEL_11_0; // NULL도 가능
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pImmediateContext = nullptr; // Immediate Context, Defferred Context
IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr; // GPU에 직접적으로 할당되는 리소스에 바인딩 된 view를 그래픽스 파이프라인에 할당

// 5-1 project
ID3D11VertexShader* g_pVertexShader = nullptr;
ID3D11PixelShader* g_pPixelShader = nullptr;
ID3D11InputLayout* g_pVertexLayout = nullptr;
ID3D11Buffer* g_pVertexBuffer = nullptr; // InitDevice에 생성
ID3D11Buffer* g_pIndexBuffer = nullptr;
ID3D11Buffer* g_pConstantBuffer = nullptr;

ID3D11Texture2D* g_pDepthStencil = nullptr;
ID3D11DepthStencilView* g_pDepthStencilView = nullptr;

ID3D11RasterizerState* g_pRSState; // Backface culling(Rasterizer)
ID3D11DepthStencilState* g_pDSState; // Oculusion culling(Output-Merger)

struct ConstantBuffer
{
    Matrix mWorld;
    Matrix mView;
    Matrix mProjection;
};

// trandform 정의
Matrix g_mWorld, g_mView, g_mProjection;

Vector3 g_pos_eye, g_pos_at, g_vec_up;
// ForwaVector3 g_vec_up rd declarations of functions included in this code module:
HREFTYPE InitWindow(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

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
    LoadStringW(hInstance, IDC_CGPROJECT1, g_szWindowClass, MAX_LOADSTRING);

    // Perform application initialization:
    if (FAILED(InitWindow(hInstance, nCmdShow)))
    {
        return FALSE;
    }

    if (FAILED(InitDevice()))
    {
        CleanupDevice();
        return 0;
    }

    /* // 이벤트가 발생할 떄만 render
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CGPROJECT1));
    MSG msg;
    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    */

    // 매프레임마다 render
    MSG msg = { 0 };
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        else
        {
            Render();
        }
    }
    

    CleanupDevice();
    return (int) msg.wParam;
}


int main()
{
    return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOWNORMAL);
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//

HREFTYPE InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // PURPOSE : Registers the window class
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CGPROJECT1));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CGPROJECT1);
    wcex.lpszClassName = g_szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    g_hInst = hInstance; // Store instance handle in our global variable

    RECT rc = { 0,0,800,600 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    g_hWnd = CreateWindowW(g_szWindowClass, g_szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);

    if (!g_hWnd)
    {
        return FALSE;
    }

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    return S_OK;
}

Vector3 ComputePosSS2WS(int x, int y, const Matrix& mView) // mView = update된 값이 그대로 사용되면 안됨! 
{
    // EXPLAIN!!
    // 어디가 near plane인가?
    RECT rc;
    GetWindowRect(g_hWnd, &rc);
    float w = (float)(rc.right - rc.left);
    float h = (float)(rc.bottom - rc.top);
    Vector3 pos_ps = Vector3((float)x / w * 2 - 1, -(float)y / h * 2 + 1, 0);
    Matrix matPS2CS;
    g_mProjection.Invert(matPS2CS);
    Matrix matCS2WS;
    mView.Invert(matCS2WS);
    Matrix matPS2WS = matPS2CS * matCS2WS; // row major
    Vector3 pos_np_ws = Vector3::Transform(pos_ps, matPS2WS);
    return pos_np_ws;
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
    // viewing state
    static Vector3 pos_start_np_ws;
    static Vector3 pos_start_eye_ws, pos_start_at_ws, vec_start_up;
    static Matrix mView_start;

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
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);

        pos_start_np_ws = ComputePosSS2WS(xPos, yPos, g_mView);
        pos_start_eye_ws = g_pos_eye;
        pos_start_at_ws = g_pos_at;
        vec_start_up = g_vec_up;
        mView_start = g_mView;
    }
    break;
    case WM_MOUSEMOVE:
        // wndProc mouse move
      {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);     

        if (wParam & MK_LBUTTON)
        {
            // To Do
            Vector3 pos_cur_np_ws = ComputePosSS2WS(xPos, yPos, mView_start);
#pragma region HW part 1
#pragma endregion HW part 1
            g_mView = Matrix::CreateLookAt(g_pos_eye, g_pos_at, g_vec_up);
        }
        else if (wParam & MK_RBUTTON)
        {
            // To Do
            Vector3 pos_cur_np_ws = ComputePosSS2WS(xPos, yPos, mView_start);
#pragma region HW part 2
#pragma endregion HW part 2
            g_mView = Matrix::CreateLookAt(g_pos_eye, g_pos_at, g_vec_up);
        }
       }
      break;

    case WM_MOUSEHWHEEL:
    {
        int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
#pragma region HW part 3
#pragma endregion HW part 3
        g_mView = Matrix::CreateLookAt(g_pos_eye, g_pos_at, g_vec_up);
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

    ID3DBlob* pErrorBlob = nullptr;
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

// Create Direct3D device and swap chain
HRESULT InitDevice()
{
    HRESULT hr=S_OK;

#pragma region Previous class
    RECT rc;
    GetClientRect(g_hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlgs = 0;
#ifdef _DEBUG
    createDeviceFlgs |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE; // 지원되는 function에 대해서 hardware
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    // featureLevels의 앞에서부터 해당하는 level을 찾음
    // g_pImmediateContext는 backgraound가 아닌 forground에서 실행 
    hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlgs, featureLevels, numFeatureLevels,
        D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);

    IDXGIFactory1* dxgiFactory = nullptr; // DXGI를 가져오기 위한 device
    {
        IDXGIDevice* dxgiDevice = nullptr;
        // DXGI 가져오기
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

    // Create swap chain
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1; // Backbuffer
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60; // 화면 주사율
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    // DXGI interface에서 swapchain 생성
    hr = dxgiFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain);
    dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER); // 전체화면
    dxgiFactory->Release(); // 반드시 release 설정!

    // Create a render target view(BackBuffer)
    ID3D11Texture2D* pBackBuffer = nullptr;
    // front buffer = screen buffer
    // back buffers는 렌더링 된 결과가 그려짐
    hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
    if (FAILED(hr))
        return hr;

    // view를 생성
    hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr))
        return hr;

    // OM = Output Multi
    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth = {};
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D32_FLOAT;
    descDepth.SampleDesc.Count = 1; // Anti aliacing
    descDepth.SampleDesc.Quality = 0; // Anti aliacing
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = g_pd3dDevice->CreateTexture2D(&descDepth, nullptr, &g_pDepthStencil);
    if (FAILED(hr))
        return hr;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
    if (FAILED(hr))
        return hr;

    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

#pragma endregion

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    // RS = Rasterizer State
    g_pImmediateContext->RSSetViewports(1, &vp);

#pragma region Create Shader
    // Compile thr vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile(L"Shaders.hlsl", "VS_TEST", "vs_4_0", &pVSBlob); // L주의!
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Vertex Shader Compiler Error!!", L"Error!!", MB_OK);
        return hr;
    }

    // Create the vertex shader
    hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
    if (FAILED(hr)) // 실패 시
    {
        pVSBlob->Release();
        return hr;
    }

    // Define the input layout
    // CreateVertexShader에 들어갈 input 
    // vertex shader는 position만 저장
    // Semantics 사용!
    // vertex attributes = position, color, normal, texture coordinate...
    // D3D11_INPUT_ELEMENT_DESC(3) = inputslot은 여러 개일 수 있음 0~16
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0}, // Inputdata
    };
    UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
    hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &g_pVertexLayout);
    pVSBlob->Release();
    if (FAILED(hr)) // 실패 시 
        return hr;
    
    g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

    // Compile the pixel shader
    ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"Shaders.hlsl", "PS", "ps_4_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Pixel Shader Compiler Error!!", L"Eroor", MB_OK);
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
    pPSBlob->Release();
    if (FAILED(hr))
        return hr;

#pragma endregion

// Vertex Buffer 
#pragma region Create a triangle
    struct SimpleVertex
    {
        Vector3 pos;
        Vector4 Color;
    };

    SimpleVertex vertices[] =
    {
        {Vector3(-0.5f,0.5f,-0.5f), Vector4(0.f,0.f,1.f,1.f)},
        {Vector3(0.5f,0.5f,-0.5f),Vector4(0.f,1.f,0.f,1.f)},
        {Vector3(0.5f,0.5f,0.5f),Vector4(0.f,1.f,1.f,1.f)},
        {Vector3(-0.5f,0.5f,0.5f), Vector4(1.f,0.f,0.f,1.f)},
        {Vector3(-0.5f,-0.5f,-0.5f),Vector4(1.f,0.f,1.f,1.f)},
        {Vector3(0.5f,-0.5f,-0.5f),Vector4(1.f,1.f,0.f,1.f)},
        {Vector3(0.5f,-0.5f,0.5f), Vector4(1.f,1.f,1.f,1.f)},
        {Vector3(-0.5f,-0.5f,0.5f),Vector4(0.f,0.f,0.f,1.f)},
    };

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_IMMUTABLE; // Immutable = GPU에 Read만 memory access 성능
    bd.ByteWidth = sizeof(SimpleVertex) * ARRAYSIZE(vertices); // Buffer Size
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = vertices; 
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer); // vertex buffer 만듦
    if (FAILED(hr))
        return hr;

    // graphics pipeline에 매핑
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset); // GPU 파이프라인에 등록

    // Create index buffer
    WORD indices[] = // 16bit index
    {
        3,1,0,
        2,1,3,

        0,5,4,
        1,5,0,

        3,4,7,
        0,4,3,

        1,6,5,
        2,6,1,

        2,7,6,
        3,7,2,

        6,4,5,
        7,4,6,
    };
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer);
    if (FAILED(hr))
        return hr;

    // Set index buffer
    g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    // set primitive topology
    g_pImmediateContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // TriangleList

#pragma endregion

    // Shader에 전달하기 위해서는 constant buffer를 생성해야 함!
    // Create the constatn buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pConstantBuffer);
    if (FAILED(hr))
        return hr;

#pragma region Transform Setting
    g_mWorld = Matrix::CreateScale(10.f);

    g_pos_eye = Vector3(0.0f, 0.0f, 20.0f);
    g_pos_at = Vector3(0.0f, 0.0f, 0.0f);
    g_vec_up = Vector3(0.0f, 1.0f, 0.0f);
    g_mView = Matrix::CreateLookAt(g_pos_eye, g_pos_at, g_vec_up);

    g_mProjection = Matrix::CreatePerspectiveFieldOfView(DirectX::XM_PIDIV2, width / (FLOAT)height, 0.01f, 100.0f);
#pragma endregion

#pragma region States
    D3D11_RASTERIZER_DESC descRaster;
    ZeroMemory(&descRaster, sizeof(D3D11_RASTERIZER_DESC));
    descRaster.FillMode = D3D11_FILL_SOLID; // 채워서 그리기
    descRaster.CullMode = D3D11_CULL_BACK; // Backface culling
    descRaster.FrontCounterClockwise = true;
    descRaster.DepthBias = 0;
    descRaster.DepthBiasClamp = 0;
    descRaster.SlopeScaledDepthBias = 0;
    descRaster.DepthClipEnable = true; // depth = 0~1 외는 clip
    descRaster.ScissorEnable = false; // masking
    descRaster.MultisampleEnable = false; 
    descRaster.AntialiasedLineEnable = false;
    hr = g_pd3dDevice->CreateRasterizerState(&descRaster, &g_pRSState);

    D3D11_DEPTH_STENCIL_DESC descDepthStencil; 
    ZeroMemory(&descDepthStencil, sizeof(D3D11_DEPTH_STENCIL_DESC));
    descDepthStencil.DepthEnable = true;
    descDepthStencil.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; // 전체 픽셀
    descDepthStencil.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; // depth가 현재 pixel 값보다 작을 떄 사용
    descDepthStencil.StencilEnable = false; 
    hr = g_pd3dDevice->CreateDepthStencilState(&descDepthStencil, &g_pDSState);
#pragma endregion 

    return hr;
}

// Clean up the objects We've created
// 메모리 관리에 매우 중요
void CleanupDevice()
{
    if (g_pImmediateContext) g_pImmediateContext->ClearState();
    
    if (g_pConstantBuffer) g_pConstantBuffer->Release();
    if (g_pIndexBuffer)g_pIndexBuffer->Release();
    if (g_pVertexShader) g_pVertexShader->Release();
    if (g_pPixelShader)g_pPixelShader->Release();
    if (g_pVertexLayout)g_pVertexLayout->Release();
    if (g_pVertexBuffer)g_pVertexBuffer->Release();

    if(g_pDepthStencil)g_pDepthStencil->Release();
    if (g_pDepthStencilView)g_pDepthStencilView->Release();
    if (g_pRSState)g_pRSState->Release();
    if (g_pDSState)g_pDSState->Release();

    if (g_pRenderTargetView) g_pRenderTargetView->Release();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pImmediateContext) g_pImmediateContext->Release();
    if (g_pd3dDevice) g_pd3dDevice->Release();
}

void Render()
{
    Matrix matR = Matrix::CreateRotationY(DirectX::XM_PI / 10000.f);
    g_mWorld = matR * g_mWorld;

    // 매프레임 constant buffer 호출
    ConstantBuffer cb;
    cb.mWorld = g_mWorld.Transpose(); // row major -> colum major 를 위한 transpose
    cb.mView = g_mView.Transpose();
    cb.mProjection = g_mProjection.Transpose();
    g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);

    g_pImmediateContext->RSSetState(g_pRSState);
    g_pImmediateContext->OMSetDepthStencilState(g_pDSState, 0);

    // Just clear the backbuffer
    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, DirectX::Colors::MidnightBlue);

    // Clear the depth buffer to 1.0 (max depth)
    g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Render a triangle
    g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
    g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
    g_pImmediateContext->DrawIndexed(36,0, 0);

    // Present the information renderd to the back buffer to the front buffer (the screen)
    g_pSwapChain->Present(0, 0);
}