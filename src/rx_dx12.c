
#ifndef D3D12_NO_HELPERS
#define D3D12_NO_HELPERS
#endif
#ifndef CINTERFACE
#define CINTERFACE
#endif
#ifndef COBJMACROS
#define COBJMACROS
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <limits.h>
#include <windows.h>
#include <initguid.h> // only used for "#define INITGUID" to make the "IID_" GUIDs work in the RX_DX12_AS macro
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
/* D3D12 extension library. */
// #include <d3dx12.h>
#if (defined(WINAPI_FAMILY_PARTITION) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP))
#pragma comment (lib, "WindowsApp.lib")
#else
#pragma comment (lib, "user32.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "dxguid.lib")
#endif
#include "rx_dx12.h"
#include "rx_dx12_helper.h"

//#define PIPELINE_SPEC(NAME, INNTERTYPE, OUTERTYPE) typedef __declspec(align(void*)) struct rx_dx12##NAME { OUTERTYPE type, INNTERTYPE inner } rx_dx12##NAME;
//PIPELINE_SPEC(PipelineStateStreamFlags, D3D12_PIPELINE_STATE_FLAGS, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_FLAGS);

#define RX_FRAME_LAG 3
#define RX_DX12_ERR_COND(condition, error) { if (FAILED(condition)) {assert(0 && error); return error;} }
#define RX_RETURN_ON_ERRORCODE(ERRCODE) { rx_errorCode code = ERRCODE; if (code != rx_errorCode_ok) {assert(0 && code); return code;} }
// #define RX_DX12_AS(TYPE, IN, OUT) TYPE##_QueryInterface(IN, &IID_##TYPE, OUT)
#define RX_DX12_AS(TYPE, IN, TARGETTYPE, OUT) TYPE##_QueryInterface(IN, &IID_##TARGETTYPE, OUT)
#define RX_DX12_GET(obj) ((IUnknown*) obj)
#define RX_DX12_SAVE_RELEASE(class, obj) if (obj) { class##_Release(obj); obj=0; }

// This fixes heap corruption bug with DX12 headers when they return a struct
typedef void(__stdcall* fixed_GetCPUDescriptorHandleForHeapStart)(ID3D12DescriptorHeap* This, D3D12_CPU_DESCRIPTOR_HANDLE* pOut);

typedef struct rx_DX12Context {
    ID3D12Device2* device;
    //ID3D12DeviceContext* ctx;
	ID3D12Fence* fence;
	rx_u64 fenceValue;
	HANDLE fenceEvent;
	IDXGISwapChain4* swapChain;
	ID3D12Resource* backbuffers[RX_FRAMES];
	ID3D12GraphicsCommandList* commandList;
    ID3D12CommandQueue* mainCommandQueue;
    ID3D12CommandAllocator* commandAllocators[RX_FRAME_LAG];
    ID3D12DescriptorHeap*  rtvDescriptorHeap;
    rx_u32 rtvDescriptorSize;
    rx_u32 currentBackbufferIndex;
	rx_u64 frameFenceValues[RX_FRAME_LAG];

	/* features */
	rx_b32 tearingSupported;
    /* on-demand loaded d3dcompiler_47.dll handles */
    HINSTANCE d3dcompilerDll;
    rx_b32 d3dcompilerDllLoadFailed;
    pD3DCompile hlslCompileFunc;

    ID3D12CommandQueue* commandQueues[100];
} rx_DX12Context;

RX_INTERN rx_errorCode rx__dx12CreateSwapChain(
        const HWND hWnd, ID3D12CommandQueue* commandQueue,
        rx_u32 width, rx_u32 height,
        rx_u32 bufferCount, IDXGISwapChain4** outSwapchain,
		rx_b32 isTearingSupported, rx_b32 debug) {
    IDXGIFactory4* dxgiFactory4;
    UINT createFactoryFlags = debug ? DXGI_CREATE_FACTORY_DEBUG : 0;
    RX_DX12_ERR_COND(CreateDXGIFactory2(createFactoryFlags,  &IID_IDXGIFactory4, &dxgiFactory4), rx_errorCode_cantCreate);
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {
        .Width = width,
        .Height = height,
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .Stereo = FALSE,
        .SampleDesc = { 1, 0 },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = bufferCount,
        .Scaling = DXGI_SCALING_STRETCH,
        .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
        .Flags = (isTearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0) | DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT,
    };

    IDXGISwapChain1 *swapChain1;
	// This,pDevice,hWnd,pDesc,pFullscreenDesc,pRestrictToOutput,ppSwapChain
    RX_DX12_ERR_COND(IDXGIFactory4_CreateSwapChainForHwnd(
		dxgiFactory4,
		//device,
		RX_DX12_GET(commandQueue), // IUnkown is a command queue... strange
		hWnd,
        &swapChainDesc,
        NULL,
        NULL,
        &swapChain1), rx_errorCode_cantCreate);
    // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
    // will be handled manually.
	rx_errorCode errorCode = rx_errorCode_cantCreate;
	if (SUCCEEDED(IDXGIFactory4_MakeWindowAssociation(dxgiFactory4, hWnd, DXGI_MWA_NO_ALT_ENTER))) {
		
		if (SUCCEEDED(RX_DX12_AS(IDXGIAdapter1, swapChain1, IDXGISwapChain4, outSwapchain))) {
			errorCode = rx_errorCode_ok;
		}
	}
	
	IDXGIAdapter1_Release(swapChain1); // Don't need version 1 anymore.

    //ctx->dx12.currentBackBufferIndex = IDXGIFactory4_GetCurrentBackBufferIndex(dxgiSwapChain4);
	//IDXGIFactory4_SetMaximumFrameLatency(dxgiSwapChain4, BufferCount - 1);
	//ctx->dx12.swapChainEvent = IDXGIFactory4_GetFrameLatencyWaitableObject(dxgiSwapChain4);

    return errorCode;
}


char const* rx_dx12ErrorText(rx_errorCode error) {
	switch (error) {
		case rx_errorCode_dx12QueueInfoFailed: return "Queuing Dx12 device for information failed";
		case rx_errorCode_dx12GIFactoryCreationFailed: return "DXGI factory creation failed.";
		case rx_errorCode_dx12AdapterNotFound: return "No Direct3D feature level 11_0 adapter found.";
		case rx_errorCode_dx12AdapterInterfaceQueryFailed: return "DXGI adapter interface version 3 query failed.";
		case rx_errorCode_dx12DeviceCreationFailed: return "Direct3D 12 feature level 11_0 device creation failed.";
		case rx_errorCode_dx12GraphicsCommandQueueCreationFaile: return "Graphics command queue creation failed.";
		case rx_errorCode_dx12CopyCommandQueueCreationFailed: return "Copy command queue creation failed.";
	}
	return rx_null;
}

RX_INTERN rx_errorCode rx_dx12EnableDebugLayer() {
    // Always enable the debug layer before doing anything DX12 related
    // so all possible errors generated while creating DX12 objects
    // are caught by the debug layer.
    ID3D12Debug* debugInterface;
    RX_DX12_ERR_COND(D3D12GetDebugInterface(&IID_ID3D12Debug, &debugInterface), rx_errorCode_cantCreate);
    ID3D12Debug_EnableDebugLayer(debugInterface); // debugInterface->EnableDebugLayer();
    return rx_errorCode_ok;
}

RX_INTERN rx_b32 rx_dx12LoadHlslCompilerDll(rx_Context* ctx) {
    /* on UWP, don't do anything (not tested) */
    #if (defined(WINAPI_FAMILY_PARTITION) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP))
        return true;
    #else
        /* load DLL on demand */
        if ((ctx->dx12->d3dcompilerDll == 0) && !ctx->dx12->d3dcompilerDllLoadFailed) {
            ctx->dx12->d3dcompilerDll = LoadLibraryA("d3dcompiler_47.dll");
            if (ctx->dx12->d3dcompilerDll == 0) {
                /* don't attempt to load missing DLL in the future */
                /* SOKOL_LOG("failed to load d3dcompiler_47.dll!\n"); */
                ctx->dx12->d3dcompilerDllLoadFailed = rx_true;
                return rx_false;
            }
            /* look up function pointers */
            ctx->dx12->hlslCompileFunc = (pD3DCompile) GetProcAddress(ctx->dx12->d3dcompilerDll, "D3DCompile");
            RX_ASSERT(ctx->dx12->hlslCompileFunc);
        }
        return ctx->dx12->d3dcompilerDll != 0;
    #endif
}

#if (defined(WINAPI_FAMILY_PARTITION) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP))
#define rx_dx12Compile D3DCompile
#else
#define rx_dx12Compile ctx->dx12->hlslCompileFunc
#endif

#if 0
_SOKOL_PRIVATE ID3DBlob* rx_dx12CompileShader(rx_Context* ctx, const sg_shader_stage_desc* stage_desc, const char* target) {
    if (!rx_dx12LoadHlslCompilerDll(ctx)) {
        return rx_null;
    }
    ID3DBlob* output = rx_null;
    ID3DBlob* errors = rx_null;
    _sg_d3d11_D3DCompile(
        stage_desc->source,             /* pSrcData */
        strlen(stage_desc->source),     /* SrcDataSize */
        rx_null,                           /* pSourceName */
        rx_null,                           /* pDefines */
        rx_null,                           /* pInclude */
        stage_desc->entry ? stage_desc->entry : "main",     /* pEntryPoint */
        target,     /* pTarget (vs_5_0 or ps_5_0) */
        D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_OPTIMIZATION_LEVEL3,   /* Flags1 */
        0,          /* Flags2 */
        &output,    /* ppCode */
        &errors);   /* ppErrorMsgs */
    if (errors) {
        SOKOL_LOG((LPCSTR)ID3D10Blob_GetBufferPointer(errors));
        ID3D10Blob_Release(errors); errors = rx_null;
        return rx_null;
    }
    return output;
}
#endif

RX_API rx_graphicsPipeline rx_dx12CreateGraphicsPipeline(rx_Context* ctx, rx_GraphicsPipelineDesc* desc) {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC dx12Desc = { 0 };
    return (rx_graphicsPipeline) { .id = 0 };
}

/* crates an command queue and stores its pointer at the specified index in the CommandQueue pool */
RX_INTERN rx_errorCode rx_dx12CreateCommandQueue(rx_Context* ctx, rx_u32 index, D3D12_COMMAND_LIST_TYPE type) {
    D3D12_COMMAND_QUEUE_DESC desc = {
        .Type =     type,
        .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
        .Flags =    D3D12_COMMAND_QUEUE_FLAG_NONE,
        .NodeMask = 0,
    };
	
    RX_DX12_ERR_COND(ID3D12Device2_CreateCommandQueue(ctx->dx12->device, &desc, &IID_ID3D12CommandQueue, &ctx->dx12->commandQueues[index]), rx_errorCode_cantCreate);
 
    return rx_errorCode_ok;
}

RX_INTERN rx_errorCode rx__dx12CreateDevice(IDXGIAdapter4* adapter, ID3D12Device2** d3d12Device2Out, rx_b32 debug) {
    ID3D12Device2* d3d12Device2;


    RX_DX12_ERR_COND(D3D12CreateDevice(RX_DX12_GET(adapter), D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device2, d3d12Device2Out), rx_errorCode_cantCreate);
	RX_ASSERT((*d3d12Device2Out)->lpVtbl->CreateRenderTargetView);

    // Enable debug messages in debug mode.
    if (debug) {
        ID3D12InfoQueue* pInfoQueue;
		
        RX_DX12_ERR_COND(RX_DX12_AS(ID3D12Device2, *d3d12Device2Out, ID3D12InfoQueue, &pInfoQueue), rx_errorCode_dx12QueueInfoFailed);
        
        ID3D12InfoQueue_SetBreakOnSeverity(pInfoQueue, D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        ID3D12InfoQueue_SetBreakOnSeverity(pInfoQueue, D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        ID3D12InfoQueue_SetBreakOnSeverity(pInfoQueue, D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

        // Suppress whole categories of messages
        //D3D12_MESSAGE_CATEGORY Categories[] = {};

        // Suppress messages based on their severity level
        D3D12_MESSAGE_SEVERITY Severities[] = {
            D3D12_MESSAGE_SEVERITY_INFO
        };

        // Suppress individual messages by their ID
        D3D12_MESSAGE_ID denyIds[] = {
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
        };

        D3D12_INFO_QUEUE_FILTER newFilter = {
			//NewFilter.DenyList.NumCategories = _countof(Categories);
			//NewFilter.DenyList.pCategoryList = Categories;
			.DenyList.NumSeverities = _countof(Severities),
			.DenyList.pSeverityList = Severities,
			.DenyList.NumIDs = _countof(denyIds),
			.DenyList.pIDList = denyIds,
		};

        RX_DX12_ERR_COND(ID3D12InfoQueue_PushStorageFilter(pInfoQueue, &newFilter), rx_errorCode_cantCreate);
    }

    //*d3d12Device2Out = d3d12Device2;
    return rx_errorCode_ok;
}

RX_INTERN rx_errorCode rx__dx12CreateCommandQueue(ID3D12Device2* device, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandQueue** d3d12CommandQueue) {

    D3D12_COMMAND_QUEUE_DESC desc = {
        .Type = type,
        .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
        .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        .NodeMask = 0,
    };

    RX_DX12_ERR_COND(ID3D12Device2_CreateCommandQueue(device , &desc, &IID_ID3D12CommandQueue, d3d12CommandQueue), rx_errorCode_cantCreate);

    return rx_errorCode_ok;
}

RX_INTERN rx_b32 rx__dx12CheckTearingSupport() {
    BOOL allowTearing = FALSE;

    // Rather than create the DXGI 1.5 factory interface directly, we create the
    // DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
    // graphics debugging tools which will not support the 1.5 factory interface 
    // until a future update.
    IDXGIFactory4* factory4;
    if (SUCCEEDED(CreateDXGIFactory1(&IID_IDXGIFactory4, &factory4))) {
        IDXGIFactory5* factory5;
        if (SUCCEEDED(RX_DX12_AS(IDXGIFactory5, factory4, IDXGIFactory5, &factory5))) {
            if (FAILED(IDXGIFactory5_CheckFeatureSupport(factory5, DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing)))) {
				allowTearing = FALSE;
            }
			IDXGIFactory4_Release(factory5);
        }
    }
	IDXGIFactory4_Release(factory4);
    return allowTearing == TRUE;
}

RX_INTERN rx_errorCode rx__dx12GetAdapter(rx_b32 useWarp, rx_b32 debug, IDXGIAdapter4** adapterOut) {
    IDXGIFactory4* dxgiFactory;
    rx_u32 createFactoryFlags = 0;
    if (debug) {
        createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
    }

    RX_DX12_ERR_COND(CreateDXGIFactory2(createFactoryFlags, &IID_IDXGIFactory4, &dxgiFactory), rx_errorCode_cantCreate);

    IDXGIAdapter1* dxgiAdapter1;
    IDXGIAdapter4* dxgiAdapter4;

    if (useWarp) {
        RX_DX12_ERR_COND(IDXGIFactory4_EnumWarpAdapter(dxgiFactory, &IID_IDXGIAdapter1, &dxgiAdapter1), rx_errorCode_cantCreate);

        RX_DX12_ERR_COND(RX_DX12_AS(IDXGIAdapter1, dxgiAdapter1, IDXGIAdapter4, &dxgiAdapter4), rx_errorCode_cantCreate);
    } else {
        size_t maxDedicatedVideoMemory = 0;
        for (rx_u32 i = 0; IDXGIFactory4_EnumAdapters1(dxgiFactory, i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i) {
            DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
            IDXGIAdapter1_GetDesc1(dxgiAdapter1, &dxgiAdapterDesc1);

            // Check to see if the adapter can create a D3D12 device without actually 
            // creating it. The adapter with the largest dedicated video memory
            // is favored.
            if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
				SUCCEEDED(D3D12CreateDevice(RX_DX12_GET(dxgiAdapter1), D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, NULL)) &&
				dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory )
            {
                maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
				
				RX_DX12_ERR_COND(RX_DX12_AS(IDXGIAdapter1, dxgiAdapter1, IDXGIAdapter4, &dxgiAdapter4), rx_errorCode_cantCreate);
            }
        }
    }
    *adapterOut = dxgiAdapter4;
    return rx_errorCode_ok;
}

RX_INTERN rx_errorCode rx__dx12CreateDescriptorHeap(ID3D12Device2* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, ID3D12DescriptorHeap** descriptorHeap) {
    D3D12_DESCRIPTOR_HEAP_DESC desc = {0};
    desc.NumDescriptors = numDescriptors;
    desc.Type = type;

    RX_DX12_ERR_COND(ID3D12Device2_CreateDescriptorHeap(device , &desc, &IID_ID3D12DescriptorHeap, descriptorHeap), rx_errorCode_cantCreate);

    return rx_errorCode_ok;
}

RX_INTERN rx_errorCode rx__dx12UpdateRenderTargetViews(rx_Context* ctx, ID3D12Device2* device, IDXGISwapChain4* swapChain, ID3D12DescriptorHeap* descriptorHeap) {
    rx_u32 rtvDescriptorSize = ID3D12Device2_GetDescriptorHandleIncrementSize(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE descHandle; // = ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart(descriptorHeap);
    //DX12Helper_GetCPUDescriptorHandleForHeapStart(descriptorHeap, &descHandle);
	((fixed_GetCPUDescriptorHandleForHeapStart)descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart)(descriptorHeap, &descHandle);

	//D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	

    for (int i = 0; i < RX_FRAMES; ++i) {
        ID3D12Resource* backBuffer = rx_null;
        RX_DX12_ERR_COND(IDXGISwapChain4_GetBuffer(swapChain, i, &IID_ID3D12Resource, &backBuffer), rx_errorCode_cantCreate);

		// This,pResource,pDesc,DestDescripto
		RX_ASSERT(device->lpVtbl->CreateRenderTargetView);
         ID3D12Device2_CreateRenderTargetView(device, backBuffer, NULL, descHandle);
        ctx->dx12->backbuffers[i] = backBuffer;
		
        descHandle.ptr += rtvDescriptorSize;
    }

    return rx_errorCode_ok;
}

RX_INTERN rx_errorCode rx__dx12CreateFence(ID3D12Device2* device, ID3D12Fence** fence) {
	RX_DX12_ERR_COND(ID3D12Device2_CreateFence(device, 0, D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence, fence), rx_errorCode_cantCreate);

	return rx_errorCode_ok;
}

RX_INTERN rx_errorCode rx__dx12CreateCommandAllocator(ID3D12Device2* device, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator** commandAllocator) {
	RX_DX12_ERR_COND(ID3D12Device2_CreateCommandAllocator(device, type, &IID_ID3D12CommandAllocator, commandAllocator), rx_errorCode_cantCreate);

	return rx_errorCode_ok;
}

RX_INTERN rx_errorCode rx__dx12CreateCommandList(ID3D12Device2* device, ID3D12CommandAllocator* commandAllocator, D3D12_COMMAND_LIST_TYPE type, ID3D12GraphicsCommandList** commandList) {
	RX_DX12_ERR_COND(ID3D12Device2_CreateCommandList(device, 0, type, commandAllocator, NULL, &IID_ID3D12GraphicsCommandList, commandList), rx_errorCode_cantCreate);

	RX_DX12_ERR_COND(ID3D12GraphicsCommandList_Close(*commandList), rx_errorCode_cantCreate);

	return rx_errorCode_ok;
}

RX_INTERN rx_errorCode  rx__dx12CreateEventHandle(HANDLE* fenceEvent) {
	*fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	return fenceEvent ? rx_errorCode_ok : rx_errorCode_cantCreateFence;
}

rx_errorCode rx__dx12Signal(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, rx_u64 *fenceValue, rx_u64* fenceValueForSignal) {
	*fenceValueForSignal = ++(*fenceValue);
	ID3D12CommandQueue_Signal(commandQueue, fence, fenceValueForSignal);
	RX_DX12_ERR_COND(ID3D12CommandQueue_Signal(commandQueue, fence, fenceValueForSignal), rx_errorCode_error);
	// ThrowIfFailed(commandQueue->Signal(fence.Get(), fenceValueForSignal));

	return rx_errorCode_ok;
}

rx_errorCode rx__dx12WaitForFenceValue(ID3D12Fence* fence, rx_u64 fenceValue, HANDLE fenceEvent) {
	if (ID3D12Fence_GetCompletedValue(fence) < fenceValue) {
		RX_DX12_ERR_COND(ID3D12Fence_SetEventOnCompletion(fence, fenceValue, fenceEvent), rx_errorCode_error);
		WaitForSingleObject(fenceEvent, ULONG_MAX);
		return rx_errorCode_ok;
	}
	return rx_errorCode_error;
}

rx_errorCode rx__dx12Flush(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, rx_u64* fenceValue, HANDLE fenceEvent) {
	rx_u64 fenceValueForSignal = 0;
	RX_RETURN_ON_ERRORCODE(rx__dx12Signal(commandQueue, fence, fenceValue, &fenceValueForSignal));
	rx__dx12WaitForFenceValue(fence, fenceValueForSignal, fenceEvent);
	return rx_errorCode_ok;
}

rx_errorCode rx_dx12destroyContext(rx_Context* ctx) {
	// Make sure the command queue has finished all commands before closing.
	rx__dx12Flush(ctx->dx12->mainCommandQueue, ctx->dx12->fence, &ctx->dx12->fenceValue, ctx->dx12->fenceEvent);

	CloseHandle(ctx->dx12->fenceEvent);
	ID3D12CommandQueue_Release(ctx->dx12->mainCommandQueue);

	ID3D12Fence_Release(ctx->dx12->fence);

	for (int i = 0; i < RX_FRAMES; i++) {
		ID3D12Resource_Release(ctx->dx12->backbuffers[i]);
	}

	IDXGISwapChain4_Release(ctx->dx12->swapChain);
	ID3D12CommandList_Release(ctx->dx12->commandList);
	ID3D12DescriptorHeap_Release(ctx->dx12->rtvDescriptorHeap);
	ID3D12Device2_Release(ctx->dx12->device);

	return rx_errorCode_ok;
}


D3D12_RESOURCE_BARRIER rx__barrierTransistion(ID3D12Resource * pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter) {
	// default value
	UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	// default value
	D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	return (D3D12_RESOURCE_BARRIER) {
		.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
		.Flags = flags,
		.Transition.pResource = pResource,
		.Transition.StateBefore = stateBefore,
		.Transition.StateAfter = stateAfter,
		.Transition.Subresource = subresource,
	};
}

D3D12_CPU_DESCRIPTOR_HANDLE rx__dx12GetCpuDescriptorHandleFromStart(ID3D12DescriptorHeap* descriptorHeap, INT offsetInDescriptors, UINT descriptorIncrementSize) {
	D3D12_CPU_DESCRIPTOR_HANDLE descHandle;
	((fixed_GetCPUDescriptorHandleForHeapStart)descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart)(descriptorHeap, &descHandle);
	return (D3D12_CPU_DESCRIPTOR_HANDLE) {
		.ptr = descHandle.ptr + offsetInDescriptors * descriptorIncrementSize
	};
}

void rx_dx12Commit(rx_Context* ctx) {
	// Testing simple stuff
	ID3D12CommandAllocator* commandAllocator = ctx->dx12->commandAllocators[ctx->dx12->currentBackbufferIndex];
	ID3D12Resource* backBuffer = ctx->dx12->backbuffers[ctx->dx12->currentBackbufferIndex];

	ID3D12CommandAllocator_Reset(commandAllocator);
	ID3D12GraphicsCommandList_Reset(ctx->dx12->commandList, commandAllocator, NULL);
	// g_CommandList->Reset(commandAllocator.Get(), NULL);

	// Clear the render target.
	{
		D3D12_RESOURCE_BARRIER barrier = rx__barrierTransistion(
			backBuffer,
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		);
		ID3D12GraphicsCommandList_ResourceBarrier(ctx->dx12->commandList, 1, &barrier);
		//g_CommandList->ResourceBarrier(1, &barrier);

		FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rx__dx12GetCpuDescriptorHandleFromStart(ctx->dx12->rtvDescriptorHeap, ctx->dx12->currentBackbufferIndex, ctx->dx12->rtvDescriptorSize);

		ID3D12GraphicsCommandList_ClearRenderTargetView(ctx->dx12->commandList, rtvHandle, clearColor, 0, NULL);
	}

	// Done testing

    // present
	D3D12_RESOURCE_BARRIER barrier = rx__barrierTransistion(ctx->dx12->backbuffers[ctx->dx12->currentBackbufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    // CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(),D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

    // g_CommandList->ResourceBarrier(1, &barrier);
    ID3D12GraphicsCommandList_ResourceBarrier(ctx->dx12->commandList, 1, &barrier);

	ID3D12GraphicsCommandList_Close(ctx->dx12->commandList); // test for error

    ID3D12CommandList* const commandLists[] = {
		(ID3D12CommandList*) ctx->dx12->commandList
    };
	ID3D12CommandQueue_ExecuteCommandLists(ctx->dx12->mainCommandQueue, _countof(commandLists), commandLists);
    //g_CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

    UINT syncInterval = ctx->useVsync ? 1 : 0;
    UINT presentFlags = ctx->dx12->tearingSupported && !ctx->useVsync ? DXGI_PRESENT_ALLOW_TEARING : 0;

	IDXGISwapChain4_Present(ctx->dx12->swapChain, syncInterval, presentFlags);

    rx__dx12Signal(ctx->dx12->mainCommandQueue, ctx->dx12->fence, &ctx->dx12->fenceValue, &ctx->dx12->frameFenceValues[ctx->dx12->currentBackbufferIndex]);

	ctx->dx12->currentBackbufferIndex = IDXGISwapChain4_GetCurrentBackBufferIndex(ctx->dx12->swapChain);

    rx__dx12WaitForFenceValue(ctx->dx12->fence, ctx->dx12->frameFenceValues[ctx->dx12->currentBackbufferIndex], ctx->dx12->fenceEvent);
}

void Render() {

	// Present
	{
	}
}
#if 0

void Resize(uint32_t width, uint32_t height)
{
	if (g_ClientWidth != width || g_ClientHeight != height)
	{
		// Don't allow 0 size swap chain back buffers.
		g_ClientWidth = std::max(1u, width);
		g_ClientHeight = std::max(1u, height);

		// Flush the GPU queue to make sure the swap chain's back buffers
		// are not being referenced by an in-flight command list.
		Flush(g_CommandQueue, g_Fence, g_FenceValue, g_FenceEvent);

		for (int i = 0; i < g_NumFrames; ++i)
		{
			// Any references to the back buffers must be released
			// before the swap chain can be resized.
			g_BackBuffers[i].Reset();
			g_FrameFenceValues[i] = g_FrameFenceValues[g_CurrentBackBufferIndex];
		}

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		ThrowIfFailed(g_SwapChain->GetDesc(&swapChainDesc));
		ThrowIfFailed(g_SwapChain->ResizeBuffers(g_NumFrames, g_ClientWidth, g_ClientHeight,
			swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

		g_CurrentBackBufferIndex = g_SwapChain->GetCurrentBackBufferIndex();

		UpdateRenderTargetViews(g_Device, g_SwapChain, g_RTVDescriptorHeap);
	}
}

void SetFullscreen(bool fullscreen)
{
	if (g_Fullscreen != fullscreen)
	{
		g_Fullscreen = fullscreen;

		if (g_Fullscreen) // Switching to fullscreen.
		{
			// Store the current window dimensions so they can be restored 
			// when switching out of fullscreen state.
			::GetWindowRect(g_hWnd, &g_WindowRect);

			// Set the window style to a borderless window so the client area fills
			// the entire screen.
			UINT windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

			::SetWindowLongW(g_hWnd, GWL_STYLE, windowStyle);

			// Query the name of the nearest display device for the window.
			// This is required to set the fullscreen dimensions of the window
			// when using a multi-monitor setup.
			HMONITOR hMonitor = ::MonitorFromWindow(g_hWnd, MONITOR_DEFAULTTONEAREST);
			MONITORINFOEX monitorInfo = {};
			monitorInfo.cbSize = sizeof(MONITORINFOEX);
			::GetMonitorInfo(hMonitor, &monitorInfo);

			::SetWindowPos(g_hWnd, HWND_TOP,
				monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.top,
				monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
				SWP_FRAMECHANGED | SWP_NOACTIVATE);

			::ShowWindow(g_hWnd, SW_MAXIMIZE);
		}
		else
		{
			// Restore all the window decorators.
			::SetWindowLong(g_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

			::SetWindowPos(g_hWnd, HWND_NOTOPMOST,
				g_WindowRect.left,
				g_WindowRect.top,
				g_WindowRect.right - g_WindowRect.left,
				g_WindowRect.bottom - g_WindowRect.top,
				SWP_FRAMECHANGED | SWP_NOACTIVATE);

			::ShowWindow(g_hWnd, SW_NORMAL);
		}
	}
}
#endif

rx_errorCode rx_dx12CreateContext(rx_Context* ctx, const rx_Desc* desc) {
    ctx->dx12 = malloc(sizeof(rx_DX12Context));
	if (!ctx->dx12) {
		return rx_errorCode_allocFailed;
	}
	RX_ASSERT(desc->dx12Hwnd && "win32 window handle missing");
	memset(ctx->dx12, 0, sizeof(rx_DX12Context));

    IDXGIAdapter4* dxgiAdapter4 = rx_null;
    RX_RETURN_ON_ERRORCODE(rx__dx12GetAdapter(desc->useWrap, ctx->debug, &dxgiAdapter4));

    ID3D12Device2* d3d12Device = rx_null;
	rx_errorCode code = rx__dx12CreateDevice(dxgiAdapter4, &d3d12Device, ctx->debug);
	ctx->dx12->device = d3d12Device;
	ctx->dx12->backbuffers[0] = rx_null;
	if (code != rx_errorCode_ok) {
		IDXGIAdapter4_Release(dxgiAdapter4);
		return code;
	}
	
    RX_RETURN_ON_ERRORCODE(rx__dx12CreateCommandQueue(d3d12Device, D3D12_COMMAND_LIST_TYPE_DIRECT, &ctx->dx12->mainCommandQueue));
	ctx->dx12->tearingSupported = rx__dx12CheckTearingSupport();
	RX_RETURN_ON_ERRORCODE(rx__dx12CreateSwapChain(
		desc->dx12Hwnd,
		ctx->dx12->mainCommandQueue,
		ctx->width,
		ctx->height,
		RX_FRAMES,
		&ctx->dx12->swapChain,
		ctx->dx12->tearingSupported,
		ctx->debug
	));

    ctx->dx12->currentBackbufferIndex = IDXGISwapChain4_GetCurrentBackBufferIndex(ctx->dx12->swapChain);

    RX_RETURN_ON_ERRORCODE(rx__dx12CreateDescriptorHeap(d3d12Device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, RX_FRAMES, &ctx->dx12->rtvDescriptorHeap));
	ctx->dx12->rtvDescriptorSize = ID3D12Device2_GetDescriptorHandleIncrementSize(d3d12Device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    
    RX_RETURN_ON_ERRORCODE(rx__dx12UpdateRenderTargetViews(ctx, d3d12Device, ctx->dx12->swapChain, ctx->dx12->rtvDescriptorHeap));
	// IID_IDXGIAdapter4

	for (int i = 0; i < RX_FRAMES; i++) {
		RX_RETURN_ON_ERRORCODE(rx__dx12CreateCommandAllocator(d3d12Device, D3D12_COMMAND_LIST_TYPE_DIRECT, &ctx->dx12->commandAllocators[i]));
	}

	RX_RETURN_ON_ERRORCODE(rx__dx12CreateCommandList(d3d12Device, ctx->dx12->commandAllocators[ctx->dx12->currentBackbufferIndex], D3D12_COMMAND_LIST_TYPE_DIRECT, &ctx->dx12->commandList));

	RX_RETURN_ON_ERRORCODE(rx__dx12CreateFence(d3d12Device, &ctx->dx12->fence));

	RX_RETURN_ON_ERRORCODE(rx__dx12CreateEventHandle(&ctx->dx12->fenceEvent));


	return rx_errorCode_ok;
	//rx_u32 currentBackBufferIndex = IDXGISwapChain4_GetCurrentBackBufferIndex(swapChain);
	
	//RX_RETURN_ON_ERRORCODE(rx__dx12CreateCommandList(d3d12Device, ctx->dx12->commandAllocators[currentBackBufferIndex], D3D12_COMMAND_LIST_TYPE_DIRECT, &ctx->dx12->commandList));

	//g_FenceEvent = CreateEventHandle();
#if 0 /* this comes next */

    g_IsInitialized = true;

    ::ShowWindow(g_hWnd, SW_SHOW);

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }

    // Make sure the command queue has finished all commands before closing.
    Flush(g_CommandQueue, g_Fence, g_FenceValue, g_FenceEvent);

    ::CloseHandle(g_FenceEvent);
#endif







#if 0 /* delete this? */
    IDXGIFactory4* dxgiFactory;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
    RX_DX12_ERR_COND(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)), rx_errorCode_cantCreate);

    IDXGIAdapter1 *dxgiAdapter1;
    IDXGIAdapter4 *dxgiAdapter4;
    // Warp enables software emulation of missing features in the used graphics card
    rx_b32 useWarp = rx_false;
    if (useWarp) {
        RX_DX12_ERR_COND(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
        RX_DX12_ERR_COND(dxgiAdapter1->As(&dxgiAdapter4));
    } else {
        rx_mm maxDedicatedVideoMemory = 0;
        for (rx_u32 i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i) {
            DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
            dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);
 
            // Check to see if the adapter can create a D3D12 device without actually 
            // creating it. The adapter with the largest dedicated video memory
            // is favored.
            if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
                SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(), 
                    D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) && 
                dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory )
            {
                maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
                ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
            }
        }
    }

	//return rx_vkCreatePhysicalDevice(ctx, desc);
#endif
}