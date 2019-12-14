#ifndef _RENDERER_H_
#define _RENDERER_H_

#ifndef RX_API
#define RX_API extern
#endif

#include <stdint.h>
#include <stdbool.h>

#if RX_GLOBAL
#define RX_APIDEF  
#define RX_APIDEFS  
#else
#define RX_APIDEF rx_Context* ctx,
#define RX_APIDEFS rx_Context* ctx
#endif
typedef uint32_t rx_u32;
typedef rx_u32 rx_b32;
typedef uint64_t rx_u64;

#ifndef RX_MAX_VERTEX_ATTRIBUTES
#define RX_MAX_VERTEX_ATTRIBUTES 16
#endif
#ifndef RX_MAX_UB_MEMBERS
#define RX_MAX_UB_MEMBERS 16
#endif
#ifndef RX_MAX_SHADERSTAGE_IMAGES
#define RX_MAX_SHADERSTAGE_IMAGES 12
#endif
#ifndef RX_MAX_SHADERSTAGE_UBS
#define RX_MAX_SHADERSTAGE_UBS 4
#endif
#ifndef RX_MAX_SHADERSTAGE_BUFFERS
#define RX_MAX_SHADERSTAGE_BUFFERS 8
#endif
/*
	SG_INVALID_ID = 0,
	SG_NUM_SHADER_STAGES = 2,
	SG_NUM_INFLIGHT_FRAMES = 2,
	SG_MAX_COLOR_ATTACHMENTS = 4,
	SG_MAX_SHADERSTAGE_BUFFERS = 8,
	SG_MAX_SHADERSTAGE_IMAGES = 12,
	SG_MAX_SHADERSTAGE_UBS = 4,
	SG_MAX_UB_MEMBERS = 16,
	SG_MAX_VERTEX_ATTRIBUTES = 16, 
	SG_MAX_MIPMAPS = 16,
	SG_MAX_TEXTUREARRAY_LAYERS = 128

*/

typedef enum rx_errorCode {
	rx_errorCode_ok = 0,
	rx_errorCode_error,
	rx_errorCode_cantCreate,
	rx_errorCode_staticMemToSmall,
	rx_errorCode_allocFailed,

	rx_errorCode_cantCreateFence,

	rx_errorCode_dx12Begin,
	rx_errorCode_dx12QueueInfoFailed,
	rx_errorCode_dx12GIFactoryCreationFailed,
	rx_errorCode_dx12AdapterNotFound,
	rx_errorCode_dx12AdapterInterfaceQueryFailed,
	rx_errorCode_dx12DeviceCreationFailed, 
	rx_errorCode_dx12GraphicsCommandQueueCreationFaile,
	rx_errorCode_dx12CopyCommandQueueCreationFailed,
	rx_errorCode_dx12End,
} rx_errorCode;

typedef struct rx_Context rx_Context;

typedef enum rx_renderBackend {
    rx_renderBackend_DX12,
    rx_renderBackend_VK,
    rx_renderBackend_MTL,
} rx_renderBackend;

typedef struct rx_Desc {
    rx_renderBackend renderBackend;
    void* win32WindowHandle;
    const void* dx12Hwnd;
    const void* dx11DeviceContext;
	rx_b32 vsyncDeactivated;
	rx_b32 useWrap;
	rx_b32 debug;
} rx_Desc;

typedef struct rx_uniformset {
    rx_u64 id;
} rx_uniformsetId;

typedef struct rx_drawlist {
    rx_u64 id;
} rx_drawlist;

typedef struct rx_computeList {
    rx_u64 id;
} rx_computeList;

typedef struct rx_graphicsList {
	rx_u64 id;
} rx_graphicsList;

typedef struct rx_graphicsPipeline {
    rx_u64 id;
} rx_graphicsPipeline;

typedef struct rx_computePipeline {
	rx_u64 id;
} rx_computePipeline;

typedef struct rx_graphicsShader {
	rx_u64 id;
} rx_graphicsShader;

RX_API rx_Context* rx_createContext(const rx_Desc* desc);
RX_API void rx_commit(RX_APIDEFS);

typedef enum rx_vertexStep {
	rx_vertexStep__default,     /* value 0 reserved for default-init */
	rx_vertexStep_perVertex,
	rx_vertexStep_perInstance,
	rx_vertexStep__count,
	rx_vertexStep__force32 = 0x7FFFFFFF
} rx_vertexStep;

/*
	rx_uniformType
	The data type of a uniform block member. This is used to
	describe the internal layout of uniform blocks when creating
	a shader object.
*/
typedef enum rx_uniformType {
	rx_uniformType__invalid,
	rx_uniformType_float,
	rx_uniformType_float2,
	rx_uniformType_float3,
	rx_uniformType_float4,
	rx_uniformType_mat4,
	rx_uniformType__count,
	rx_uniformType__force32 = 0x7FFFFFFF
} rx_uniformType;


/*
    rx_indexType
    Indicates whether indexed rendering (fetching vertex-indices from an
    index buffer) is used, and if yes, the index data type (16- or 32-bits).
    This is used in the sg_pipeline_desc.index_type member when creating a
    pipeline object.
    The default index type is SG_INDEXTYPE_NONE.
*/
typedef enum rx_indexType {
    rx_indexType__default,   /* value 0 reserved for default-init */
    rx_indexType_none,
    rx_indexType_uint16,
    rx_indexType_uint32,
    rx_indexType__count,
    rx_indexType__force32 = 0x7FFFFFFF
} rx_indexType;

/*
    rx_imageType
    Indicates the basic image type (2D-texture, cubemap, 3D-texture
    or 2D-array-texture). 3D- and array-textures are not supported
    on the GLES2/WebGL backend. The image type is used in the
    sg_image_desc.type member when creating an image.
    The default image type when creating an image is SG_IMAGETYPE_2D.
*/
typedef enum rx_imageType {
    rx_imageType__default,  /* value 0 reserved for default-init */
    rx_imageType_2d,
    rx_imageType_cube,
    rx_imageType_3d,
    rx_imageType_array,
    rx_imageType__count,
    rx_imageType__force32 = 0x7FFFFFFF
} rx_imageType;

/* pipeline */

typedef enum rx_blendFactor {
	rx_blendFactor__default, /* value 0 reserved for default-init */
    rx_blendFactor_zero,
    rx_blendFactor_one,
    rx_blendFactor_srcColor,
    rx_blendFactor_oneMinusSrcColor,
    rx_blendFactor_srcAlpha,
    rx_blendFactor_oneMinusSrcAlpha,
    rx_blendFactor_dstColor,
    rx_blendFactor_oneMinusDstColor,
    rx_blendFactor_dstAlpha,
    rx_blendFactor_oneMinusDstAlpha,
    rx_blendFactor_srcAlpha_SATURATED,
    rx_blendFactor_blendColor,
    rx_blendFactor_oneMinusBlendColor,
    rx_blendFactor_blendAlpha,
    rx_blendFactor_oneMinusBlendAlpha,
    rx_blendFactor__count,
    rx_blendFactor__forceU32 = 0x7FFFFFFF
} rx_blendFactor;

typedef enum rx_blendOp {
    rx_blendOp__default, /* value 0 reserved for default-init */
    rx_blendOp_add,
    rx_blendOp_substract,
    rx_blendOp_reverseSubtract,
    rx_blendOp__count,
    rx_blendOp__forceU32 = 0x7FFFFFFF
} rx_blendOp;

/*
    rx_vertexFormat
    The data type of a vertex component. This is used to describe
    the layout of vertex data when creating a pipeline object.
*/
typedef enum rx_vertexFormat {
    rx_VERTEXFORMAT__INVALID,
    rx_vertexFormat_float,
    rx_vertexFormat_float2,
    rx_vertexFormat_float3,
    rx_vertexFormat_float4,
    rx_vertexFormat_byte4,
    rx_vertexFormat_byte4N,
    rx_vertexFormat_uByte4,
    rx_vertexFormat_uByte4N,
    rx_vertexFormat_short2,
    rx_vertexFormat_short2N,
    rx_vertexFormat_uShort2N,
    rx_vertexFormat_short4,
    rx_vertexFormat_short4N,
    rx_vertexFormat_uShort4N,
    rx_vertexFormat_uInt10_N2,
    rx_VERTEXFORMAT__NUM,
    rx_VERTEXFORMAT__FORCE_U32 = 0x7FFFFFFF
} rx_vertexFormat;

typedef enum rx_pixelFormat {
    rx_pixelFormat__default,    /* value 0 reserved for default-init */
    rx_pixelFormat_NONE,

    rx_pixelFormat_R8,
    rx_pixelFormat_R8SN,
    rx_pixelFormat_R8UI,
    rx_pixelFormat_R8SI,

    rx_pixelFormat_R16,
    rx_pixelFormat_R16SN,
    rx_pixelFormat_R16UI,
    rx_pixelFormat_R16SI,
    rx_pixelFormat_R16F,
    rx_pixelFormat_RG8,
    rx_pixelFormat_RG8SN,
    rx_pixelFormat_RG8UI,
    rx_pixelFormat_RG8SI,

    rx_pixelFormat_R32UI,
    rx_pixelFormat_R32SI,
    rx_pixelFormat_R32F,
    rx_pixelFormat_RG16,
    rx_pixelFormat_RG16SN,
    rx_pixelFormat_RG16UI,
    rx_pixelFormat_RG16SI,
    rx_pixelFormat_RG16F,
    rx_pixelFormat_RGBA8,
    rx_pixelFormat_RGBA8SN,
    rx_pixelFormat_RGBA8UI,
    rx_pixelFormat_RGBA8SI,
    rx_pixelFormat_BGRA8,
    rx_pixelFormat_RGB10A2,
    rx_pixelFormat_RG11B10F,

    rx_pixelFormat_RG32UI,
    rx_pixelFormat_RG32SI,
    rx_pixelFormat_RG32F,
    rx_pixelFormat_RGBA16,
    rx_pixelFormat_RGBA16SN,
    rx_pixelFormat_RGBA16UI,
    rx_pixelFormat_RGBA16SI,
    rx_pixelFormat_RGBA16F,

    rx_pixelFormat_RGBA32UI,
    rx_pixelFormat_RGBA32SI,
    rx_pixelFormat_RGBA32F,

    rx_pixelFormat_DEPTH,
    rx_pixelFormat_DEPTH_STENCIL,

    rx_pixelFormat_BC1_RGBA,
    rx_pixelFormat_BC2_RGBA,
    rx_pixelFormat_BC3_RGBA,
    rx_pixelFormat_BC4_R,
    rx_pixelFormat_BC4_RSN,
    rx_pixelFormat_BC5_RG,
    rx_pixelFormat_BC5_RGSN,
    rx_pixelFormat_BC6H_RGBF,
    rx_pixelFormat_BC6H_RGBUF,
    rx_pixelFormat_BC7_RGBA,
    rx_pixelFormat_PVRTC_RGB_2BPP,
    rx_pixelFormat_PVRTC_RGB_4BPP,
    rx_pixelFormat_PVRTC_RGBA_2BPP,
    rx_pixelFormat_PVRTC_RGBA_4BPP,
    rx_pixelFormat_ETC2_RGB8,
    rx_pixelFormat_ETC2_RGB8A1,
    rx_pixelFormat_ETC2_RGBA8,
    rx_pixelFormat_ETC2_RG11,
    rx_pixelFormat_ETC2_RG11SN,

    rx_pixelFormat__count,
    rx_pixelFormat__forceU32 = 0x7FFFFFFF
} rx_pixelFormat;

typedef struct rx_shaderAttrDesc {
	const char* name;           /* GLSL vertex attribute name (only required for GLES2) */
	const char* semName;       /* HLSL semantic name */
	int semIndex;              /* HLSL semantic index */
} rx_shaderAttrDesc;

typedef struct rx_ShaderUniformDesc {
	const char* name;
	rx_uniformType type;
	int array_count;
} rx_ShaderUniformDesc;

typedef struct rx_ShaderUniformBlockDesc {
	int size;
	rx_ShaderUniformDesc uniforms[RX_MAX_UB_MEMBERS];
} rx_ShaderUniformBlockDesc;

typedef struct rx_ShaderImageDesc {
	const char* name;
	rx_imageType type;
} rx_ShaderImageDesc;

typedef struct rx_shaderStageDesc {
	const char* source;
	const uint8_t* byteCode;
	int byte_code_size;
	const char* entry;
	rx_ShaderUniformBlockDesc uniform_blocks[RX_MAX_SHADERSTAGE_UBS];
	rx_ShaderImageDesc images[RX_MAX_SHADERSTAGE_IMAGES];
} sg_shader_stage_desc;

typedef struct rx_GraphicsShaderDesc {
	uint32_t _start_canary;
	rx_shaderAttrDesc attrs[RX_MAX_VERTEX_ATTRIBUTES];
	sg_shader_stage_desc vs;
	sg_shader_stage_desc fs;
	const char* label;
	uint32_t _end_canary;
} rx_GraphicsShaderDesc;

typedef struct rx_BlendState {
    bool enabled;
    rx_blendFactor srcFactorRgb;
    rx_blendFactor dstFactorRgb;
    rx_blendOp opRgb;
    rx_blendFactor srcFactorAlpha;
    rx_blendFactor dstFactorAlpha;
    rx_blendOp opAlpha;
    uint8_t colorWriteMask;
    int colorAttachmentCount;
    rx_pixelFormat colorFormat;
    rx_pixelFormat depthFormat;
    float blendColor[4];
} rx_BlendState;

typedef struct rx_BufferLayoutDesc {
    int stride;
    rx_vertexStep stepFunc;
    int step_rate;
} rx_BufferLayoutDesc;

typedef struct rx_VertexAttrDesc {
    int buffer_index;
    int offset;
    rx_vertexFormat format;
} rx_VertexAttrDesc;

typedef struct rx_LayoutDesc {
    rx_BufferLayoutDesc buffers[RX_MAX_SHADERSTAGE_BUFFERS];
    rx_VertexAttrDesc attrs[RX_MAX_VERTEX_ATTRIBUTES];
} rx_LayoutDesc;

typedef struct rx_GraphicsPipelineDesc {
	uint32_t _startCanary;
	rx_LayoutDesc layout;
	rx_graphicsShader shader;
	rx_BlendState blend;
	const char* label;
#if 0
	sg_primitive_type primitive_type;
	sg_index_type index_type;
	sg_depth_stencil_state depth_stencil;
	sg_rasterizer_state rasterizer;
#endif
	uint32_t _endCanary;
} rx_GraphicsPipelineDesc;

typedef struct rx_ComputePipelineDesc {
    int foo;
} rx_ComputePipelineDesc;
/* graphics list */
RX_API rx_graphicsPipeline rx_createGraphicsPipeline(RX_APIDEF rx_GraphicsPipelineDesc* desc);
RX_API void rx_graphicsListCreate(RX_APIDEFS);
RX_API void rx_graphicsListClearRenderTargetView(RX_APIDEF rx_graphicsList list, const float rgba[4]);
/* compute lists */
RX_API rx_computePipeline rx_createComputePipeline(RX_APIDEF rx_ComputePipelineDesc* desc);
RX_API void rx_computeListBindPipeline(RX_APIDEF rx_computeList list, rx_graphicsPipeline pipeline);
RX_API void rx_computeListBindUniformset(RX_APIDEF rx_computeList list, rx_graphicsPipeline pipeline, rx_uniformsetId set, int index);
RX_API void rx_computeListPushConstants(RX_APIDEF rx_computeList list, void *data, int size);
RX_API void rx_computeListAddBarrier(RX_APIDEF rx_computeList list);
RX_API void rx_computeListDispatch(RX_APIDEF rx_computeList list, int xGroups, int yGroups, int zGroups);
RX_API void rx_computeListEnd(RX_APIDEF rx_computeList list);
RX_API void rx_freeComputeList(RX_APIDEF rx_computeList list);

#endif /* _RENDERER_H_ */
