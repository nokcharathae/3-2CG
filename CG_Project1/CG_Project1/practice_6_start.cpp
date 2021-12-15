// practice_1.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "CG_Project1.h"
#include "stl_reader.h"
#include "stb_image.h"

#include <windowsx.h>
#include <stdio.h>

#include <d3d11_1.h>
#include <directxcolors.h>
#include <d3dcompiler.h>

#include <directxmath.h>
#include "SimpleMath.h"
#include <vector>
#include <map>
#include <string>

using namespace DirectX::SimpleMath;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE g_hInst;                                // current instance
WCHAR g_szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR g_szWindowClass[MAX_LOADSTRING];            // the main window class name

// for D3D setting
HWND g_hWnd;

// D3D Variables
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pImmediateContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;

ID3D11VertexShader* g_pVertexShaderPCN = nullptr;
ID3D11VertexShader* g_pVertexShaderPNT = nullptr;
ID3D11VertexShader* g_pVertexShaderP = nullptr;

ID3D11PixelShader* g_pPixelShader1 = nullptr;
ID3D11PixelShader* g_pPixelShader2 = nullptr;
ID3D11PixelShader* g_pPixelShader3 = nullptr;
ID3D11PixelShader* g_pPixelShader4 = nullptr;

ID3D11InputLayout* g_pIALayoutPCN = nullptr;
ID3D11InputLayout* g_pIALayoutPNT = nullptr;
ID3D11InputLayout* g_pIALayoutP = nullptr;

ID3D11ShaderResourceView* g_pSRV_cube = nullptr;
ID3D11ShaderResourceView* g_pSRV_stl = nullptr;

ID3D11Buffer* g_pVertexBuffer_cube = nullptr;
ID3D11Buffer* g_pNormalBuffer_cube = nullptr;
ID3D11Buffer* g_pIndexBuffer_cube = nullptr;
ID3D11Buffer* g_pVertexBuffer_stl = nullptr;
ID3D11Buffer* g_pNormalBuffer_stl = nullptr;
ID3D11Buffer* g_pIndexBuffer_stl = nullptr;
ID3D11Buffer* g_pVertexBuffer_sphere = nullptr;
ID3D11Buffer* g_pIndexBuffer_sphere = nullptr;
ID3D11Buffer* g_pCB_TransformWorld = nullptr;
ID3D11Buffer* g_pCB_Lights = nullptr;

ID3D11SamplerState* g_samplerTex2D = nullptr;
ID3D11Texture2D* g_texEnvMap = nullptr;
ID3D11ShaderResourceView* g_tSRVEnvMap = nullptr;

ID3D11Texture2D* g_pDepthStencil = nullptr;
ID3D11DepthStencilView* g_pDepthStencilView = nullptr;

ID3D11RasterizerState* g_pRSState = nullptr;
ID3D11DepthStencilState* g_pDSState = nullptr;

Vector3 g_pos_light;
Vector3 g_pos_eye, g_pos_at, g_vec_up;

struct MyObject {
public:
	// resources //
	ID3D11Buffer* pVBuffer;
	ID3D11Buffer* pIBuffer;
	ID3D11InputLayout* pIALayer;
	ID3D11VertexShader* pVShader;
	ID3D11PixelShader* pPShader;
	ID3D11RasterizerState* pRSState;
	ID3D11DepthStencilState* pDSState;
	ID3D11ShaderResourceView* pSRViewNormal;

	UINT vb_stride;
	UINT ib_stride;
	int drawCount;
	Matrix mModel;
	Color mtAmbient, mtDiffuse, mtSpec;
	float shininess;

	MyObject() {
		ZeroMemory(this, sizeof(MyObject));
	}

	MyObject(
		const ID3D11Buffer* pVBuffer_, const ID3D11Buffer* pIBuffer_, const ID3D11ShaderResourceView* pSRViewNormal_,
		const ID3D11InputLayout* pIALayer_, const ID3D11VertexShader* pVShader_, const ID3D11PixelShader* pPShader_,
		const ID3D11RasterizerState* pRSState_, const ID3D11DepthStencilState* pDSState_,
		const UINT vb_stride_, const UINT ib_stride_, const int drawCount_, const Matrix &mModel_,
		const Color& mtAmbient_, const Color& mtDiffuse_, const Color& mtSpec_, const float shininess_) {
		pVBuffer = (ID3D11Buffer*)pVBuffer_;
		pIBuffer = (ID3D11Buffer*)pIBuffer_;
		pIALayer = (ID3D11InputLayout*)pIALayer_;
		pVShader = (ID3D11VertexShader*)pVShader_;
		pPShader = (ID3D11PixelShader*)pPShader_;
		pRSState = (ID3D11RasterizerState*)pRSState_;
		pDSState = (ID3D11DepthStencilState*)pDSState_;
		pSRViewNormal = (ID3D11ShaderResourceView*)pSRViewNormal_; // constant로 생성하였기에 casting 해줘야함
		vb_stride = vb_stride_;
		ib_stride = ib_stride_;
		drawCount = drawCount_;
		mModel = mModel_;
		mtAmbient = mtAmbient_;
		mtDiffuse = mtDiffuse_;
		mtSpec = mtSpec_;
		shininess = shininess_;
		pConstBuffer = nullptr;

		// create constant buffer resource
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(Matrix) + 4 * 4; // MUST BE times of 16
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		HRESULT hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &pConstBuffer);
		if (FAILED(hr))
			MessageBoxA(NULL, "Constant Buffer of A MODEL ERROR", "ERROR", MB_OK);
	}

	void UpdateConstanceBuffer() {
		struct ModelConst {
			Matrix mModel;
			DirectX::PackedVector::XMUBYTEN4 mtAmbient, mtDiffuse, mtSpec;
			float shininess;
		};
		ModelConst data = { mModel.Transpose() , mtAmbient.RGBA() , mtDiffuse.RGBA() , mtSpec.RGBA() , shininess };
		g_pImmediateContext->UpdateSubresource(pConstBuffer, 0, nullptr, &data, 0, 0);
	}
	ID3D11Buffer* GetConstantBuffer() {
		return pConstBuffer;
	}
	void Delete() {
		if (pConstBuffer) pConstBuffer->Release();
	}

private:
	// local resource //
	ID3D11Buffer* pConstBuffer; // transform, material colors...
};

std::map<std::string, MyObject> g_sceneObjs; // dictionary와 비슷 <key, value>

// http://www.songho.ca/opengl/gl_sphere.html
// 구 그리는 함수
void GenerateSphere(const float radius, const int sectorCount, const int stackCount,
	std::vector<float>& positions, std::vector<float>& normals, std::vector<float>& texCoords)
{
	// clear memory of prev arrays
	// std::vector : 동적으로 추가할 수 있는 array
	// texcoord는 u, v값을 갖고 2차원 값
	std::vector<float>().swap(positions);
	std::vector<float>().swap(normals);
	std::vector<float>().swap(texCoords);

	float x, y, z, xy;                              // vertex position
	float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
	float s, t;                                     // vertex texCoord

	float sectorStep = 2 * DirectX::XM_PI / (float)sectorCount;
	float stackStep = DirectX::XM_PI / (float)stackCount;
	float sectorAngle, stackAngle;

	for (int i = 0; i <= stackCount; ++i)
	{
		stackAngle = DirectX::XM_PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
		xy = radius * cosf(stackAngle);             // r * cos(u)
		z = radius * sinf(stackAngle);              // r * sin(u)

		// add (sectorCount+1) vertices per stack
		// the first and last vertices have same position and normal, but different tex coords
		for (int j = 0; j <= sectorCount; ++j)
		{
			sectorAngle = j * sectorStep;           // starting from 0 to 2pi

			// vertex position (x, y, z)
			x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
			y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
			positions.push_back(x);
			positions.push_back(y);
			positions.push_back(z);

			// normalized vertex normal (nx, ny, nz)
			nx = x * lengthInv;
			ny = y * lengthInv;
			nz = z * lengthInv;
			normals.push_back(nx);
			normals.push_back(ny);
			normals.push_back(nz);

			// vertex tex coord (s, t) range between [0, 1]
			s = (float)j / sectorCount;
			t = (float)i / stackCount;
			texCoords.push_back(s);
			texCoords.push_back(t);
		}
	}
}

void GenerateIndicesSphere(const int sectorCount, const int stackCount,
	std::vector<UINT>& indices)
{
	// generate CCW index list of sphere triangles
	// k1--k1+1
	// |  / |
	// | /  |
	// k2--k2+1
	//std::vector<int> lineIndices;
	int k1, k2;
	for (int i = 0; i < stackCount; ++i)
	{
		k1 = i * (sectorCount + 1);     // beginning of current stack
		k2 = k1 + sectorCount + 1;      // beginning of next stack

		for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
		{
			// 2 triangles per sector excluding first and last stacks
			// k1 => k2 => k1+1
			if (i != 0)
			{
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			// k1+1 => k2 => k2+1
			if (i != (stackCount - 1))
			{
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}

			// store indices for lines
			// vertical lines for all stacks, k1 => k2
			//lineIndices.push_back(k1);
			//lineIndices.push_back(k2);
			//if (i != 0)  // horizontal lines except 1st stack, k1 => k+1
			//{
			//	lineIndices.push_back(k1);
			//	lineIndices.push_back(k1 + 1);
			//}
		}
	}
}

HRESULT Recompile(bool generateIALayout);

struct CB_TransformScene
{
	Matrix mView;
	Matrix mProjection;
};

struct CB_Object
{
	Matrix mWorld;
	DirectX::PackedVector::XMUBYTEN4 mtAmbient;
	DirectX::PackedVector::XMUBYTEN4 mtDiffuse;
	DirectX::PackedVector::XMUBYTEN4 mtSpec;
	float shininess;
};

struct CB_Lights
{
	Vector3 posLight;
	DirectX::PackedVector::XMUBYTEN4 lightColor; // int 각 4byte 

	Vector3 dirLight; 
	int lightFlag;

	Matrix mView2EnvOS;
};

struct CubeVertex
{
	Vector3 Pos;
	Vector4 Color;
	Vector3 Nor;
};
struct SphereVertex
{
	Vector3 Pos;
	Vector3 Nor;
	Vector2 Tex;
};

Matrix g_mWorld_cube, g_mWorld_sphere, g_mView, g_mProjection;

// Forward declarations of functions included in this code module:
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
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

	if (FAILED(InitWindow(hInstance, nCmdShow)))
		return 0;

	if (FAILED(InitDevice()))
	{
		CleanupDevice();
		return 0;
	}

	//HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PRACTICE1));
	//MSG msg;
	//// Main message loop:
	//while (GetMessage(&msg, nullptr, 0, 0))
	//{
	//    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
	//    {
	//        TranslateMessage(&msg);
	//        DispatchMessage(&msg);
	//    }
	//}
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

	CleanupDevice();
	return (int)msg.wParam;
}

int main()
{
	return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOWNORMAL);
}

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
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDC_CGPROJECT1));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CGPROJECT1);
	wcex.lpszClassName = g_szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL)); 

	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	g_hInst = hInstance; // Store instance handle in our global variable

	// https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindoww
	RECT rc = { 0, 0, 800, 600 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
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

Vector3 ComputePosSS2WS(int x, int y, const Matrix& mView)
{
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
	static Vector3 pos_start_np_ws;
	static Vector3 pos_start_cam_ws, pos_start_at_ws, vec_start_up;
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
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		// case VK_BACK: if (FAILED(Recompile(false))) printf("FAILED!!!!\n"); break;
		}
	}
	break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	{
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		pos_start_np_ws = ComputePosSS2WS(xPos, yPos, g_mView);
		pos_start_cam_ws = g_pos_eye;
		pos_start_at_ws = g_pos_at;
		vec_start_up = g_vec_up;
		mView_start = g_mView;
	}
	break;
	case WM_MOUSEMOVE:
		// WndProc mouse move
		// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-mousemove
	{
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		// https://docs.microsoft.com/en-us/windows/win32/learnwin32/mouse-clicks
		if (wParam & MK_LBUTTON)
		{
			Vector3 pos_cur_np_ws = ComputePosSS2WS(xPos, yPos, mView_start);
#pragma region HW part 1
			//printf("%f, %f, %f\n", pos_start_np_ws.x, pos_start_np_ws.y, pos_start_np_ws.z);
			Vector3 vec_start_cam2np = pos_start_np_ws - pos_start_cam_ws;
			vec_start_cam2np.Normalize();
			Vector3 vec_cur_cam2np = pos_cur_np_ws - pos_start_cam_ws;
			vec_cur_cam2np.Normalize();
			float angle_rad = acosf(vec_start_cam2np.Dot(vec_cur_cam2np)) * 3.0f;
			Vector3 rot_axis = vec_start_cam2np.Cross(vec_cur_cam2np);
			if (rot_axis.LengthSquared() > 0.000001)
			{
				printf("%f\n", angle_rad);
				rot_axis.Normalize();
				Matrix matR = Matrix::CreateFromAxisAngle(rot_axis, angle_rad);

				g_pos_eye = Vector3::Transform(pos_start_cam_ws, matR);
				//g_pos_at = no change
				g_vec_up = Vector3::TransformNormal(vec_start_up, matR);
			}
#pragma endregion HW part 1
			g_mView = Matrix::CreateLookAt(g_pos_eye, g_pos_at, g_vec_up);
		}
		else if (wParam & MK_RBUTTON)
		{
			Vector3 pos_cur_np_ws = ComputePosSS2WS(xPos, yPos, mView_start);
#pragma region HW part 2
			float dist_at = (pos_start_at_ws - pos_start_cam_ws).Length();
			// np : 0.01f
			Vector3 vec_diff_np = pos_cur_np_ws - pos_start_np_ws;
			float dist_diff_np = vec_diff_np.Length();
			float dist_diff = dist_diff_np / 0.01f * dist_at;
			if (dist_diff_np > 0.000001)
			{
				Vector3 vec_diff = vec_diff_np / dist_diff_np * dist_diff;
				g_pos_eye = pos_start_cam_ws - vec_diff;
				g_pos_at = pos_start_at_ws - vec_diff;
			}
#pragma endregion HW part 2
			g_mView = Matrix::CreateLookAt(g_pos_eye, g_pos_at, g_vec_up);
		}
	}
	break;
	case WM_MOUSEWHEEL:
	{
		int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
#pragma region HW part 3
		float move_delta = zDelta > 0 ? 0.5f : -0.5f;
		Vector3 view_dir = (g_pos_at - g_pos_eye);
		view_dir.Normalize();
		g_pos_eye += move_delta * view_dir;
		g_pos_at += move_delta * view_dir;
#pragma endregion HW part 3
		g_mView = Matrix::CreateLookAt(g_pos_eye, g_pos_at, g_vec_up);
	}
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

HRESULT Recompile(bool generateIALayout)
{
	HRESULT hr = S_OK;
	// Compile the vertex shader
	ID3DBlob *pVSBlobPCN = nullptr, *pVSBlobPNT = nullptr, * pVSBlobP = nullptr;
	
	auto CreateShader = [](const std::string& shaderName, const std::string& shaderProfile, ID3DBlob **ppShaderBlob,
		ID3D11DeviceChild **ppShader)
	{
		if (shaderProfile != "vs_4_0" && shaderProfile != "ps_4_0") return E_FAIL;

		HRESULT hr = CompileShaderFromFile(L"Shaders_6_start.hlsl", shaderName.c_str(), shaderProfile.c_str(), ppShaderBlob);
		if (FAILED(hr))
		{
			MessageBoxA(nullptr, (std::string("Shader Compiler Error!! ") + shaderName + " " + shaderProfile).c_str(), "Error!!", MB_OK);
			return hr;
		}

		if (shaderProfile == "vs_4_0") {
			ID3D11VertexShader **ppVS = (ID3D11VertexShader**)ppShader;
			if (*ppVS) (*ppVS)->Release();
			// Create the vertex shader
			hr = g_pd3dDevice->CreateVertexShader((*ppShaderBlob)->GetBufferPointer(), (*ppShaderBlob)->GetBufferSize(), nullptr, ppVS);
		}
		else if (shaderProfile == "ps_4_0") {
			ID3D11PixelShader** ppPS = (ID3D11PixelShader**)ppShader;
			if (*ppPS) (*ppPS)->Release();
			// Create the vertex shader
			hr = g_pd3dDevice->CreatePixelShader((*ppShaderBlob)->GetBufferPointer(), (*ppShaderBlob)->GetBufferSize(), nullptr, ppPS);
		}

		return hr;
	};

	hr |= CreateShader("VS_PCN", "vs_4_0", &pVSBlobPCN, (ID3D11DeviceChild**)&g_pVertexShaderPCN);
	hr |= CreateShader("VS_PNT", "vs_4_0", &pVSBlobPNT, (ID3D11DeviceChild**)&g_pVertexShaderPNT);
	hr |= CreateShader("VS_P", "vs_4_0", &pVSBlobP, (ID3D11DeviceChild**)&g_pVertexShaderP);

	if (generateIALayout && SUCCEEDED(hr)) {
		// Define the input layout
		D3D11_INPUT_ELEMENT_DESC layout_PCN[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE(layout_PCN);
		// Create the input layout
		hr |= g_pd3dDevice->CreateInputLayout(layout_PCN, numElements, pVSBlobPCN->GetBufferPointer(), pVSBlobPCN->GetBufferSize(), &g_pIALayoutPCN);

		// Define the input layout
		D3D11_INPUT_ELEMENT_DESC layout_PNT[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		numElements = ARRAYSIZE(layout_PNT);
		// Create the input layout
		hr |= g_pd3dDevice->CreateInputLayout(layout_PNT, numElements, pVSBlobPNT->GetBufferPointer(), pVSBlobPNT->GetBufferSize(), &g_pIALayoutPNT);
	}

	// 왜 if문에 안 넣을까요?
	D3D11_INPUT_ELEMENT_DESC layout_P[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout_P);
	// Create the input layout
	hr |= g_pd3dDevice->CreateInputLayout(layout_P, numElements, pVSBlobP->GetBufferPointer(), pVSBlobP->GetBufferSize(), &g_pIALayoutP);

	if (pVSBlobPCN) pVSBlobPCN->Release();
	if (pVSBlobPNT) pVSBlobPNT->Release();
	if (pVSBlobP) pVSBlobP->Release();
	
	ID3DBlob* pPSBlob1 = nullptr, * pPSBlob2 = nullptr, * pPSBlob3 = nullptr, * pPSBlob4 = nullptr;
	hr |= CreateShader("PS1", "ps_4_0", &pPSBlob1, (ID3D11DeviceChild**)&g_pPixelShader1);
	hr |= CreateShader("PS2", "ps_4_0", &pPSBlob2, (ID3D11DeviceChild**)&g_pPixelShader2);
	hr |= CreateShader("PS3", "ps_4_0", &pPSBlob3, (ID3D11DeviceChild**)&g_pPixelShader3);
	hr |= CreateShader("PS4", "ps_4_0", &pPSBlob4, (ID3D11DeviceChild**)&g_pPixelShader4);
	
	if (pPSBlob1) pPSBlob1->Release();
	if (pPSBlob2) pPSBlob2->Release();
	if (pPSBlob3) pPSBlob3->Release();
	if (pPSBlob4) pPSBlob4->Release();

	return hr;
}

#include <iostream>
//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
	HRESULT hr = S_OK;

#pragma region Previous class
	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
		D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory1* dxgiFactory = nullptr;
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

	// Create swap chain
	// DirectX 11.0 systems
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
#ifdef MSAA // Multi Sampling Anti-Aliasing. SSAA가 지나치게 무식한 성능과 메모리를 요구해서 만들어진 개량법
	sd.SampleDesc.Count = 2;
#else
	sd.SampleDesc.Count = 1;
#endif
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	hr = dxgiFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain);

	// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
	dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);

	dxgiFactory->Release();

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
		return hr;

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
#ifdef MSAA
	descDepth.SampleDesc.Count = 2;
#else
	descDepth.SampleDesc.Count = 1;
#endif
	descDepth.SampleDesc.Quality = 0;
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
#ifdef MSAA
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
#else
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
#endif
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
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);

#pragma region Create Shader
	Recompile(true);
#pragma endregion Create Shader

#pragma region Create a cube
	int indices_cube = 0, indices_stl = 0, indices_sphere = 0;
	{
		CubeVertex vertices[] =
		{
			{ Vector3(-0.5f, 0.5f, -0.5f), Vector4(0.0f, 0.0f, 1.0f, 1.0f), Vector3() },
			{ Vector3(0.5f, 0.5f, -0.5f), Vector4(0.0f, 1.0f, 0.0f, 1.0f), Vector3() },
			{ Vector3(0.5f, 0.5f, 0.5f), Vector4(0.0f, 1.0f, 1.0f, 1.0f), Vector3() },
			{ Vector3(-0.5f, 0.5f, 0.5f), Vector4(1.0f, 0.0f, 0.0f, 1.0f), Vector3() },
			{ Vector3(-0.5f, -0.5f, -0.5f), Vector4(1.0f, 0.0f, 1.0f, 1.0f), Vector3() },
			{ Vector3(0.5f, -0.5f, -0.5f), Vector4(1.0f, 1.0f, 0.0f, 1.0f), Vector3() },
			{ Vector3(0.5f, -0.5f, 0.5f), Vector4(1.0f, 1.0f, 1.0f, 1.0f), Vector3() },
			{ Vector3(-0.5f, -0.5f, 0.5f), Vector4(0.0f, 0.0f, 0.0f, 1.0f), Vector3() },
		};
		for (int i = 0; i < ARRAYSIZE(vertices); i++)
		{
			CubeVertex& vtx = vertices[i];
			vtx.Nor = vtx.Pos;
			vtx.Nor.Normalize();
		}

		D3D11_BUFFER_DESC bd = {}; // https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_buffer_desc
		bd.Usage = D3D11_USAGE_IMMUTABLE; // https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_usage 
		bd.ByteWidth = sizeof(CubeVertex) * ARRAYSIZE(vertices);
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData = {};
		InitData.pSysMem = vertices;
		hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer_cube);
		if (FAILED(hr))
			return hr;

		// Create index buffer
		// 위쪽 면, 뒤쪽 면, 왼쪽 면, 오른쪽 면, 앞쪽 면, 아랫 면 순으로 그려짐
		WORD indices[] =
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
		indices_cube = ARRAYSIZE(indices);

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(WORD) * indices_cube;        // 36 vertices needed for 12 triangles in a triangle list
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = indices;
		hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer_cube);
		if (FAILED(hr))
			return hr;

		// +y, -z, -x, +x, +z, -y 순으로 그려짐
		// 면단위로 normal vector 생성
		Vector3 cubeFaceNormals[] = {
		  Vector3(0.f,1.f,0.f),Vector3(0.f,1.f,0.f), Vector3(0.f,0.f,-1.f),Vector3(0.f,0.f,-1.f), Vector3(-1.f,0.f,0.f),Vector3(-1.f,0.f,0.f),
		  Vector3(1.f,0.f,0.f),Vector3(1.f,0.f,0.f), Vector3(0.f,0.f,1.f),Vector3(0.f,0.f,1.f), Vector3(0.f,-1.f,0.f),Vector3(0.f,-1.f,0.f),
		};

		bd.ByteWidth = sizeof(Vector3) * 12;
		bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		InitData.pSysMem = cubeFaceNormals;
		hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pNormalBuffer_cube);
		if (FAILED(hr))
			return hr;

		// Shader resource는 해당 shader에 할당하기 위해서 view라는 interface를 사용
		// 이것이 syntex
		D3D11_SHADER_RESOURCE_VIEW_DESC dcSrv = {};
		dcSrv.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
		// view가 가리키는 resource 지목
		dcSrv.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX; //UNION 방식 여러개를 다를수 있음
		dcSrv.BufferEx.FirstElement = 0;
		dcSrv.BufferEx.NumElements = 12;
		// view도 array로 만들 수 있음
		hr = g_pd3dDevice->CreateShaderResourceView(g_pNormalBuffer_cube, &dcSrv, &g_pSRV_cube);
		if (FAILED(hr))
			return hr;
	}
#pragma endregion

#pragma region Load a STL
	{
		stl_reader::StlMesh <float, unsigned int> mesh("Armadillo2.stl");
		const float* raw_coords = mesh.raw_coords();
		/*
		* bounding box
		// const Vector3* raw3_coords = (const Vector3*)raw_coords; // 일반적
		// Vector3 minPos(FLT_MAX), maxPos(-FLT_MAX);
		// for (int i = 0; i < mesh.num_vrts(); i++) {
		// 	minPos.x = min(minPos.x, raw3_coords[i].x);
		// 	minPos.y = min(minPos.y, raw3_coords[i].y);
		// 	minPos.z = min(minPos.z, raw3_coords[i].z); 
		// 
		// 	maxPos.x = min(maxPos.x, raw3_coords[i].x);
		// 	maxPos.y = min(maxPos.y, raw3_coords[i].y);
		// 	maxPos.z = min(maxPos.z, raw3_coords[i].z);
		// }
		*/ 
  		const float* raw_normals = mesh.raw_normals();
		const unsigned int* raw_tris = mesh.raw_tris();
		
		// STL용 vertex shader
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(Vector3) * mesh.num_vrts();
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData = {};
        InitData.pSysMem = raw_coords;
        hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer_stl);
        if (FAILED(hr))
            return hr;

		indices_stl = mesh.num_tris() * 3;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(UINT) * indices_stl;       // 36 vertices needed for 12 triangles in a triangle list
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = raw_tris;
		hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer_stl);
		if (FAILED(hr))
			return hr;

		bd.ByteWidth = sizeof(Vector3) * mesh.num_tris(); // Vector3 주의
		bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		InitData.pSysMem = raw_normals;
		hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pNormalBuffer_stl);
		if (FAILED(hr))
			return hr;

		// Shader resource는 해당 shader에 할당하기 위해서 view라는 interface를 사용
		D3D11_SHADER_RESOURCE_VIEW_DESC dcSrv = {};
		dcSrv.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
		// view가 가리키는 resource 지목
		dcSrv.ViewDimension = D3D11_SRV_DIMENSION_BUFFER; // 왜 이걸로 했을까?
		dcSrv.BufferEx.FirstElement = 0;
		dcSrv.BufferEx.NumElements = mesh.num_tris();
		// view도 array로 만들 수 있음
		hr = g_pd3dDevice->CreateShaderResourceView(g_pNormalBuffer_stl, &dcSrv, &g_pSRV_stl);
		if (FAILED(hr))
			return hr;
	}
#pragma endregion 

#pragma region Create a sphere
	{
		std::vector<float> position, normal, texcoord;
		const int stackCount = 100, sectorCount = 100;
		// vertex를 생성해주는 코드
		// 지구본의 map이 사각형이고, 가로 세로가 각각 0~1로 매핑될 때 구의 각 vertex point에서 해당 texture에 매핑되면 이를 texture coordinate에 저장
		GenerateSphere(0.5f, sectorCount, stackCount, position, normal, texcoord);
		std::cout << "# of position: " << position.size() / 3 << ", # of normal: " << normal.size() / 3 << ", # of texcoord: "
			<< texcoord.size() / 2 << std::endl;

		int numVertices = (int)position.size() / 3;

		D3D11_BUFFER_DESC bd_sphere = {};
		bd_sphere.Usage = D3D11_USAGE_IMMUTABLE;
		bd_sphere.ByteWidth = sizeof(SphereVertex) * numVertices;
		bd_sphere.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd_sphere.CPUAccessFlags = 0;

		std::vector<SphereVertex> verticesSphere(numVertices);
		for (int i = 0; i < numVertices; i++)
		{
			SphereVertex& vtx = verticesSphere[i];

#define VECTOR3std(VEC_NAME, I) Vector3(VEC_NAME[3 * I + 0], VEC_NAME[3 * I + 1], VEC_NAME[3 * I + 2])

			vtx.Pos = VECTOR3std(position, i);
			vtx.Nor = VECTOR3std(normal, i);
			vtx.Tex = Vector2(texcoord[2 * i + 0], texcoord[2 * i + 1]);
		}

		D3D11_SUBRESOURCE_DATA InitData = {};
		InitData.pSysMem = &verticesSphere[0];
		hr = g_pd3dDevice->CreateBuffer(&bd_sphere, &InitData, &g_pVertexBuffer_sphere);
		if (FAILED(hr))
			return hr;

		std::vector<UINT> sphereIndices;
		GenerateIndicesSphere(sectorCount, stackCount, sphereIndices);

		indices_sphere = sphereIndices.size();
		
		// 삼각형을 그릴 때는 indexbuffer를 사용
		bd_sphere.Usage = D3D11_USAGE_DEFAULT;
		bd_sphere.ByteWidth = sizeof(int) * (UINT)indices_sphere;        // 36 vertices needed for 12 triangles in a triangle list
		bd_sphere.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd_sphere.CPUAccessFlags = 0;
		InitData.pSysMem = &sphereIndices[0];
		hr = g_pd3dDevice->CreateBuffer(&bd_sphere, &InitData, &g_pIndexBuffer_sphere);
		if (FAILED(hr))
			return hr;
	}
#pragma endregion Create a sphere

	// Create the constant buffer
	D3D11_BUFFER_DESC bdCB = {};
	bdCB.Usage = D3D11_USAGE_DEFAULT;
	// CB_TransformScene : scene에 공통으로 사용하는 transform matrix를 저장하는 constant buffer
	bdCB.ByteWidth = sizeof(CB_TransformScene);
	bdCB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bdCB.CPUAccessFlags = 0;
	hr = g_pd3dDevice->CreateBuffer(&bdCB, nullptr, &g_pCB_TransformWorld);
	if (FAILED(hr))
		return hr;

	bdCB.Usage = D3D11_USAGE_DEFAULT;
	bdCB.ByteWidth = sizeof(CB_Lights);
	bdCB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bdCB.CPUAccessFlags = 0;
	hr = g_pd3dDevice->CreateBuffer(&bdCB, nullptr, &g_pCB_Lights);
	if (FAILED(hr))
		return hr;

#pragma region Env Mapping
	// image 읽어오기
	int w, h, n;
	unsigned char* img = stbi_load("envi.jpg", &w, &h, &n, 0);
	unsigned char* img_rgba = new unsigned char[w * h * 4];
	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++) {
			img_rgba[4 * (w * y + x) + 0] = img[3 * (w * y + x) + 0];
			img_rgba[4 * (w * y + x) + 1] = img[3 * (w * y + x) + 1];
			img_rgba[4 * (w * y + x) + 2] = img[3 * (w * y + x) + 2];
			img_rgba[4 * (w * y + x) + 3] = (unsigned char)255;
		} 

	D3D11_TEXTURE2D_DESC dcTex2D = {};
	// ZeroMemory(&dcTex2D, sizeof(D3D11_TEXTURE2D_DESC)); // C-style
	// memset(&dcTex2D, sizeof(D3D11_TEXTURE2D_DESC), 0); // C-style
	dcTex2D.Width = w;
	dcTex2D.Height = h;
	dcTex2D.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	dcTex2D.ArraySize = 1;
	dcTex2D.MipLevels = 1;
	dcTex2D.SampleDesc.Count=1; // super sampling을 하니? 하지만 우리는 쓰지 않음, 매 sample마다 1의 값을 저장 그대로 두면 될듯?
	dcTex2D.SampleDesc.Quality=0;
	dcTex2D.Usage=D3D11_USAGE_IMMUTABLE; // Dynamic은 constant에서 많이 씀
	// constant buffer는 매 시간마다 계속 frame됨.
	// Usage 시험에 나올 듯
	dcTex2D.BindFlags=D3D11_BIND_SHADER_RESOURCE;
	dcTex2D.CPUAccessFlags=0; // IMMUTABLE이기 때문에 CPU Access가 안됨
	dcTex2D.MiscFlags=0; // Mipmap을 허용할 지?

	D3D11_SUBRESOURCE_DATA InitTex2DData;
	InitTex2DData.pSysMem = img_rgba;
	InitTex2DData.SysMemPitch = w * 4; // y의 stride를 정의
	InitTex2DData.SysMemSlicePitch = 0; // 3차원일 때
	hr = g_pd3dDevice->CreateTexture2D(&dcTex2D, &InitTex2DData, &g_texEnvMap);
	delete[] img_rgba;
	if (FAILED(hr)) {
		return hr;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC dcTexSRV = {};
	dcTexSRV.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	dcTexSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D; 
	dcTexSRV.Texture2D.MipLevels = 1;
	dcTexSRV.Texture2D.MostDetailedMip = 0;
	hr = g_pd3dDevice->CreateShaderResourceView(g_texEnvMap, &dcTexSRV, &g_tSRVEnvMap);
	if (FAILED(hr)) {
		return hr;
	}
	 
#pragma endregion Env Mapping

	// view를 scene단위에 넣을지, object 단위에 넣을지
#pragma region Transform Setting
	g_pos_light = Vector3(0, 8, 0);

	g_mWorld_cube = Matrix::CreateScale(10.f);
	g_mWorld_sphere = Matrix::CreateScale(2.f) * Matrix::CreateTranslation(g_pos_light); // 구의 지름 2.f, point light position에 둠
	// row major기 때문에 계산 순서는 ->

	g_pos_eye = Vector3(0.0f, 0.0f, 20.0f);
	g_pos_at = Vector3(0.0f, 0.0f, 0.0f);
	g_vec_up = Vector3(0.0f, 1.0f, 0.0f);
	g_mView = Matrix::CreateLookAt(g_pos_eye, g_pos_at, g_vec_up);

	g_mProjection = Matrix::CreatePerspectiveFieldOfView(DirectX::XM_PIDIV2, width / (FLOAT)height, 0.1f, 1000.0f);
#pragma endregion

#pragma region States
	D3D11_RASTERIZER_DESC descRaster;
	ZeroMemory(&descRaster, sizeof(D3D11_RASTERIZER_DESC));
	descRaster.FillMode = D3D11_FILL_SOLID;
	descRaster.CullMode = D3D11_CULL_NONE; // 주의
	descRaster.FrontCounterClockwise = true;
	descRaster.DepthBias = 0;
	descRaster.DepthBiasClamp = 0;
	descRaster.SlopeScaledDepthBias = 0;
	descRaster.DepthClipEnable = true;
	descRaster.ScissorEnable = false;
#ifdef MSAA
	descRaster.MultisampleEnable = true;
	descRaster.AntialiasedLineEnable = true;
#else
	descRaster.MultisampleEnable = false;
	descRaster.AntialiasedLineEnable = false;
#endif
	hr = g_pd3dDevice->CreateRasterizerState(&descRaster, &g_pRSState);

	D3D11_DEPTH_STENCIL_DESC descDepthStencil;
	ZeroMemory(&descDepthStencil, sizeof(D3D11_DEPTH_STENCIL_DESC));
	descDepthStencil.DepthEnable = true;
	descDepthStencil.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	descDepthStencil.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	descDepthStencil.StencilEnable = false;
	hr = g_pd3dDevice->CreateDepthStencilState(&descDepthStencil, &g_pDSState);
#pragma endregion

	D3D11_SAMPLER_DESC dcSample = {};
	dcSample.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	dcSample.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
	dcSample.AddressV= D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
	dcSample.AddressW= D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
	*(Vector4*)&dcSample.BorderColor = Vector4();
	dcSample.MipLODBias = 0;
	dcSample.MaxAnisotropy = 16;
	dcSample.MinLOD = 0;
	dcSample.MaxLOD = D3D11_FLOAT32_MAX;
	dcSample.ComparisonFunc = D3D11_COMPARISON_NEVER;
	hr = g_pd3dDevice->CreateSamplerState(&dcSample, &g_samplerTex2D);
	if (FAILED(hr))
		return hr;

	// object
	g_sceneObjs["STL"] = MyObject(g_pVertexBuffer_stl, g_pIndexBuffer_stl, g_pSRV_stl, g_pIALayoutP, g_pVertexShaderP, g_pPixelShader4,
		g_pRSState, g_pDSState, sizeof(Vector3), sizeof(UINT), indices_stl,
		Matrix::CreateScale(1.f / 12.f) * Matrix::CreateTranslation(-10.f, 0.f, 0.f), Color(0.1f, 0.1f, 0.1f), Color(0.7f, 0.7f, 0.7f), Color(0.2f, 0.2f, 0.2f), 10.f);

	g_sceneObjs["CUBE"] = MyObject(g_pVertexBuffer_cube, g_pIndexBuffer_cube, g_pSRV_cube, g_pIALayoutPCN, g_pVertexShaderPCN, g_pPixelShader4,
		g_pRSState, g_pDSState, sizeof(CubeVertex), sizeof(WORD), indices_cube,
		g_mWorld_cube, Color(0.1f, 0.1f, 0.1f), Color(0.7f, 0.7f, 0), Color(0.2f, 0, 0.2f), 10.f);

	g_sceneObjs["CUBE2"] = MyObject(g_pVertexBuffer_cube, g_pIndexBuffer_cube, g_pSRV_cube, g_pIALayoutPCN, g_pVertexShaderPCN, g_pPixelShader1,
	   g_pRSState, g_pDSState, sizeof(CubeVertex), sizeof(WORD), indices_cube,
		Matrix::CreateScale(5.f) * Matrix::CreateTranslation(10.f, 0.f, 0.f), Color(0.1f, 0.1f, 0.1f), Color(0.f, 0.7f, 0.7f), Color(0.2f, 0, 0.2f), 10.f);

	g_sceneObjs["SPHERE"] = MyObject(g_pVertexBuffer_sphere, g_pIndexBuffer_sphere, NULL, g_pIALayoutPNT, g_pVertexShaderPNT, g_pPixelShader2,
		g_pRSState, g_pDSState, sizeof(SphereVertex), sizeof(UINT), indices_sphere,
		g_mWorld_sphere, Color(1.f, 1.f, 1.f), Color(), Color(), 1.f);

	Matrix matR = Matrix::CreateLookAt(Vector3(0, 0, 0), Vector3(0, -1, 0), Vector3(0, 0, 1));
	
	g_sceneObjs["SPHERE_ENV"] = MyObject(g_pVertexBuffer_sphere, g_pIndexBuffer_sphere, NULL, g_pIALayoutPNT, g_pVertexShaderPNT, g_pPixelShader3,
		g_pRSState, g_pDSState, sizeof(SphereVertex), sizeof(UINT), indices_sphere,
		matR * Matrix::CreateScale(500.f), Color(1.f, 1.f, 1.f), Color(), Color(), 1.f);

	return hr;
}

//--------------------------------------------------------------------------------------
// Render the frame
//--------------------------------------------------------------------------------------
void Render()
{
	g_pImmediateContext->PSSetSamplers(0, 1, &g_samplerTex2D);
#pragma region Common Scene (World)
	// scene에 공통으로 적용될 constant buffer
	CB_TransformScene cbTransformScene;
	cbTransformScene.mView = g_mView.Transpose();
	cbTransformScene.mProjection = g_mProjection.Transpose();
	g_pImmediateContext->UpdateSubresource(g_pCB_TransformWorld, 0, nullptr, &cbTransformScene, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCB_TransformWorld); // projection taransform은 vertexshader에서만 사용
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pCB_TransformWorld);

	CB_Lights cbLight;
	cbLight.posLight = Vector3::Transform(g_pos_light, g_mView);
	cbLight.dirLight = Vector3::TransformNormal(Vector3(0, 1, 0), g_mView);
	cbLight.dirLight.Normalize(); 
	cbLight.lightColor = Color(1, 1, 1, 1).RGBA();
	cbLight.lightFlag = 0;

	// 시험 출제
	Matrix mView2World = g_mView.Invert();
	MyObject& envSphere = g_sceneObjs["SPHERE_ENV"];
	Matrix mView2EnvSphere = envSphere.mModel.Invert();
	cbLight.mView2EnvOS = (mView2World * mView2EnvSphere).Transpose();

	g_pImmediateContext->UpdateSubresource(g_pCB_Lights, 0, nullptr, &cbLight, 0, 0);
	// g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pCB_Lights); // setting할 필요 없음
	g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCB_Lights);

	g_pImmediateContext->PSSetShaderResources(1,1,&g_tSRVEnvMap);

	// Just clear the backbuffer
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, DirectX::Colors::MistyRose);

	// Clear the depth buffer to 1.0 (max depth) 
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
#pragma endregion Common Scene (World)

	// auto = std::pair<std::string, MyObject>
	for (auto& pair : g_sceneObjs)
	{
		//if (pair.first == "CUBE") continue;
		MyObject& obj = pair.second; // &를 통해 copy가 아닌 참조를 할 수 있음 
#pragma region For Each Object
		// Set vertex buffer
		UINT stride = obj.vb_stride;
		UINT offset = 0;
		g_pImmediateContext->IASetInputLayout(obj.pIALayer);
		g_pImmediateContext->IASetVertexBuffers(0, 1, &obj.pVBuffer, &stride, &offset); // g_pVertexBuffer_sphere
		// Set index buffer
		switch (obj.ib_stride)
		{
		case 2:
			g_pImmediateContext->IASetIndexBuffer(obj.pIBuffer, DXGI_FORMAT_R16_UINT, 0); break;
		case 4:
			g_pImmediateContext->IASetIndexBuffer(obj.pIBuffer, DXGI_FORMAT_R32_UINT, 0); break;
		default:
			break;
		}
		// Set primitive topology
		g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		// 경우에 따라 다른 로직에서 world matrix나 material property가 바뀔 수 있기 때문에 바뀐 값이 바로 적용될 수 있도록 정의
		obj.UpdateConstanceBuffer();
		ID3D11Buffer* pCBuffer = obj.GetConstantBuffer();
		g_pImmediateContext->VSSetConstantBuffers(1, 1, &pCBuffer);
		g_pImmediateContext->PSSetConstantBuffers(1, 1, &pCBuffer);

		// Buffer<float3> NormalBuffer : register(t0) 위한 setting
		if (obj.pSRViewNormal) {
			g_pImmediateContext->PSSetShaderResources(0, 1, &obj.pSRViewNormal);
		}
		// 이를 통해 해당 버퍼 인덱싱 가능

		g_pImmediateContext->RSSetState(obj.pRSState);
		g_pImmediateContext->OMSetDepthStencilState(obj.pDSState, 0);

		// Render a triangle
		g_pImmediateContext->VSSetShader(obj.pVShader, nullptr, 0);
		g_pImmediateContext->PSSetShader(obj.pPShader, nullptr, 0);
		
		if (obj.pIBuffer)
			g_pImmediateContext->DrawIndexed(obj.drawCount, 0, 0);
		else 
			g_pImmediateContext->Draw(obj.drawCount, 0);
	}
#pragma endregion For Each Object

	// Present the information rendered to the back buffer to the front buffer (the screen)
	g_pSwapChain->Present(0, 0);
}

//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
	// map에서 constant buffer를 생성하기에 그냥 종료를 하면 자동으로 delete()를 호출하지 않음
	for (auto& obj : g_sceneObjs) obj.second.Delete();
	g_sceneObjs.clear(); 

	if (g_pImmediateContext) g_pImmediateContext->ClearState();

	if (g_pCB_TransformWorld) g_pCB_TransformWorld->Release();
	if (g_pCB_Lights) g_pCB_Lights->Release();

	if (g_pSRV_cube)g_pSRV_cube->Release();
	if (g_pVertexBuffer_cube) g_pVertexBuffer_cube->Release();
	if (g_pNormalBuffer_cube) g_pNormalBuffer_cube->Release();
	if (g_pIndexBuffer_cube) g_pIndexBuffer_cube->Release();

	if (g_pSRV_stl)g_pSRV_stl->Release();
	if (g_pVertexBuffer_stl) g_pVertexBuffer_stl->Release();
	if (g_pNormalBuffer_stl) g_pNormalBuffer_stl->Release();
	if (g_pIndexBuffer_stl) g_pIndexBuffer_stl->Release();

	if (g_pIndexBuffer_sphere) g_pIndexBuffer_sphere->Release();
	if (g_pVertexBuffer_sphere) g_pVertexBuffer_sphere->Release();

	if (g_pIALayoutPCN) g_pIALayoutPCN->Release();
	if (g_pIALayoutPNT) g_pIALayoutPNT->Release();
	if (g_pIALayoutP) g_pIALayoutP->Release();

	if (g_pVertexShaderPCN) g_pVertexShaderPCN->Release();
	if (g_pVertexShaderPNT) g_pVertexShaderPNT->Release();
	if (g_pVertexShaderP) g_pVertexShaderP->Release();

	if (g_pPixelShader1) g_pPixelShader1->Release();
	if (g_pPixelShader2) g_pPixelShader2->Release();
	if (g_pPixelShader3) g_pPixelShader3->Release();
	if (g_pPixelShader4) g_pPixelShader4->Release();

	if (g_pDepthStencilView) g_pDepthStencilView->Release();
	if (g_pDepthStencil) g_pDepthStencil->Release();

	if (g_pRSState) g_pRSState->Release();
	if (g_pDSState) g_pDSState->Release();

	if (g_texEnvMap)g_texEnvMap->Release();
	if (g_tSRVEnvMap)g_tSRVEnvMap->Release();

	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pd3dDevice) g_pd3dDevice->Release();

	if (g_samplerTex2D)g_samplerTex2D->Release();
}

// 
// 
// Texture2D<float4> EnvTexture=

//[시험출제]
// view2world, world2envsphere
// Matrix mView2World = g_mView.Invert();
// MyObject& envSphre=g_sceneObjs["SPHRE_ENV"];
// Matrix mworld2envsphere =envSphre.mModel.Invert();
//  
// 
// cbLight.mView2EnvOS = mworld2envsphere*mView2World;