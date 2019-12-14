#include "rx_renderer.h"
#ifndef COBJMACROS
#define COBJMACROS
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>
#include <stdbool.h>
#include <assert.h>

#if 0
#include <dxgi1_6.h>
#include <d3d12.h>
static bool quit_requested = false;
static IDXGIAdapter1* dx12adapter1 = NULL;
static IDXGIAdapter4* dx12adapter4 = NULL;

static LRESULT CALLBACK d3d12_winproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void d3d12_init(int w, int h, int sample_count, const wchar_t* title) {
    RegisterClassW(&(WNDCLASSW){
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc = (WNDPROC) d3d12_winproc,
        .hInstance = GetModuleHandleW(NULL),
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .hIcon = LoadIcon(NULL, IDI_WINLOGO),
        .lpszClassName = L"RXD3D12"
    });

    /* create window */
    in_create_window = true;
    RECT rect = { .left = 0, .top = 0, .right = w, .bottom = h };
    AdjustWindowRectEx(&rect, win_style, FALSE, win_ex_style);
    const int win_width = rect.right - rect.left;
    const int win_height = rect.bottom - rect.top;
    hwnd = CreateWindowExW(
        win_ex_style,       // dwExStyle
        L"RXD3D12",      // lpClassName
        title,              // lpWindowName
        win_style,          // dwStyle
        CW_USEDEFAULT,      // X
        CW_USEDEFAULT,      // Y
        win_width,          // nWidth
        win_height,         // nHeight
        NULL,               // hWndParent
        NULL,               // hMenu
        GetModuleHandle(NULL),  //hInstance
        NULL);              // lpParam
    ShowWindow(hwnd, SW_SHOW);
    in_create_window = false;


    IDXGIAdapter1* dxgiAdapter1;
    IDXGIAdapter4* dxgiAdapter4;

    /* create device and swap chain */
    swap_chain_desc = (DXGI_SWAP_CHAIN_DESC) {
        .BufferDesc = {
            .Width = w,
            .Height = h,
            .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
            .RefreshRate = {
                .Numerator = 60,
                .Denominator = 1
            }
        },
        .OutputWindow = hwnd,
        .Windowed = true,
        .SwapEffect = DXGI_SWAP_EFFECT_DISCARD,
        .BufferCount = 1,
        .SampleDesc = {
            .Count = sample_count,
            .Quality = sample_count > 1 ? D3D12_STANDARD_MULTISAMPLE_PATTERN : 0
        },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT
    };
    int create_flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
    #ifdef _DEBUG
        create_flags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif
    D3D_FEATURE_LEVEL feature_level;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL,                       /* pAdapter (use default) */
        D3D_DRIVER_TYPE_HARDWARE,   /* DriverType */
        NULL,                       /* Software */
        create_flags,               /* Flags */
        NULL,                       /* pFeatureLevels */
        0,                          /* FeatureLevels */
        D3D11_SDK_VERSION,          /* SDKVersion */
        &swap_chain_desc,           /* pSwapChainDesc */
        &swap_chain,                /* ppSwapChain */
        &device,                    /* ppDevice */
        &feature_level,             /* pFeatureLevel */
        &device_context);           /* ppImmediateContext */
    assert(SUCCEEDED(hr) && swap_chain && device && device_context);

    /* default render target and depth-stencil-buffer */
    d3d11_create_default_render_target();
}
#endif

static bool in_create_window = false;
static DWORD win_style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX;
static DWORD win_ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
static HWND hwnd = NULL;
bool d3d11_process_events() {
	MSG msg;
	BOOL quit_requested = false;
	while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (WM_QUIT == msg.message) {
			quit_requested = true;
		}
		else {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}
	return !quit_requested;
}
LRESULT CALLBACK d3d12_winproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
void d3d12_init(int w, int h, int sample_count, const wchar_t* title) {
	RegisterClassW(&(WNDCLASSW) {
		.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
			.lpfnWndProc = (WNDPROC)d3d12_winproc,
			.hInstance = GetModuleHandleW(NULL),
			.hCursor = LoadCursor(NULL, IDC_ARROW),
			.hIcon = LoadIcon(NULL, IDI_WINLOGO),
			.lpszClassName = L"RXD3D12"
	});

	/* create window */
	in_create_window = true;
	RECT rect = { .left = 0,.top = 0,.right = w,.bottom = h };
	AdjustWindowRectEx(&rect, win_style, FALSE, win_ex_style);
	const int win_width = rect.right - rect.left;
	const int win_height = rect.bottom - rect.top;
	hwnd = CreateWindowExW(
		win_ex_style,       // dwExStyle
		L"RXD3D12",      // lpClassName
		title,              // lpWindowName
		win_style,          // dwStyle
		CW_USEDEFAULT,      // X
		CW_USEDEFAULT,      // Y
		win_width,          // nWidth
		win_height,         // nHeight
		NULL,               // hWndParent
		NULL,               // hMenu
		GetModuleHandle(NULL),  //hInstance
		NULL);              // lpParam
	ShowWindow(hwnd, SW_SHOW);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
	/*
	printf("Foooo!");
	char str[256];
	int number = 20;
	sprintf_s(str, "It works! - number: %d \n", number);
	*/
	OutputDebugString("Setup dx!\n");
	d3d12_init(800, 650, 1, L"D3D12 test");
	rx_Context* ctx = rx_createContext(&(rx_Desc) {
		.dx12Hwnd = hwnd
	});
	assert(ctx && "No context!");
	while (d3d11_process_events()) {
		rx_commit(ctx);
	};
	return 0;
}