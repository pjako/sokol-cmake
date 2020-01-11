
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
#include "rx_renderer.h"
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


typedef struct rx__Dx12VertexBuffer {
	int stride;
	rx_buffer buffer;
} rx__Dx12VertexBuffer;

typedef struct rx__Dx12Pipeline {
	rx_u16 generation;
	ID3D12PipelineState* dx12Pipeline;
	ID3D12RootSignature* dx12RootSignature;
	rx__Dx12VertexBuffer vertexBuffers[RX_MAX_VERTEX_ATTRIBUTES];
	int vertexBufferCount;
} rx__Dx12Pipeline;

typedef struct rx__DX12GraphicsProgram {
	rx_u32 generation;
	void* pixelShaderBlob;
	rx_u64 pixelShaderBlobSize;
	void* fragmentShaderBlob;
	rx_u64 fragmentShaderBlobSize;
} rx__DX12GraphicsProgram;

#if 0
typedef struct rx_dx12PipelinePool {
	ID3D12Heap* heap;
	struct rx_dx12PipelinePool* next;
	rx__Dx12Pipeline pipelines[RX_PIPELINES_PER_POOL];
} rx_dx12PipelinePool;
#endif
// Fence

typedef enum rx__dx12CommandQueueType {
	rx__dx12CommandQueueType_graphics,
	rx__dx12CommandQueueType_copy,
	rx__dx12CommandQueueType__count
} rx__dx12CommandQueueType;

typedef struct rx__Dx12Fence {
	rx__dx12CommandQueueType queueType;
	ID3D12Fence* dx12Fence;
	HANDLE completionEvent;
	rx_u64 awaitedValue;
} rx__Dx12Fence;

// CommandList

typedef struct rx__DX12GfxCmdList {
	rx_u32 generation;
	rx__dx12CommandQueueType queueType;
	ID3D12CommandAllocator* dx12Allocator;
	ID3D12CommandList* dx12CmdList;
	ID3D12GraphicsCommandList* dx12GfxCmdList;
} rx__DX12GfxCmdList;

typedef struct rx__DX12ComputeCmdList {
	rx_u32 generation;
	rx__dx12CommandQueueType queueType;
	ID3D12CommandAllocator* dx12Allocator;
	ID3D12CommandList* dx12CmdList;
} rx__DX12ComputeCmdList;

// Buffer

typedef struct rx__Dx12Buffer {
	rx_u32 generation;
	rx_u32 size;
	rx_bufferAccess access;
	ID3D12Resource* dx12Resource;
	D3D12_GPU_VIRTUAL_ADDRESS dx12GpuVirtualAddress;
} rx__Dx12Buffer;

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

	struct {
		rx__DX12GfxCmdList gfxLists[12];
        rx__Dx12Buffer buffers[12];
		rx__DX12GraphicsProgram graphicsPrograms[12];
		rx__Dx12Pipeline pipelines[12];
	} pools;

	/* features */
	rx_b32 tearingSupported;
    /* on-demand loaded d3dcompiler_47.dll handles */
	ID3D12Debug* debugLayer;
    HINSTANCE d3dcompilerDll;
    rx_b32 d3dcompilerDllLoadFailed;
    pD3DCompile hlslCompileFunc;

    ID3D12CommandQueue* commandQueues[100];
} rx_DX12Context;

static DXGI_FORMAT rx__mapIndexBuffer[rx_indexType__count] = {
	[rx_indexType__default] = DXGI_FORMAT_R16_UINT,
	[rx_indexType_none] = DXGI_FORMAT_UNKNOWN,
	[rx_indexType_u16] = DXGI_FORMAT_R16_UINT,
	[rx_indexType_u32] = DXGI_FORMAT_R32_UINT
};

rx_errorCode rx__dx12CreateSwapChain(
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

inline void MemcpySubresource(const D3D12_MEMCPY_DEST* pDest, const D3D12_SUBRESOURCE_DATA* pSrc, SIZE_T RowSizeInBytes, UINT NumRows, UINT NumSlices) {
    for (UINT z = 0; z < NumSlices; ++z) {
        BYTE* pDestSlice = (BYTE*)(pDest->pData) + pDest->SlicePitch * z;
        const BYTE* pSrcSlice = (BYTE*)(pSrc->pData) + pSrc->SlicePitch * z;
        for (UINT y = 0; y < NumRows; ++y) {
            memcpy(pDestSlice + pDest->RowPitch * y, pSrcSlice + pSrc->RowPitch * y, RowSizeInBytes);
        }
    }
}

inline UINT64 UpdateSubresources(ID3D12GraphicsCommandList* pCmdList, ID3D12Resource* pDestinationResource, ID3D12Resource* pIntermediate,
    UINT FirstSubresource, UINT NumSubresources, UINT64 RequiredSize, const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts,
    const UINT* pNumRows,const UINT64* pRowSizesInBytes, const D3D12_SUBRESOURCE_DATA* pSrcData) {
    // Minor validation
    
    D3D12_RESOURCE_DESC IntermediateDesc = ID3D12Resource_GetDesc(pIntermediate);
    D3D12_RESOURCE_DESC DestinationDesc = ID3D12Resource_GetDesc(pDestinationResource);
    if (IntermediateDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER || IntermediateDesc.Width < RequiredSize + pLayouts[0].Offset || 
        RequiredSize > (SIZE_T)-1 || (DestinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER && (FirstSubresource != 0 || NumSubresources != 1))) {
        return 0;
    }
    
    BYTE* pData;
    HRESULT hr = ID3D12Resource_Map(pIntermediate, 0, rx_null, (void**)(&pData));
    if (FAILED(hr)) {
        return 0;
    }
    
    for (UINT i = 0; i < NumSubresources; ++i) {
        if (pRowSizesInBytes[i] > (SIZE_T)-1) {
            return 0;
        }
        D3D12_MEMCPY_DEST DestData = { pData + pLayouts[i].Offset, pLayouts[i].Footprint.RowPitch, pLayouts[i].Footprint.RowPitch * pNumRows[i] };
        MemcpySubresource(&DestData, &pSrcData[i], (SIZE_T)pRowSizesInBytes[i], pNumRows[i], pLayouts[i].Footprint.Depth);
    }
    ID3D12Resource_Unmap(pIntermediate, 0, rx_null);
    
    if (DestinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {
        D3D12_BOX srcBox = (D3D12_BOX) {
            .left = (rx_u32) (pLayouts[0].Offset),
            .top = 0,
            .front = 0,
            .right = (rx_u32) (pLayouts[0].Offset + pLayouts[0].Footprint.Width),
            .bottom = 1,
            .back = 1,
        };
        // CD3DX12_BOX SrcBox( UINT( pLayouts[0].Offset ), UINT( pLayouts[0].Offset + pLayouts[0].Footprint.Width ) );
        ID3D12GraphicsCommandList_CopyBufferRegion(pCmdList,pDestinationResource, 0, pIntermediate, pLayouts[0].Offset, pLayouts[0].Footprint.Width);
    } else {
        for (UINT i = 0; i < NumSubresources; ++i) {
            D3D12_TEXTURE_COPY_LOCATION dst = (D3D12_TEXTURE_COPY_LOCATION) {
                .pResource = pDestinationResource,
                .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
                .SubresourceIndex = i + FirstSubresource,
            };

            D3D12_TEXTURE_COPY_LOCATION src = (D3D12_TEXTURE_COPY_LOCATION) {
                .pResource = pDestinationResource,
                .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
                .PlacedFootprint = pLayouts[i],
            };

            ID3D12GraphicsCommandList_CopyTextureRegion(pCmdList, &dst, 0, 0, 0, &src, rx_null);
        }
    }
    return RequiredSize;
}

inline UINT64 UpdateHeapSubresources(ID3D12GraphicsCommandList* pCmdList, ID3D12Resource* pDestinationResource, ID3D12Resource* pIntermediate,
    UINT64 IntermediateOffset, UINT FirstSubresource, UINT NumSubresources, D3D12_SUBRESOURCE_DATA* pSrcData) {
    UINT64 RequiredSize = 0;
    UINT64 MemToAlloc = (UINT64)(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * NumSubresources;
    if (MemToAlloc > SIZE_MAX) {
       return 0;
    }
    void* pMem = HeapAlloc(GetProcessHeap(), 0, (SIZE_T)(MemToAlloc));
    if (pMem == rx_null) {
       return 0;
    }
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)(pMem);
    UINT64* pRowSizesInBytes = (UINT64*)(pLayouts + NumSubresources);
    UINT* pNumRows = (UINT*)(pRowSizesInBytes + NumSubresources);
    
    D3D12_RESOURCE_DESC Desc = ID3D12Resource_GetDesc(pDestinationResource);
    ID3D12Device* pDevice;
	
    ID3D12Resource_GetDevice(pDestinationResource, &IID_ID3D12Device, (void**)(&pDevice));
    ID3D12Device_GetCopyableFootprints(pDevice, &Desc, FirstSubresource, NumSubresources, IntermediateOffset, pLayouts, pNumRows, pRowSizesInBytes, &RequiredSize);
    ID3D12Device_Release(pDevice);
    
    UINT64 Result = UpdateSubresources(pCmdList, pDestinationResource, pIntermediate, FirstSubresource, NumSubresources, RequiredSize, pLayouts, pNumRows, pRowSizesInBytes, pSrcData);
    HeapFree(GetProcessHeap(), 0, pMem);
    return Result;
}

#define RX_DX12_SET_OBJECT_NAME(OBJ, SETTER, NAME) rx__dx12SetObjectName(OBJ, (rx__dx12NameSetter*) SETTER, NAME)
typedef HRESULT(STDMETHODCALLTYPE rx__dx12NameSetter)(void* object, WCHAR const* name);
void rx__dx12SetObjectName(void* ressourceObject, rx__dx12NameSetter* nameSetter, char const* name) {
	if (name == rx_null || name[0] == '\0') {
		return;
	}
	int wCharsCount = MultiByteToWideChar(CP_UTF8, 0, name, -1, NULL, 0);
#define RX_DX12_OBJECT_NAME_MAX_LENGTH 1024
	RX_ASSERT(wCharsCount < RX_DX12_OBJECT_NAME_MAX_LENGTH && "RX: DX12 object name is too long");
	WCHAR wideName[RX_DX12_OBJECT_NAME_MAX_LENGTH];
	MultiByteToWideChar(CP_UTF8, 0, name, -1, wideName, wCharsCount);
	nameSetter(ressourceObject, (WCHAR const*)wideName);
}

////////////////////////////////////////////////
// Graphics Program
////////////////////////////////////////////////
#if 0
RX_API rx_graphicsProgram rx_dx12MakeGraphicsProgram(RX_APIDEF rx_GraphicsProgramDesc* desc) {
	rx_u32 index = -1;
	rx_u32 generation;
	rx__DX12GraphicsProgram* program;
	for (int i = 0; i < _countof(ctx->dx12->pools.buffers); i++) {
		if (ctx->dx12->pools.gfxLists[i].generation == 0) {
			generation = ctx->dx12->pools.gfxLists[i].generation = 1;
			program = &ctx->dx12->pools.graphicsPrograms[i];
			index = i;
		}
	}
	const void* vsPtr = 0, *fsPtr = 0;
    SIZE_T vsLength = 0, fsLength = 0;
    ID3DBlob* vsBlob = 0, *fsBlob = 0;
	if (desc->vs.byteCode && desc->fs.byteCode) {
        /* create program from byte code */
        vsPtr = desc->vs.byteCode;
        fsPtr = desc->fs.byteCode;
        vsLength = desc->vs.byteCodeSize;
        fsLength = desc->fs.byteCodeSize;
	} else {
        /* compile shader code */
        vsBlob = _sg_d3d11_compile_shader(&desc->vs, "vs_5_0");
        fsBlob = _sg_d3d11_compile_shader(&desc->fs, "ps_5_0");
        if (vsBlob && fsBlob) {
            vsPtr = ID3D10Blob_GetBufferPointer(vsBlob);
            vsLength = ID3D10Blob_GetBufferSize(vsBlob);
            fsPtr = ID3D10Blob_GetBufferPointer(fsBlob);
            fsLength = ID3D10Blob_GetBufferSize(fsBlob);
        }
    }

	
    if (vs_ptr && fs_ptr && (vs_length > 0) && (fs_length > 0)) {
        /* create the D3D vertex- and pixel-shader objects */
        hr = ID3D12Device_CreateVertexShader(_sg.d3d11.dev, vs_ptr, vs_length, NULL, &shd->d3d11_vs);
        SOKOL_ASSERT(SUCCEEDED(hr) && shd->d3d11_vs);
        hr = ID3D12Device_CreatePixelShader(_sg.d3d11.dev, fs_ptr, fs_length, NULL, &shd->d3d11_fs);
        SOKOL_ASSERT(SUCCEEDED(hr) && shd->d3d11_fs);

        /* need to store the vertex shader byte code, this is needed later in sg_create_pipeline */
        shd->d3d11_vs_blob_length = (int)vs_length;
        shd->d3d11_vs_blob = SOKOL_MALLOC((int)vs_length);
        SOKOL_ASSERT(shd->d3d11_vs_blob);
        memcpy(shd->d3d11_vs_blob, vs_ptr, vs_length);

        result = SG_RESOURCESTATE_VALID;
    }
    if (vs_blob) {
        ID3D10Blob_Release(vs_blob); vs_blob = 0;
    }
    if (fs_blob) {
        ID3D10Blob_Release(fs_blob); fs_blob = 0;
    }
}
#endif
////////////////////////////////////////////////
// Graphics Pipeline
////////////////////////////////////////////////

typedef enum rx_wrap {
	rx_wrap_default,   /* value 0 reserved for default-init */
	rx_wrap_repeat,
	rx_wrap_clampToEdge,
	rx_wrap_clampToBorder,
	rx_wrap_mirroredRepeat,
	rx_wrap__count,
	rx_wrap__forceU32 = 0x7FFFFFFF
} rx_wrap;



D3D12_TEXTURE_ADDRESS_MODE rx__dx12AddressMode(rx_wrap m) {
	switch (m) {
	case rx_wrap_repeat:            return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	case rx_wrap_clampToEdge:		return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	case rx_wrap_clampToBorder:		return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	case rx_wrap_mirroredRepeat:	return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	default: RX_UNREACHABLE; return (D3D12_TEXTURE_ADDRESS_MODE)0;
	}
}


/*
	rx_filter
	The filtering mode when sampling a texture image. This is
	used in the sg_image_desc.min_filter and sg_image_desc.mag_filter
	members when creating an image object.
	The default filter mode is rx_filter_NEAREST.
*/
typedef enum rx_filter {
	rx_filter__default, /* value 0 reserved for default-init */
	rx_filter_nearest,
	rx_filter_linear,
	rx_filter_nearestMipmapNearest,
	rx_filter_nearestMipmapLinear,
	rx_filter_linearMipmapNearest,
	rx_filter_linearMipmapLinear,
	rx_filter__count,
	rx_filter__forceU32 = 0x7FFFFFFF
} rx_filter;

typedef enum sg_border_color {
	rx_borderColor__default,    /* value 0 reserved for default-init */
	SG_BORDERCOLOR_transparentBlack,
	SG_BORDERCOLOR_opaqueBlack,
	SG_BORDERCOLOR_opaqueWhite,
	rx_borderColor__count,
	rx_borderColor__forceU32 = 0x7FFFFFFF
} sg_border_color;
/*
	sg_image_desc
	Creation parameters for sg_image objects, used in the
	sg_make_image() call.
	The default configuration is:
	.type:              SG_IMAGETYPE_2D
	.render_target:     false
	.width              0 (must be set to >0)
	.height             0 (must be set to >0)
	.depth/.layers:     1
	.num_mipmaps:       1
	.usage:             SG_USAGE_IMMUTABLE
	.pixel_format:      SG_PIXELFORMAT_RGBA8 for textures, backend-dependent
						for render targets (RGBA8 or BGRA8)
	.sample_count:      1 (only used in render_targets)
	.min_filter:        SG_FILTER_NEAREST
	.mag_filter:        SG_FILTER_NEAREST
	.wrap_u:            SG_WRAP_REPEAT
	.wrap_v:            SG_WRAP_REPEAT
	.wrap_w:            SG_WRAP_REPEAT (only SG_IMAGETYPE_3D)
	.border_color       SG_BORDERCOLOR_OPAQUE_BLACK
	.max_anisotropy     1 (must be 1..16)
	.min_lod            0.0f
	.max_lod            FLT_MAX
	.content            an sg_image_content struct to define the initial content
	.label              0       (optional string label for trace hooks)
	SG_IMAGETYPE_ARRAY and SG_IMAGETYPE_3D are not supported on
	WebGL/GLES2, use sg_query_features().imagetype_array and
	sg_query_features().imagetype_3d at runtime to check
	if array- and 3D-textures are supported.
	Images with usage SG_USAGE_IMMUTABLE must be fully initialized by
	providing a valid .content member which points to
	initialization data.
	ADVANCED TOPIC: Injecting native 3D-API textures:
	The following struct members allow to inject your own GL, Metal
	or D3D11 textures into sokol_gfx:
	.gl_textures[SG_NUM_INFLIGHT_FRAMES]
	.mtl_textures[SG_NUM_INFLIGHT_FRAMES]
	.d3d11_texture
	The same rules apply as for injecting native buffers
	(see sg_buffer_desc documentation for more details).
*/
typedef struct rx_Sampler {
	//sg_image_type type;
	bool renderTarget;
	int width;
	int height;
	int depth;
	int num_mipmaps;
	//sg_usage usage;
	//sg_pixel_format pixelFormat;
	int sampleCount;
	sg_border_color borderColor;
	uint32_t maxAnisotropy;
	rx_filter minFilter;
	rx_filter magFilter;
	rx_wrap wrapU;
	rx_wrap wrapV;
	rx_wrap wrapW;
	float minLod;
	float maxLod;
} rx_Sampler;

D3D12_FILTER sg__dx12Filter(rx_filter minF, rx_filter magF, rx_u32 maxAnisotropy) {
	if (maxAnisotropy > 1) {
		return D3D12_FILTER_ANISOTROPIC;
	} else if (magF == rx_filter_nearest) {
		switch (minF) {
		case rx_filter_nearest:
		case rx_filter_nearestMipmapNearest:
			return D3D12_FILTER_MIN_MAG_MIP_POINT;
		case rx_filter_linear:
		case rx_filter_linearMipmapNearest:
			return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		case rx_filter_nearestMipmapLinear:
			return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		case rx_filter_linearMipmapLinear:
			return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		default:
			RX_UNREACHABLE; break;
		}
	} else if (magF == rx_filter_linear) {
		switch (minF) {
		case rx_filter_nearest:
		case rx_filter_nearestMipmapNearest:
			return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case rx_filter_linear:
		case rx_filter_linearMipmapNearest:
			return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		case rx_filter_nearestMipmapLinear:
			return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
		case rx_filter_linearMipmapLinear:
			return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		default:
			RX_UNREACHABLE; break;
		}
	}
	/* invalid value for mag filter */
	RX_UNREACHABLE;
	return D3D12_FILTER_MIN_MAG_MIP_POINT;
}
void rx__dx12SetStaticSampler(rx_Sampler* img, D3D12_STATIC_SAMPLER_DESC* desc) {
	RX_ASSERT(img);
	RX_ASSERT(desc);
	//D3D11_SAMPLER_DESC d3d11_smp_desc;
	//memset(&d3d11_smp_desc, 0, sizeof(d3d11_smp_desc));
	desc->Filter = sg__dx12Filter(img->minFilter, img->magFilter, img->maxAnisotropy);
	desc->AddressU = rx__dx12AddressMode(img->wrapU);
	desc->AddressV = rx__dx12AddressMode(img->wrapV);
	desc->AddressW = rx__dx12AddressMode(img->wrapW);
	switch (img->borderColor) {
	case SG_BORDERCOLOR_transparentBlack:
		desc->BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		break;
	case SG_BORDERCOLOR_opaqueWhite:
		desc->BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		break;
	default:
		desc->BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		break;
	}
	desc->MaxAnisotropy = img->maxAnisotropy;
	desc->ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	desc->MinLOD = 0.0f;
	desc->MaxLOD = 0.0f;
	desc->MipLODBias = 0.0f;

#if 0
	unsigned int compareFail = (sampler >> abGPU_Sampler_CompareFailShift) & abGPU_Sampler_CompareFailMask;
	D3D12_FILTER_REDUCTION_TYPE reduction = (compareFail != 0u ? D3D12_FILTER_REDUCTION_TYPE_COMPARISON : D3D12_FILTER_REDUCTION_TYPE_STANDARD);
	if (sampler & abGPU_Sampler_FilterAniso) {
		desc->Filter = D3D12_ENCODE_ANISOTROPIC_FILTER(reduction);
		desc->MaxAnisotropy = (1u << (((sampler >> abGPU_Sampler_FilterAnisoPowerM1Shift) & abGPU_Sampler_FilterAnisoPowerM1Mask) + 1u));
	}
	else {
		desc->Filter = D3D12_ENCODE_BASIC_FILTER(
			(sampler & abGPU_Sampler_FilterLinearMin) ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT,
			(sampler & abGPU_Sampler_FilterLinearMag) ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT,
			(sampler & abGPU_Sampler_FilterLinearMip) ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT, reduction);
		desc->MaxAnisotropy = 0u;
	}
	desc->AddressU = ((sampler & abGPU_Sampler_RepeatS) ? D3D12_TEXTURE_ADDRESS_MODE_WRAP : D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	desc->AddressV = ((sampler & abGPU_Sampler_RepeatT) ? D3D12_TEXTURE_ADDRESS_MODE_WRAP : D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	desc->AddressW = ((sampler & abGPU_Sampler_RepeatR) ? D3D12_TEXTURE_ADDRESS_MODE_WRAP : D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	desc->MipLODBias = 0.0f;
	desc->ComparisonFunc = (D3D12_COMPARISON_FUNC)((compareFail ^ abGPU_Sampler_CompareFailMask) + 1u); // Change from fail to pass.
	desc->BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	desc->MinLOD = 0.0f;
	unsigned int maxLOD = (sampler >> abGPU_Sampler_MipCountShift) & abGPU_Sampler_MipCountMask;
	desc->MaxLOD = (maxLOD != abGPU_Sampler_MipCountMask ? (float)maxLOD : FLT_MAX);
#endif
}


rx_u32 rx__mapVertexStep(rx_vertexStep step) {
	rx_u32 mapVertex[rx_vertexStep__count] = {
		[rx_vertexStep__default] = 0,
		[rx_vertexStep_perVertex] = 0,
		[rx_vertexStep_perInstance] = 1,
	};

	return (step < rx_vertexStep__count && step >= rx_vertexStep__default) ? mapVertex[step] : mapVertex[rx_vertexStep__default];
}

DXGI_FORMAT rx_dx12VertexFormat(rx_vertexFormat format) {
    switch (format) {
		case rx_vertexFormat_f16x2:		return DXGI_FORMAT_R16G16_FLOAT;
		case rx_vertexFormat_f16x4: 	return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case rx_vertexFormat_f32x1:     return DXGI_FORMAT_R32_FLOAT;
        case rx_vertexFormat_f32x2:    	return DXGI_FORMAT_R32G32_FLOAT;
        case rx_vertexFormat_f32x3:    	return DXGI_FORMAT_R32G32B32_FLOAT;
        case rx_vertexFormat_f32x4:    	return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case rx_vertexFormat_s8x4:     return DXGI_FORMAT_R8G8B8A8_SINT;
        case rx_vertexFormat_s8x4N:    return DXGI_FORMAT_R8G8B8A8_SNORM;
        case rx_vertexFormat_u8x4:    return DXGI_FORMAT_R8G8B8A8_UINT;
        case rx_vertexFormat_u8x4N:   return DXGI_FORMAT_R8G8B8A8_UNORM;
        case rx_vertexFormat_s16x2:    return DXGI_FORMAT_R16G16_SINT;
        case rx_vertexFormat_s16x2N:   return DXGI_FORMAT_R16G16_SNORM;
        case rx_vertexFormat_u16x2N:  return DXGI_FORMAT_R16G16_UNORM;
        case rx_vertexFormat_s16x4:    return DXGI_FORMAT_R16G16B16A16_SINT;
        case rx_vertexFormat_s16x4N:   return DXGI_FORMAT_R16G16B16A16_SNORM;
        case rx_vertexFormat_u16x4N:  return DXGI_FORMAT_R16G16B16A16_UNORM;
        case rx_vertexFormat_u10x3U2x1N: return DXGI_FORMAT_R10G10B10A2_UNORM;
        default: RX_UNREACHABLE; return (DXGI_FORMAT) 0;
    }
}

D3D12_SHADER_VISIBILITY rx__dx12StageVisibility(rx_stageVisibility vis) {
	switch (vis) {
		case rx_stageVisibility_vertex: 	return D3D12_SHADER_VISIBILITY_VERTEX;
		case rx_stageVisibility_fragment: 	return D3D12_SHADER_VISIBILITY_PIXEL;
		case rx_stageVisibility_all:
		case rx_stageVisibility__default:
			return D3D12_SHADER_VISIBILITY_ALL;
		default:
		RX_UNREACHABLE;
		return D3D12_SHADER_VISIBILITY_ALL;
	}
}

rx_u8 rx__dx12ColorWriteMask(rx_colorMask m) {
    rx_u8 res = 0;
    if (m & rx_colorMask_r) {
        res |= D3D12_COLOR_WRITE_ENABLE_RED;
    }
    if (m & rx_colorMask_g) {
        res |= D3D12_COLOR_WRITE_ENABLE_GREEN;
    }
    if (m & rx_colorMask_b) {
        res |= D3D12_COLOR_WRITE_ENABLE_BLUE;
    }
    if (m & rx_colorMask_a) {
        res |= D3D12_COLOR_WRITE_ENABLE_ALPHA;
    }
    return res;
}

enum {
    rx_inputSampler_dynamicOnly = UINT8_MAX
};

rx_b32 rx_dx12MakeInputConfig(rx_Context* ctx, rx_InputLayoutDesc* desc, ID3D12RootSignature** dx12RootSignature,
		D3D12_INPUT_ELEMENT_DESC *vertexElements, int* vertexCount, const char* label) {

	*vertexCount = 0;
	// D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = { 0 };
	D3D12_ROOT_PARAMETER rootParameters[RX_MAX_INPUTS + 2];
	D3D12_ROOT_PARAMETER* rootParameter;
	D3D12_DESCRIPTOR_RANGE descriptorRanges[_countof(rootParameters)];
	D3D12_STATIC_SAMPLER_DESC staticSamplerDescs[RX_MAX_SAMPLER];
	unsigned int rootParameterCount = 0u, staticSamplerCount = 0u;

	if ((desc->uniform32BitCount > 0 || !desc->uniformUseBuffer)) {
		D3D12_SHADER_VISIBILITY uniformVisibility = rx__dx12StageVisibility(desc->uniformStages);

		if (desc->uniform32BitCount != 0) {
			rootParameter = &rootParameters[rootParameterCount]; // Always parameter 0.
			rootParameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			rootParameter->Constants.ShaderRegister = 0;
			rootParameter->Constants.RegisterSpace = 0;
			rootParameter->Constants.Num32BitValues = desc->uniform32BitCount;
			rootParameter->ShaderVisibility = uniformVisibility;
			rootParameterCount++;
		}
		if (desc->uniformUseBuffer) {
			rootParameter = &rootParameters[rootParameterCount]; // Parameter 0 or 1.
			rootParameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			rootParameter->Descriptor.ShaderRegister = 1;
			rootParameter->Descriptor.RegisterSpace = 1;
			rootParameter->ShaderVisibility = uniformVisibility;
			rootParameterCount++;
		}
	}

	// should be inside a pool
	uint8_t bRootParameters[RX_MAX_INPUTS];

	for (unsigned int inputIndex = 0; inputIndex < RX_MAX_INPUTS; inputIndex++) {
		rx_Input * input = &desc->inputs[inputIndex];
		rootParameter = &rootParameters[rootParameterCount];


		if (input->type == rx_inputType__invalid) {
			bRootParameters[inputIndex] = UINT8_MAX; // Input not used by any shader stage (a hole in the input list).
			continue;
		}

		rootParameter->ShaderVisibility = rx__dx12StageVisibility(input->stageVisibility);

		D3D12_DESCRIPTOR_RANGE * descriptorRange = &descriptorRanges[rootParameterCount];
		descriptorRange->OffsetInDescriptorsFromTableStart = 0;
		switch (input->type) {
		case rx_inputType_constantBuffer:
			rootParameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			rootParameter->Descriptor.ShaderRegister = input->parameters.constantBuffer.constantBufferIndex;
			rootParameter->Descriptor.RegisterSpace = 0;
			break;
		case rx_inputType_constantBufferHandle:
			rootParameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParameter->DescriptorTable.NumDescriptorRanges = 1u;
			rootParameter->DescriptorTable.pDescriptorRanges = descriptorRange;
			descriptorRange->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			descriptorRange->NumDescriptors = input->parameters.constantBufferHandle.count;
			descriptorRange->BaseShaderRegister = input->parameters.constantBufferHandle.constantBufferFirstIndex;
			descriptorRange->RegisterSpace = 0;
			break;
		case rx_inputType_structureBufferHandle:
			rootParameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParameter->DescriptorTable.NumDescriptorRanges = 1u;
			rootParameter->DescriptorTable.pDescriptorRanges = descriptorRange;
			descriptorRange->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			descriptorRange->NumDescriptors = input->parameters.structureBufferHandle.count;
			descriptorRange->BaseShaderRegister = input->parameters.structureBufferHandle.structureBufferFirstIndex;
			descriptorRange->RegisterSpace = 1;
			break;
		case rx_inputType_editBufferHandle:
			rootParameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParameter->DescriptorTable.NumDescriptorRanges = 1u;
			rootParameter->DescriptorTable.pDescriptorRanges = descriptorRange;
			descriptorRange->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			descriptorRange->NumDescriptors = input->parameters.editBufferHandle.count;
			descriptorRange->BaseShaderRegister = input->parameters.editBufferHandle.editBufferFirstIndex;
			descriptorRange->RegisterSpace = 1;
			break;
		case rx_inputType_textureHandle:
			rootParameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParameter->DescriptorTable.NumDescriptorRanges = 1;
			rootParameter->DescriptorTable.pDescriptorRanges = descriptorRange;
			descriptorRange->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			descriptorRange->NumDescriptors = input->parameters.imageHandle.count;
			descriptorRange->BaseShaderRegister = input->parameters.imageHandle.textureFirstIndex;
			descriptorRange->RegisterSpace = 0;
			break;
		case rx_inputType_editImageHandle:
			rootParameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParameter->DescriptorTable.NumDescriptorRanges = 1;
			rootParameter->DescriptorTable.pDescriptorRanges = descriptorRange;
			descriptorRange->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			descriptorRange->NumDescriptors = input->parameters.editImageHandle.count;
			descriptorRange->BaseShaderRegister = input->parameters.editImageHandle.editImageFirstIndex;
			descriptorRange->RegisterSpace = 0;
			break;
		case rx_inputType_samplerHandle:
			RX_ASSERT(!"Not implemented yet!");
#if 0
			if (staticSamplers == rx_null) {
				input->parameters.samplerHandle.staticSamplerIndex = rx_inputSampler_dynamicOnly;
			}
			if (input->parameters.samplerHandle.staticSamplerIndex != rx_inputSampler_dynamicOnly) {
				if (staticSamplerCount + input->parameters.samplerHandle.count > abArrayLength(staticSamplerDescs)) {
					input->parameters.samplerHandle.staticSamplerIndex = abGPU_Input_SamplerDynamicOnly;
					return abFalse;
				}
				for (unsigned int staticSamplerIndex = 0u; staticSamplerIndex < input->parameters.samplerHandle.count; ++staticSamplerIndex) {
					D3D12_STATIC_SAMPLER_DESC * staticSamplerDesc = &staticSamplerDescs[staticSamplerCount++];
					abGPUi_D3D_Sampler_WriteStaticSamplerDesc(
							staticSamplers[input->parameters.samplerHandle.staticSamplerIndex + staticSamplerIndex], staticSamplerDesc);
					staticSamplerDesc->ShaderRegister = input->parameters.samplerHandle.samplerFirstIndex + staticSamplerIndex;
					staticSamplerDesc->RegisterSpace = 0u;
					staticSamplerDesc->ShaderVisibility = rootParameter->ShaderVisibility;
				}
				bRootParameters[inputIndex] = UINT8_MAX;
				//goto nextInput;
				continue;
			}
			rootParameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParameter->DescriptorTable.NumDescriptorRanges = 1u;
			rootParameter->DescriptorTable.pDescriptorRanges = descriptorRange;
			descriptorRange->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
			descriptorRange->NumDescriptors = input->parameters.samplerHandle.count;
			descriptorRange->BaseShaderRegister = input->parameters.samplerHandle.samplerFirstIndex;
#endif
			break;
		default:
			return rx_false;
		}
		bRootParameters[inputIndex] = rootParameterCount;
		rootParameterCount++;
		//nextInput: {}
	}

	// rx_u32 attributeTypesUsed = 0;
	
	for (rx_u32 attributeIndex = 0; attributeIndex < RX_MAX_INPUT_ATTRIBUTES; attributeIndex++) {
		rx_VertexAttributeDesc const* attribute = &desc->vertexAttributes[attributeIndex];
        if (attribute->format == rx_VERTEXFORMAT__invalid) {
            break;
        }
		*vertexCount += 1;
		rx_u32 bufferIndex = attribute->bufferIndex;
#if 0
		if (attribute->type >= abGPU_VertexData_MaxAttributes || bufferIndex >= config->vertexBufferCount ||
				(attributeTypesUsed & (1 << attribute->type))) {
			return rx_false;
		}
		attributeTypesUsed |= 1 << attribute->type; // Don't allow more than 1 attribute of the same type.
#endif
		D3D12_INPUT_ELEMENT_DESC* elementDesc = &vertexElements[attributeIndex];
		elementDesc->SemanticName = attribute->semanticName;
		elementDesc->SemanticIndex = attribute->semanticIndex;
		elementDesc->Format = rx_dx12VertexFormat(attribute->format);
		rx_BufferLayoutDesc * buffer = &desc->vertexBuffers[bufferIndex];
		elementDesc->InputSlot = bufferIndex;
		elementDesc->AlignedByteOffset = attribute->offset;
		rx_u32 stepFunc = rx__mapVertexStep(buffer->stepFunc);
		elementDesc->InputSlotClass = (stepFunc != 0 ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA);
		elementDesc->InstanceDataStepRate = stepFunc;
		rx_u32 minStride = attribute->offset + rx__vertexFormatSize(attribute->format);
#if 0
		if (minStride > UINT8_MAX) {
			return abFalse;
		}
#endif
		buffer->stride = RX_MAX(minStride, buffer->stride);
	}

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {
		.NumParameters = rootParameterCount,
		.pParameters = rootParameters,
		.NumStaticSamplers = staticSamplerCount,
		.pStaticSamplers = staticSamplerDescs,
		.Flags = ((*vertexCount) != 0 ? D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT : D3D12_ROOT_SIGNATURE_FLAG_NONE)
	};

	ID3D10Blob* blob;
	if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, rx_null))) {
		return rx_false;
	}
	rx_b32 succeeded = (SUCCEEDED(ID3D12Device_CreateRootSignature(ctx->dx12->device, 0, ID3D10Blob_GetBufferPointer(blob), ID3D10Blob_GetBufferSize(blob), &IID_ID3D12RootSignature, dx12RootSignature)) ? rx_true : rx_false);
	RX_DX12_SET_OBJECT_NAME(*dx12RootSignature, (*dx12RootSignature)->lpVtbl->SetName, label);
	ID3D10Blob_Release(blob);

	return succeeded;
}

DXGI_FORMAT rx__dx12PixelFormat(rx_pixelFormat format) {
    switch (format) {
        case rx_pixelFormat_R8:             return DXGI_FORMAT_R8_UNORM;
        case rx_pixelFormat_R8SN:           return DXGI_FORMAT_R8_SNORM;
        case rx_pixelFormat_R8UI:           return DXGI_FORMAT_R8_UINT;
        case rx_pixelFormat_R8SI:           return DXGI_FORMAT_R8_SINT;
        case rx_pixelFormat_R16:            return DXGI_FORMAT_R16_UNORM;
        case rx_pixelFormat_R16SN:          return DXGI_FORMAT_R16_SNORM;
        case rx_pixelFormat_R16UI:          return DXGI_FORMAT_R16_UINT;
        case rx_pixelFormat_R16SI:          return DXGI_FORMAT_R16_SINT;
        case rx_pixelFormat_R16F:           return DXGI_FORMAT_R16_FLOAT;
        case rx_pixelFormat_RG8:            return DXGI_FORMAT_R8G8_UNORM;
        case rx_pixelFormat_RG8SN:          return DXGI_FORMAT_R8G8_SNORM;
        case rx_pixelFormat_RG8UI:          return DXGI_FORMAT_R8G8_UINT;
        case rx_pixelFormat_RG8SI:          return DXGI_FORMAT_R8G8_SINT;
        case rx_pixelFormat_R32UI:          return DXGI_FORMAT_R32_UINT;
        case rx_pixelFormat_R32SI:          return DXGI_FORMAT_R32_SINT;
        case rx_pixelFormat_R32F:           return DXGI_FORMAT_R32_FLOAT;
        case rx_pixelFormat_RG16:           return DXGI_FORMAT_R16G16_UNORM;
        case rx_pixelFormat_RG16SN:         return DXGI_FORMAT_R16G16_SNORM;
        case rx_pixelFormat_RG16UI:         return DXGI_FORMAT_R16G16_UINT;
        case rx_pixelFormat_RG16SI:         return DXGI_FORMAT_R16G16_SINT;
        case rx_pixelFormat_RG16F:          return DXGI_FORMAT_R16G16_FLOAT;
        case rx_pixelFormat_RGBA8:          return DXGI_FORMAT_R8G8B8A8_UNORM;
        case rx_pixelFormat_RGBA8SN:        return DXGI_FORMAT_R8G8B8A8_SNORM;
        case rx_pixelFormat_RGBA8UI:        return DXGI_FORMAT_R8G8B8A8_UINT;
        case rx_pixelFormat_RGBA8SI:        return DXGI_FORMAT_R8G8B8A8_SINT;
        case rx_pixelFormat_BGRA8:          return DXGI_FORMAT_B8G8R8A8_UNORM;
        case rx_pixelFormat_RGB10A2:        return DXGI_FORMAT_R10G10B10A2_UNORM;
        case rx_pixelFormat_RG11B10F:       return DXGI_FORMAT_R11G11B10_FLOAT;
        case rx_pixelFormat_RG32UI:         return DXGI_FORMAT_R32G32_UINT;
        case rx_pixelFormat_RG32SI:         return DXGI_FORMAT_R32G32_SINT;
        case rx_pixelFormat_RG32F:          return DXGI_FORMAT_R32G32_FLOAT;
        case rx_pixelFormat_RGBA16:         return DXGI_FORMAT_R16G16B16A16_UNORM;
        case rx_pixelFormat_RGBA16SN:       return DXGI_FORMAT_R16G16B16A16_SNORM;
        case rx_pixelFormat_RGBA16UI:       return DXGI_FORMAT_R16G16B16A16_UINT;
        case rx_pixelFormat_RGBA16SI:       return DXGI_FORMAT_R16G16B16A16_SINT;
        case rx_pixelFormat_RGBA16F:        return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case rx_pixelFormat_RGBA32UI:       return DXGI_FORMAT_R32G32B32A32_UINT;
        case rx_pixelFormat_RGBA32SI:       return DXGI_FORMAT_R32G32B32A32_SINT;
        case rx_pixelFormat_RGBA32F:        return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case rx_pixelFormat_DEPTH:          return DXGI_FORMAT_D32_FLOAT;
        case rx_pixelFormat_DEPTH_STENCIL:  return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case rx_pixelFormat_BC1_RGBA:       return DXGI_FORMAT_BC1_UNORM;
        case rx_pixelFormat_BC2_RGBA:       return DXGI_FORMAT_BC2_UNORM;
        case rx_pixelFormat_BC3_RGBA:       return DXGI_FORMAT_BC3_UNORM;
        case rx_pixelFormat_BC4_R:          return DXGI_FORMAT_BC4_UNORM;
        case rx_pixelFormat_BC4_RSN:        return DXGI_FORMAT_BC4_SNORM;
        case rx_pixelFormat_BC5_RG:         return DXGI_FORMAT_BC5_UNORM;
        case rx_pixelFormat_BC5_RGSN:       return DXGI_FORMAT_BC5_SNORM;
        case rx_pixelFormat_BC6H_RGBF:      return DXGI_FORMAT_BC6H_SF16;
        case rx_pixelFormat_BC6H_RGBUF:     return DXGI_FORMAT_BC6H_UF16;
        case rx_pixelFormat_BC7_RGBA:       return DXGI_FORMAT_BC7_UNORM;
		case rx_pixelFormat_d32:			return DXGI_FORMAT_R32_TYPELESS;
		case rx_pixelFormat_d24S8:			return DXGI_FORMAT_R24G8_TYPELESS;
        default:                            return DXGI_FORMAT_UNKNOWN;
    };
}

RX_FORCE_INLINE D3D12_LOGIC_OP rx_dx12BitOperation(rx_bitOperation operation) {
	static const D3D12_LOGIC_OP mapBitOperation[rx_bitOperation__count] = {
		[rx_bitOperation_copy] = D3D12_LOGIC_OP_COPY, // Not really an operation though, means there's no bit operation.
		[rx_bitOperation_copyInv] = D3D12_LOGIC_OP_COPY_INVERTED,
		[rx_bitOperation_setZero] = D3D12_LOGIC_OP_CLEAR,
		[rx_bitOperation_setOne] = D3D12_LOGIC_OP_SET,
		[rx_bitOperation_reverse] = D3D12_LOGIC_OP_INVERT,
		[rx_bitOperation_and] = D3D12_LOGIC_OP_AND,
		[rx_bitOperation_notAnd] = D3D12_LOGIC_OP_NAND,
		[rx_bitOperation_or] = D3D12_LOGIC_OP_OR,
		[rx_bitOperation_notOr] = D3D12_LOGIC_OP_NOR,
		[rx_bitOperation_xor] = D3D12_LOGIC_OP_XOR,
		[rx_bitOperation_equal] = D3D12_LOGIC_OP_EQUIV,
		[rx_bitOperation_andRev] = D3D12_LOGIC_OP_AND_REVERSE,
		[rx_bitOperation_andInv] = D3D12_LOGIC_OP_AND_INVERTED,
		[rx_bitOperation_orRev] = D3D12_LOGIC_OP_OR_REVERSE,
		[rx_bitOperation_orInv] = D3D12_LOGIC_OP_OR_INVERTED
	};
	return ((rx_u32) operation < rx_bitOperation__count ? mapBitOperation[operation] : D3D12_LOGIC_OP_COPY);
}

DXGI_FORMAT rx__dx12DepthImageFormat(rx_pixelFormat format) {
	if (format == rx_pixelFormat__default) {
		return DXGI_FORMAT_D32_FLOAT;
	}
	switch (format) {
	case rx_pixelFormat_d32:	return DXGI_FORMAT_R32_TYPELESS;
	case rx_pixelFormat_d24S8:	return DXGI_FORMAT_R24G8_TYPELESS;
	default: return rx__dx12PixelFormat(format);
	}
}

D3D12_COMPARISON_FUNC rx__dx12CompareFunc(rx_compareFunc func) {
	static const D3D12_COMPARISON_FUNC mapCompare[rx_compareFunc__count] = {
		[rx_compareFunc__default] = D3D12_COMPARISON_FUNC_ALWAYS,
		[rx_compareFunc_never] = D3D12_COMPARISON_FUNC_NEVER,
		[rx_compareFunc_less] = D3D12_COMPARISON_FUNC_LESS,
		[rx_compareFunc_equal] = D3D12_COMPARISON_FUNC_EQUAL,
		[rx_compareFunc_lessEqual] = D3D12_COMPARISON_FUNC_LESS_EQUAL,
		[rx_compareFunc_greater] = D3D12_COMPARISON_FUNC_GREATER,
		[rx_compareFunc_notEqual] = D3D12_COMPARISON_FUNC_NOT_EQUAL,
		[rx_compareFunc_greaterEqual] = D3D12_COMPARISON_FUNC_GREATER_EQUAL,
		[rx_compareFunc_always] = D3D12_COMPARISON_FUNC_ALWAYS,
	};
	return (rx_u32) func > rx_compareFunc__count ? (D3D12_COMPARISON_FUNC) 0 : mapCompare[func];
}

D3D12_STENCIL_OP rx__dx12StencilOp(rx_stencilOp op) {
	static const D3D12_STENCIL_OP mapStencilOp[rx_stencilOp__count] = {
		[rx_stencilOp__default] = D3D12_STENCIL_OP_KEEP,
		[rx_stencilOp_keep] = D3D12_STENCIL_OP_KEEP,
		[rx_stencilOp_zero] = D3D12_STENCIL_OP_ZERO,
		[rx_stencilOp_replace] = D3D12_STENCIL_OP_REPLACE,
		[rx_stencilOp_incrementClamp] = D3D12_STENCIL_OP_INCR_SAT,
		[rx_stencilOp_decrementClamp] = D3D12_STENCIL_OP_DECR_SAT,
		[rx_stencilOp_invert] = D3D12_STENCIL_OP_INVERT,
		[rx_stencilOp_incrementWrap] = D3D12_STENCIL_OP_INCR,
		[rx_stencilOp_decrementWrap] = D3D12_STENCIL_OP_DECR,
	};
	if ((rx_u32)op < rx_stencilOp__count) {
		return mapStencilOp[(rx_u32)op];
	}
	RX_ASSERT((rx_u32) op > rx_stencilOp__count);
	return D3D12_STENCIL_OP_KEEP;
}

rx_b32 rx__dx12InitCompiler(rx_Context* ctx) {
	/* on UWP, don't do anything (not tested) */
#if (defined(WINAPI_FAMILY_PARTITION) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP))
	return true;
#else
	/* load DLL on demand */
	if ((ctx->dx12->d3dcompilerDll == rx_null) && !ctx->dx12->d3dcompilerDllLoadFailed) {
		ctx->dx12->d3dcompilerDll = LoadLibraryA(D3DCOMPILER_DLL_A);
		if (ctx->dx12->d3dcompilerDll == rx_null) {
			/* don't attempt to load missing DLL in the future */
			RX_ASSERT(!"failed to load d3dcompiler_47.dll!");
			ctx->dx12->d3dcompilerDllLoadFailed = rx_true;
			return rx_false;
		}
		/* look up function pointers */
		ctx->dx12->hlslCompileFunc = (pD3DCompile)GetProcAddress(ctx->dx12->d3dcompilerDll, "D3DCompile");
		RX_ASSERT(ctx->dx12->hlslCompileFunc && "Failed to load compiler");
	}
	return ctx->dx12->d3dcompilerDll != rx_null;
#endif
}

ID3DBlob* rx__dx12CompileShader(rx_Context* ctx, const char* shader, const char* entryPoint, const char* shaderType) {
	rx__dx12InitCompiler(ctx);
	ID3DBlob* errors = rx_null;
	ID3DBlob* shaderBlob = rx_null;
	ctx->dx12->hlslCompileFunc(shader, strlen(shader), rx_null, rx_null, rx_null, entryPoint, shaderType, 0, D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_ENABLE_STRICTNESS, &shaderBlob, &errors);
	if (errors) {
		LPCSTR errorsStr = (LPCSTR)ID3D10Blob_GetBufferPointer(errors);
		OutputDebugStringA(errorsStr);
		ID3D10Blob_Release(errors); errors = rx_null;
		return rx_null;
	}
	return shaderBlob;
}

RX_API rx_graphicsPipeline rx_dx12MakeGraphicsPipeline(rx_Context* ctx, rx_GraphicsPipelineDesc* desc) {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC dx12Desc = { 0 };
	int index = -1;
	rx__Dx12Pipeline* pipeline = rx_null;

	
	for (int i = 0; i < _countof(ctx->dx12->pools.buffers); i++) {
		if (ctx->dx12->pools.pipelines[i].generation == 0) {
			ctx->dx12->pools.pipelines[i].generation = 1;
			pipeline = &ctx->dx12->pools.pipelines[i];
			index = i;
			break;
		}
	}

	RX_ASSERT(index > -1);
	rx_id pipelineId = { { .index = index, .generation = pipeline->generation } };
	// make root signature
	int vertexElementsCount = 0;
	D3D12_INPUT_ELEMENT_DESC dx12VertexElements[RX_MAX_VERTEX_ATTRIBUTES];
	ID3D12RootSignature* dx12RootSignature;
	rx_dx12MakeInputConfig(ctx, &desc->input, &dx12RootSignature, dx12VertexElements, &vertexElementsCount, desc->label);
	pipeline->dx12RootSignature = dx12RootSignature;
	dx12Desc.pRootSignature = dx12RootSignature;

	{
		// this is only temporary!
		dx12Desc.BlendState = (D3D12_BLEND_DESC){
			.AlphaToCoverageEnable = rx_false,
			.RenderTarget[0].BlendEnable = rx_true,
			.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA,
			.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA,
			.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD,
			.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA,
			.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO,
			.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD,
			.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL,
		};
		dx12Desc.NumRenderTargets = 1;
		dx12Desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	// Handle shader

	if (desc->program.vs.byteCode && desc->program.fs.byteCode) {
		// ID3D10Blob_GetBufferPointer(shaderCode->i_blob);
		// dx12Desc.VS.BytecodeLength = ID3D10Blob_GetBufferSize(shaderCode->i_blob);
		dx12Desc.VS.pShaderBytecode = desc->program.vs.byteCode;
		dx12Desc.VS.BytecodeLength = desc->program.vs.byteCodeSize;
		dx12Desc.PS.pShaderBytecode = desc->program.fs.byteCode;
		dx12Desc.PS.BytecodeLength = desc->program.fs.byteCodeSize;
	} else if (desc->program.vs.source && desc->program.fs.source) {
		ID3DBlob* errors = rx_null;
		ID3DBlob* vertexShader = rx__dx12CompileShader(ctx, desc->program.vs.source, "main", "vs_5_1");
		ID3DBlob* fragmentShader = rx__dx12CompileShader(ctx, desc->program.fs.source, "main", "ps_5_1");

		dx12Desc.VS = (D3D12_SHADER_BYTECODE) {
			.pShaderBytecode = ID3D10Blob_GetBufferPointer(vertexShader),
			.BytecodeLength = ID3D10Blob_GetBufferSize(vertexShader),
		};

		dx12Desc.PS = (D3D12_SHADER_BYTECODE) {
			.pShaderBytecode = ID3D10Blob_GetBufferPointer(fragmentShader),
			.BytecodeLength = ID3D10Blob_GetBufferSize(fragmentShader),
		};
	} else {
		RX_ASSERT(0 && "Shader sources is missing");
	}

	D3D12_INPUT_ELEMENT_DESC localLayout[RX_MAX_VERTEX_ATTRIBUTES];

// TODO Render targets
#if 0
	for (rx_u32 rtIndex = 0u; rtIndex < RX_MAX_RENDERTARGETS; rtIndex++) {
		rx_pixelFormat format = desc->renderTargets[rtIndex].format;
		if (format == rx_pixelFormat_none) {
			break;
		}
		DXGI_FORMAT dxgiFormat = rx__dx12PixelFormat(format);
		RX_ASSERT(dxgiFormat != DXGI_FORMAT_UNKNOWN && "Unknown rendet target pixel format");
		dx12Desc.RTVFormats[dx12Desc.NumRenderTargets] = dxgiFormat;
		dx12Desc.NumRenderTargets++;
	}
#endif
// TODO: Blendstate
#if 0 
	if (dx12Desc.NumRenderTargets > 0) {
		rx_RenderTargetDesc const * renderTargetDesc = &desc->renderTargets[0];
		D3D12_RENDER_TARGET_BLEND_DESC * rtBlend;
		rx_bitOperation bitOperation = renderTargetDesc->bitOperation;
		if (bitOperation != rx_bitOperation_copy) {
			// Bit operation must be the same for all render targets, and it can't be used together with blending.
			rtBlend = &dx12Desc.BlendState.RenderTarget[0];
			rtBlend->LogicOpEnable = TRUE;
			rtBlend->LogicOp = rx_dx12BitOperation(bitOperation);
			rtBlend->RenderTargetWriteMask = rx__dx12ColorWriteMask(renderTargetDesc->colorWriteMask);
		} else {
#if 0
			if (dx12Desc.NumRenderTargets != 1 && (options & abGPU_DrawConfig_Options_BlendAndMaskSeparate)) {
				dx12Desc.BlendState.IndependentBlendEnable = rx_true;
			}
#endif
			for (unsigned int rtIndex = 0u; rtIndex < dx12Desc.NumRenderTargets; ++rtIndex) {
				rx_RenderTargetDesc const* renderTargetDesc = &desc->renderTargets[rtIndex];
				D3D12_RENDER_TARGET_BLEND_DESC* rtBlend = &dx12Desc.BlendState.RenderTarget[rtIndex];
				if (renderTargetDesc->blend) {
					rtBlend->BlendEnable = TRUE;
					RX_ASSERT(!"Not implemented yet!");
#if 0
					rtBlend->SrcBlend = abGPUi_D3D_DrawConfig_BlendFactorToD3D(renderTargetDesc->srcFactorRgb);
					rtBlend->SrcBlendAlpha = abGPUi_D3D_DrawConfig_BlendFactorToD3D(renderTargetDesc->srcFactorAlpha);
					rtBlend->DestBlend = abGPUi_D3D_DrawConfig_BlendFactorToD3D(renderTargetDesc->dstFactorRgb);
					rtBlend->DestBlendAlpha = abGPUi_D3D_DrawConfig_BlendFactorToD3D(renderTargetDesc->dstFactorAlpha);
					rtBlend->BlendOp = abGPUi_D3D_DrawConfig_BlendOperationToD3D(renderTargetDesc->opRgb);
					rtBlend->BlendOpAlpha = abGPUi_D3D_DrawConfig_BlendOperationToD3D(renderTargetDesc->opAlpha);
#endif
				} 
				rtBlend->RenderTargetWriteMask = rx__dx12ColorWriteMask(renderTargetDesc->colorWriteMask);
				if (!dx12Desc.BlendState.IndependentBlendEnable) {
					break;
				}
			}
		}
	}
#endif
	dx12Desc.SampleMask = UINT_MAX;
	
	// Create the rasterizer state
	{
		dx12Desc.RasterizerState.FillMode = ((desc->rasterizer.fillMode == rx_fillMode_wireFrame) ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID);

		switch (desc->rasterizer.cullMode) {
			case rx_cullMode_none: dx12Desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; break;
			case rx_cullMode_back: dx12Desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK; break;
			case rx_cullMode_front:
			default:
			dx12Desc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
		}
		dx12Desc.RasterizerState.FrontCounterClockwise = (desc->rasterizer.faceWinding == rx_faceWinding_counterClockWise ? FALSE : TRUE);
		dx12Desc.RasterizerState.DepthBias = (INT) desc->rasterizer.depthBias;
		dx12Desc.RasterizerState.DepthBiasClamp = desc->rasterizer.depthBiasClamp;
		dx12Desc.RasterizerState.SlopeScaledDepthBias = desc->rasterizer.depthBiasSlopeScale;
		dx12Desc.RasterizerState.DepthClipEnable = TRUE; // ((options & abGPU_DrawConfig_Options_DepthClip) ? TRUE : FALSE);
		// dx12Desc.RasterizerState.ScissorEnable = TRUE;
		dx12Desc.RasterizerState.MultisampleEnable = desc->rasterizer.sampleCount > 1;
		dx12Desc.RasterizerState.AntialiasedLineEnable = FALSE;
		
		// overwrite all of this for now
		dx12Desc.RasterizerState = (D3D12_RASTERIZER_DESC) {
			.FillMode = D3D12_FILL_MODE_SOLID,
			.CullMode = D3D12_CULL_MODE_NONE,
			.FrontCounterClockwise = FALSE,
			.DepthBias = D3D12_DEFAULT_DEPTH_BIAS,
			.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
			.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
			.DepthClipEnable = true,
			.MultisampleEnable = FALSE,
			.AntialiasedLineEnable = FALSE,
			.ForcedSampleCount = 0,
			.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF,
		};
	}

	if (ctx->debug) {
		//dx12Desc.Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;
	}

	// Create depth-stencil State
	{
		dx12Desc.DepthStencilState.DepthEnable = TRUE;
		dx12Desc.DepthStencilState.DepthWriteMask = desc->depthStencil.depthWriteEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		dx12Desc.DepthStencilState.DepthFunc = rx__dx12CompareFunc(desc->depthStencil.depthCompareFunc);
		dx12Desc.DepthStencilState.StencilEnable = desc->depthStencil.stencilEnabled;
		dx12Desc.DepthStencilState.StencilReadMask = desc->depthStencil.stencilReadMask;
		dx12Desc.DepthStencilState.StencilWriteMask = desc->depthStencil.stencilWriteMask;

		const rx_stencilState* sf = &desc->depthStencil.stencilFront;
		dx12Desc.DepthStencilState.FrontFace.StencilFailOp = rx__dx12StencilOp(sf->failOp);
		dx12Desc.DepthStencilState.FrontFace.StencilDepthFailOp = rx__dx12StencilOp(sf->depthFailOp);
		dx12Desc.DepthStencilState.FrontFace.StencilPassOp = rx__dx12StencilOp(sf->passOp);
		dx12Desc.DepthStencilState.FrontFace.StencilFunc = rx__dx12CompareFunc(sf->compareFunc);
		const rx_stencilState* sb = &desc->depthStencil.stencilBack;
		dx12Desc.DepthStencilState.BackFace.StencilFailOp = rx__dx12StencilOp(sb->failOp);
		dx12Desc.DepthStencilState.BackFace.StencilDepthFailOp = rx__dx12StencilOp(sb->depthFailOp);
		dx12Desc.DepthStencilState.BackFace.StencilPassOp = rx__dx12StencilOp(sb->passOp);
		dx12Desc.DepthStencilState.BackFace.StencilFunc = rx__dx12CompareFunc(sb->compareFunc);

		// Overwrite depth stencil for now
		dx12Desc.DepthStencilState = (D3D12_DEPTH_STENCIL_DESC) {
        	.DepthEnable = false,
        	.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
        	.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS,
        	.StencilEnable = false,
        	.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS,
		};
        dx12Desc.DepthStencilState.FrontFace.StencilFailOp = dx12Desc.DepthStencilState.FrontFace.StencilDepthFailOp = dx12Desc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        dx12Desc.DepthStencilState.BackFace = dx12Desc.DepthStencilState.FrontFace;
	}

	// dx12Desc.DSVFormat = rx__dx12DepthImageFormat(desc->depthFormat);// abGPUi_D3D_Image_FormatToDepthStencil(config->depthFormat);

	dx12Desc.InputLayout.pInputElementDescs = dx12VertexElements;
	dx12Desc.InputLayout.NumElements = vertexElementsCount;
	dx12Desc.IBStripCutValue =  ((desc->indexType == rx_indexType_u16) ? D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF : D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF);
	switch (desc->primitiveType) {
	case rx_primitiveType_points: dx12Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT; break;
	case rx_primitiveType_lines: dx12Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE; break;
	case rx_primitiveType_triangles:
	default: dx12Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	}
	dx12Desc.SampleDesc.Count = 1;
	OutputDebugStringA("Create pipeline\n");
	HRESULT result = ID3D12Device2_CreateGraphicsPipelineState(ctx->dx12->device, &dx12Desc, &IID_ID3D12PipelineState, &pipeline->dx12Pipeline);

	OutputDebugStringA("My output string.\n");
	if (FAILED(result)) {
		RX_ASSERT(!"Failed to create DX12 pipeline state.");
	}
	RX_DX12_SET_OBJECT_NAME(pipeline->dx12Pipeline, pipeline->dx12Pipeline->lpVtbl->SetName, desc->label);

    return (rx_graphicsPipeline) { .id = pipelineId.id };
}


////////////////////////////////////////////////
// CommandList
////////////////////////////////////////////////

rx_b32 rx__dx12GfxCommandListInit(ID3D12Device2* device, rx__DX12GfxCmdList* list, char const* name) {
	if (FAILED(ID3D12Device_CreateCommandAllocator(device, D3D12_COMMAND_LIST_TYPE_DIRECT, &IID_ID3D12CommandAllocator, &list->dx12Allocator))) {
		return rx_false;
	}

	RX_DX12_SET_OBJECT_NAME(list->dx12Allocator, list->dx12Allocator->lpVtbl->SetName, name);
	if (FAILED(ID3D12Device_CreateCommandList(device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, list->dx12Allocator, rx_null,
		&IID_ID3D12GraphicsCommandList, &list->dx12CmdList))) {
		ID3D12CommandAllocator_Release(list->dx12Allocator);
		return rx_false;
	}

	RX_DX12_SET_OBJECT_NAME(list, list->dx12CmdList->lpVtbl->SetName, name);
	if (FAILED(ID3D12GraphicsCommandList_QueryInterface(list->dx12CmdList, &IID_ID3D12CommandList, &list->dx12GfxCmdList))) {
		RX_DX12_SAVE_RELEASE(ID3D12GraphicsCommandList, list->dx12CmdList);
		RX_DX12_SAVE_RELEASE(ID3D12CommandAllocator, list->dx12Allocator);
		list->dx12GfxCmdList = rx_null;
		return rx_false;
	}
	ID3D12GraphicsCommandList_Close(list->dx12GfxCmdList); // will be open again when we start recording

	return rx_true;
}


RX_API rx_graphicsList rx_dx12MakeGraphicsList(rx_Context* ctx, const rx_graphicsPipeline pipeline) {
	rx_id pipelineId = { .id = pipeline.id };
#if 0
	int index = -1;
	rx__Dx12Pipeline* pipeline = rx_null;


	for (int i = 0; i < _countof(ctx->dx12->pools.buffers); i++) {
		if (ctx->dx12->pools.pipelines[i].generation == 0) {
			ctx->dx12->pools.pipelines[i].generation = 1;
			pipeline = &ctx->dx12->pools.pipelines[i];
			index = i;
			break;
		}
	}

	RX_ASSERT(index > -1);
	rx_id pipelineId = { {.index = index,.generation = pipeline->generation } };
#endif
	RX_ASSERT(!"Not implemented!");
	return (rx_graphicsList) { .id = 0 };
}

#if 0
void rx__dx12GfxCmdCopyBuffer(rx__DX12GfxCmdList* list, abGPU_Buffer* source, abGPU_Buffer* target) {
	ID3D12GraphicsCommandList_CopyResource(list->dx12CmdList, target->i_resource, source->i_resource);
}
#endif
rx_graphicsList rx_dx12MakeGfxList(rx_Context* ctx) {
	RX_ASSERT(ctx && ctx->dx12 && ctx->dx12->device);

	rx_u32 index = -1;
	rx__DX12GfxCmdList* gfxList = rx_null;
	// RX_ASSERT(desc && "Missing buffer description");
	// RX_ASSERT(desc->usage > 0 && "Missing Buffer usage");

	for (int i = 0; i < _countof(ctx->dx12->pools.gfxLists); i++) {
		if (ctx->dx12->pools.gfxLists[i].generation == 0) {
			ctx->dx12->pools.gfxLists[i].generation = 1;
			gfxList = &ctx->dx12->pools.gfxLists[i];
			index = i;
			break;
		}
	}

	RX_ASSERT(index > -1 && "No open lists");
	rx__dx12GfxCommandListInit(ctx->dx12->device, gfxList, "NoName");
	RX_ASSERT(!"Not Implemented yet!");
	return (rx_graphicsList) { .id = 0 };
}

void rx_dx12GraphicsListBegin(rx_Context* ctx, rx_graphicsList list, rx_GraphicsListStateDesc* config) {
	rx_id listId = { .id = list.id };
	RX_ASSERT(listId.generation > 0 && "Invalid graphics list handle");
	rx__DX12GfxCmdList* gfxList = &ctx->dx12->pools.gfxLists[listId.index];
	RX_ASSERT(listId.generation == gfxList->generation && "The provieded graphics list handle is not valid anymore");

	D3D12_DISCARD_REGION discardRegion = { .NumRects = 0, .pRects = rx_null, .NumSubresources = 1 };
	RX_ASSERT(!"Not implemented yet!");
	//vOMSetRenderTargets


}

////////////////////////////////////////////////
// Buffer
////////////////////////////////////////////////

D3D12_RESOURCE_STATES rx__dx12UseageToState(rx_bufferUsage usage) {
	static const D3D12_RESOURCE_STATES usageMap[rx_bufferUsage__count] = {
		[rx_bufferUsage__default] = D3D12_RESOURCE_STATE_COMMON,
		[rx_bufferUsage_vertices] = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		[rx_bufferUsage_constants] = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		[rx_bufferUsage_indices] = D3D12_RESOURCE_STATE_INDEX_BUFFER,
		[rx_bufferUsage_structures] = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		[rx_bufferUsage_structuresNonPixelStage] = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		[rx_bufferUsage_structuresAnyStage] = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		[rx_bufferUsage_edit] = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		[rx_bufferUsage_copySource] = D3D12_RESOURCE_STATE_COPY_SOURCE,
		[rx_bufferUsage_copyDestination] = D3D12_RESOURCE_STATE_COPY_DEST,
		[rx_bufferUsage_copyQueue] = D3D12_RESOURCE_STATE_COMMON,
		[rx_bufferUsage_cpuWrite] = D3D12_RESOURCE_STATE_GENERIC_READ
		// 0 is D3D12_RESOURCE_STATE_COMMON.
	};

	return (((rx_u32) usage < rx_bufferUsage__count) ? usageMap[usage] : D3D12_RESOURCE_STATE_COMMON);
}

rx_b32 rx__dx12InitBuffer(ID3D12Device2* device, rx__Dx12Buffer* buffer, char const* name, rx_bufferAccess access, rx_u32 size, rx_b32 editable, rx_bufferUsage initialUsage) {
	if (size == 0) {
		return rx_false;
	}
	// D3D12 ERROR: ID3D12Device::CreateCommittedResource: A buffer cannot be created on a D3D12_HEAP_TYPE_UPLOAD or D3D12_HEAP_TYPE_READBACK heap when either D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET or D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS is used.
	// [ STATE_CREATION ERROR #638: CREATERESOURCEANDHEAP_INVALIDHEAPPROPERTIES]
	// example: https://github.com/ocornut/imgui/blob/master/examples/imgui_impl_dx12.cpp#L144
	D3D12_RESOURCE_DESC desc = (D3D12_RESOURCE_DESC) {
	    .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
	    .Alignment = 0,
	    .Width = size,
	    .Height = 1,
	    .DepthOrArraySize = 1,
	    .MipLevels = 1,
	    .Format = DXGI_FORMAT_UNKNOWN,
	    .SampleDesc.Count = 1,
	    .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
	    .Flags = D3D12_RESOURCE_FLAG_NONE, // editable ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE,
    };

	D3D12_HEAP_PROPERTIES heapProperties = {
		.Type = D3D12_HEAP_TYPE_UPLOAD,
		.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN
	};

#if 0
	D3D12_HEAP_PROPERTIES heapProperties = { 0 };
	if (access == rx_bufferAccess_gpuInternal) {
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	} else {
		heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		initialUsage = rx_bufferAccess_cpuWrite;
	}
	//props.Type = D3D12_HEAP_TYPE_UPLOAD;
	//props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
#endif

	buffer->access = access;
	buffer->size = size;
	OutputDebugStringA("Create Buffer\n");
#if 0
	if (FAILED(ID3D12Device_CreateCommittedResource(device, &heapProperties, D3D12_HEAP_FLAG_NONE, &desc, rx__dx12UseageToState(initialUsage), rx_null, &IID_ID3D12Resource, &buffer->dx12Resource))) {
		return rx_false;
	}
#else
	if (FAILED(ID3D12Device_CreateCommittedResource(device, &heapProperties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, rx_null, &IID_ID3D12Resource, &buffer->dx12Resource))) {
		return rx_false;
	}
#endif
	rx__dx12SetObjectName(buffer->dx12Resource, (rx__dx12NameSetter*) buffer->dx12Resource->lpVtbl->SetName, name);
	buffer->dx12GpuVirtualAddress = ID3D12Resource_GetGPUVirtualAddress(buffer->dx12Resource);
#if 0
	// upload start data
	D3D12_RANGE range = { 0 };
	void* bufferPtr;
	ID3D12Resource_Map(buffer->dx12Resource, 0, &range, &bufferPtr);
	RX_ASSERT(!"Implement set data here!");
	memset(bufferPtr, bufferContent, bufferSize);
	ID3D12Resource_Unmap(buffer->dx12Resource, 0, &range);
	//if (fr->VertexBuffer->Map(0, &range, &bufferPtr) != S_OK)
#endif
	return rx_true;
}

RX_API rx_buffer rx_dx12MakeBuffer(rx_Context* ctx, const rx_BufferDesc* desc) {
	// this is really a bad for allocation performance, fix this at some point
	// Create a committed resource for the GPU resource in a default heap.
	rx_u32 index = -1;
	rx_u32 generation = 0;
	rx__Dx12Buffer* buffer = rx_null;
	RX_ASSERT(desc && "Missing buffer description");
	// RX_ASSERT(desc->usage > 0 && "Missing Buffer usage");

	for (int i = 0; i < _countof(ctx->dx12->pools.buffers); i++) {
		if (ctx->dx12->pools.buffers[i].generation == 0) {
			ctx->dx12->pools.buffers[i].generation = 1;
			buffer = &ctx->dx12->pools.buffers[i];
			index = i;
			generation = buffer->generation;
			break;
		}
	}
	RX_ASSERT(index >= 0);
	rx_b32 result = rx__dx12InitBuffer(ctx->dx12->device, buffer, desc->label, desc->access, desc->size, rx_true, desc->usage);
	RX_ASSERT(result && "Creating buffer failed");

	if (desc->content) {
		// upload data
	}
	rx_id bufferId = {{.index = index, .generation = generation}};
	return (rx_buffer) { .id = bufferId.id };
}
#if 0
RX_API rx_buffer rx_dx12MakeBuffer(rx_Context* ctx, const rx_BufferDesc* desc) {
    // this is really a bad for allocation performance, fix this at some point
    // Create a committed resource for the GPU resource in a default heap.
    ID3D12Resource* buffer = rx_null;
	rx_u32 bufferIndex;
    for (int i = 0; i < sizeof(ctx->dx12->pools.buffers); i++) {
        if (ctx->dx12->pools.buffers[i].generation == 0) {
            ctx->dx12->pools.buffers[i].generation = 1;
            buffer = &ctx->dx12->pools.buffers[i].dx12Resource;
			bufferIndex = i;
			break;
        }
    }

    size_t bufferSize = desc->size;
    // Create a committed resource for the GPU resource in a default heap.
	// This,pHeapProperties,HeapFlags,pDesc,InitialResourceState,pOptimizedClearValue,riidResource,ppvResource
	const D3D12_HEAP_PROPERTIES heapProps = (D3D12_HEAP_PROPERTIES) {
		.Type = D3D12_HEAP_TYPE_DEFAULT,
		.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		.CreationNodeMask = 1,
		.VisibleNodeMask = 1,
	};
	const D3D12_RESOURCE_DESC resDesc = (D3D12_RESOURCE_DESC){
		.Dimension = bufferSize,
		.Alignment = 0,
		.Width = bufferSize,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.SampleDesc.Count = 1,
		.SampleDesc.Quality = 0,
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = D3D12_RESOURCE_FLAG_NONE,
	};
    ID3D12Device2_CreateCommittedResource(
        ctx->dx12->device,
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        rx_null,
        &IID_ID3D12Resource,
		&buffer
    );
#if 0
    /*
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(pDestinationResource)));
    */
    // Create a committed resource for the upload.
    if (bufferData) {
        ID3D12Resource* intermediateResource;
		const D3D12_HEAP_PROPERTIES heapProps = (D3D12_HEAP_PROPERTIES) {
			.Type = D3D12_HEAP_TYPE_DEFAULT,
			.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
			.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
			.CreationNodeMask = 1,
			.VisibleNodeMask = 1,
		};
		const D3D12_RESOURCE_DESC resDesc = (D3D12_RESOURCE_DESC){
			.Dimension = bufferSize,
				.Alignment = 0,
				.Width = D3D12_HEAP_TYPE_DEFAULT,
				.Height = 1,
				.DepthOrArraySize = 1,
				.MipLevels = 1,
				.Format = DXGI_FORMAT_UNKNOWN,
				.SampleDesc.Count = 1,
				.SampleDesc.Quality = 0,
				.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
				.Flags = D3D12_RESOURCE_FLAG_NONE,
		};
        ID3D12Device2_CreateCommittedResource(
            ctx->dx12->device,
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &resDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            rx_null,
            &IID_ID3D12Resource,
            &intermediateResource
        );
#if 0
        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(pIntermediateResource)));
#endif
        D3D12_SUBRESOURCE_DATA subresourceData = (D3D12_SUBRESOURCE_DATA) {
            .pData = bufferData,
            .RowPitch = bufferSize,
            .SlicePitch = subresourceData.RowPitch,
        };

        UpdateHeapSubresources(commandList, *buffer, *intermediateResource, 0, 0, 1, &subresourceData);
    }
#endif
}
#endif

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
    // ID3D12Device2* d3d12Device2;
	HRESULT result = D3D12CreateDevice(RX_DX12_GET(adapter), D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device2, d3d12Device2Out);
    RX_DX12_ERR_COND(result, rx_errorCode_cantCreate);

    // Enable debug messages in debug mode.
#if 0
    if (debug) {
        ID3D12InfoQueue* pInfoQueue = rx_null;
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
#endif
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
        if (SUCCEEDED(RX_DX12_AS(IDXGIFactory4, factory4, IDXGIFactory5, &factory5))) {
            if (FAILED(IDXGIFactory5_CheckFeatureSupport(factory5, DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing)))) {
				allowTearing = FALSE;
            }
			IDXGIFactory5_Release(factory5);
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
	++(*fenceValue);
	*fenceValueForSignal = (*fenceValue);
	// ID3D12CommandQueue_Signal(commandQueue, fence, *fenceValueForSignal);
	RX_DX12_ERR_COND(ID3D12CommandQueue_Signal(commandQueue, fence, *fenceValueForSignal), rx_errorCode_error);
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
#if 1
	if (ctx->debug) {
		ID3D12Debug* debugController0 = rx_null;
		ID3D12Debug1* debugController1 = rx_null;
		RX_ASSERT(SUCCEEDED(D3D12GetDebugInterface(&IID_ID3D12Debug, &debugController0)) && "Failed creating DebugLayer");
		//ID3D12Debug_EnableDebugLayer(debugController0, debug);
		RX_DX12_AS(ID3D12Debug, debugController0, ID3D12Debug1, &debugController1);
		RX_ASSERT(debugController1);
		ID3D12Debug1_EnableDebugLayer(debugController1);
		ID3D12Debug1_SetEnableGPUBasedValidation(debugController1, rx_true);
	}
#endif
    ID3D12Device2* d3d12Device = rx_null;
	rx_errorCode code = rx__dx12CreateDevice(dxgiAdapter4, &d3d12Device, ctx->debug);
	ctx->dx12->device = d3d12Device;
	ctx->dx12->backbuffers[0] = rx_null;

#if 0
	if (ctx->debug) {
		ID3D12DebugDevice1* debugDevice;
		RX_DX12_AS(ID3D12Device2, d3d12Device, ID3D12DebugDevice1, &debugDevice);
		RX_ASSERT(debugDevice);
		ID3D12DebugDevice1_ReportLiveDeviceObjects(debugDevice, D3D12_RLDO_DETAIL);
	}
#endif
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