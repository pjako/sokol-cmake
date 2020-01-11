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
typedef uint8_t rx_u8;
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

#ifndef RX_MAX_RENDERTARGETS
#define RX_MAX_RENDERTARGETS 8
#endif

#ifndef RX_SET_BUFFERS_COUNT
#define RX_SET_BUFFERS_COUNT 8
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

typedef struct rx_graphicsProgram {
	rx_u64 id;
} rx_graphicsProgram;

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
    rx_indexType_u16,
    rx_indexType_u32,
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

typedef enum rx_bitOperation {
	rx_bitOperation_copy, // s - zero, means bit operation is disabled.
	rx_bitOperation_copyInv, // ~s
	rx_bitOperation_setZero, // 0
	rx_bitOperation_setOne, // 1
	rx_bitOperation_reverse, // ~d
	rx_bitOperation_and, // d & s
	rx_bitOperation_notAnd, // ~(d & s)
	rx_bitOperation_or, // d | s
	rx_bitOperation_notOr, // ~(d | s)
	rx_bitOperation_xor, // d ^ s
	rx_bitOperation_equal, // ~(d ^ s)
	rx_bitOperation_andRev, // ~d & s
	rx_bitOperation_andInv, // d & ~s
	rx_bitOperation_orRev, // ~d | s
	rx_bitOperation_orInv, // d | ~s
	rx_bitOperation__count,
    rx_bitOperation__force32 = 0x7FFFFFFF
} rx_bitOperation;

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
    rx_VERTEXFORMAT__invalid,
    rx_vertexFormat_f32x1,
    rx_vertexFormat_f32x2,
    rx_vertexFormat_f32x3,
    rx_vertexFormat_f32x4,
    rx_vertexFormat_f16x2,
    rx_vertexFormat_f16x4,
    rx_vertexFormat_s8x4,
    rx_vertexFormat_s8x4N,
    rx_vertexFormat_u8x4,
    rx_vertexFormat_u8x4N,
    rx_vertexFormat_s16x2,
    rx_vertexFormat_s16x2N,
    rx_vertexFormat_u16x2N,
    rx_vertexFormat_s16x4,
    rx_vertexFormat_s16x4N,
    rx_vertexFormat_u16x4N,
    rx_vertexFormat_u10x3U2x1N,
    rx_VERTEXFORMAT__NUM,
    rx_VERTEXFORMAT__FORCE_U32 = 0x7FFFFFFF
} rx_vertexFormat;

typedef enum rx_pixelFormat {
    rx_pixelFormat__default,    /* value 0 reserved for default-init */
    rx_pixelFormat_none,

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

    
    rx_pixelFormat_d32,
    rx_pixelFormat_d24S8,

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

typedef struct rx_ShaderStageDesc {
	const char* source;
	const rx_u8* byteCode;
	int byteCodeSize;
	const char* entry;
	rx_ShaderUniformBlockDesc uniform_blocks[RX_MAX_SHADERSTAGE_UBS];
	rx_ShaderImageDesc images[RX_MAX_SHADERSTAGE_IMAGES];
} rx_ShaderStageDesc;

typedef struct rx_GraphicsProgramDesc {
	uint32_t _startCanary;
	rx_shaderAttrDesc attrs[RX_MAX_VERTEX_ATTRIBUTES];
	rx_ShaderStageDesc vs;
	rx_ShaderStageDesc fs;
	uint32_t _endCanary;
} rx_GraphicsProgramDesc;

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
    rx_u32 stride;
    rx_vertexStep stepFunc;
	rx_u32 stepRate;
} rx_BufferLayoutDesc;

typedef struct rx_ComputePipelineDesc {
    rx_u64 foo;
} rx_ComputePipelineDesc;

typedef struct rx_buffer {
    rx_u64 id;
} rx_buffer;

// These must be immutable as long as they are attached to an active input list.
typedef enum rx_inputType {
    rx_inputType__invalid,
	rx_inputType_constantBuffer, // Buffer and offset bound directly.
	rx_inputType_constantBufferHandle, // Buffer bound via a handle.
	rx_inputType_uniform, // One of the constant input types, depending on the strategy.
	rx_inputType_structureBufferHandle,
	rx_inputType_editBufferHandle,
	rx_inputType_textureHandle,
	rx_inputType_editImageHandle,
	rx_inputType_samplerHandle,
    rx_inputType__count,
    rx_inputType__forceU32 = 0x7FFFFFFF
} rx_inputType;

typedef enum rx_stageVisibility {
    rx_stageVisibility__default,
    rx_stageVisibility_all, /* default */
    rx_stageVisibility_vertex,
    rx_stageVisibility_fragment,
    rx_stageVisibility__count,
    rx_stageVisibility__forceU32 = 0x7FFFFFFF
} rx_stageVisibility;

/*
    sg_color_mask
    Selects the color channels when writing a fragment color to the
    framebuffer. This is used in the members
    sg_pipeline_desc.blend.color_write_mask when creating a pipeline object.
    The default colormask is rx_colorMask_RGBA (write all colors channels)
*/
typedef enum rx_colorMask {
    rx_colorMask__default = 0,      /* value 0 reserved for default-init */
    rx_colorMask_NONE = (0x10),     /* special value for 'all channels disabled */
    rx_colorMask_r = (1<<0),
    rx_colorMask_g = (1<<1),
    rx_colorMask_b = (1<<2),
    rx_colorMask_a = (1<<3),
    rx_colorMask_rgb = 0x7,
    rx_colorMask_rgba = 0xF,
    rx_colorMask_forceU32 = 0x7FFFFFFF
} rx_colorMask;

typedef struct rx_Input {
	rx_inputType type;
	rx_stageVisibility stageVisibility;
	union {
		struct { uint8_t constantBufferIndex, globalIndex; } constantBuffer;
		struct { uint8_t constantBufferFirstIndex, globalIndex, count; } constantBufferHandle;
		struct { uint8_t structureBufferFirstIndex, globalIndex, count; } structureBufferHandle;
		struct { uint8_t editBufferFirstIndex, globalIndex, count; } editBufferHandle;
		struct { uint8_t textureFirstIndex, globalIndex, count; } imageHandle;
		struct { uint8_t editImageFirstIndex, globalIndex, count; } editImageHandle;
		// Static samplers are an optimization that may be unsupported, not a full replacement for binding.
		// staticSamplerIndex is an index in the array of static samplers that is passed when the input list is created.
		// If static samplers are supported and provided, they will override bindings for non-SamplerDynamicOnly inputs.
		// However, as they may be unsupported by implementations, samplers still must be bound.
		struct { uint8_t samplerFirstIndex, globalIndex, count, staticSamplerIndex; } samplerHandle;
	} parameters;
} rx_Input;

typedef struct rx_VertexAttributeDesc {
    int bufferIndex;
    int offset;
    const char* semanticName;
    int semanticIndex;
    rx_vertexFormat format;
} rx_VertexAttributeDesc;


typedef enum rx_shaderStage {
//	rx_shaderStage_invalid,
	rx_shaderStage_vertex,
	rx_shaderStage_pixel,
	rx_shaderStage_compute,
    rx_shaderStage__count,
    rx_shaderStage__forceU32 = 0x7FFFFFFF
} rx_shaderStage;

typedef enum rx_shaderStageBits {
	rx_shaderStageBits_vertex = 1 << rx_shaderStage_vertex,
	rx_shaderStageBits_pixel = 1 << rx_shaderStage_pixel,
	rx_shaderStageBits_compute = 1 << rx_shaderStage_compute
} rx_shaderStageBits;

#define RX_MAX_INPUTS 16
#define RX_MAX_SAMPLER 24
#define RX_MAX_INPUT_BUFFERS 16
#define RX_MAX_INPUT_ATTRIBUTES 16
typedef struct rx_InputLayoutDesc {
	rx_stageVisibility uniformStages;
	rx_u32 uniform32BitCount; // Add abGPU_InputConfig_UniformDrawIndex32BitCount to this if needed.
	rx_b32 uniformUseBuffer;
//	uint8_t uniformBufferGlobalIndex;
    rx_Input inputs[RX_MAX_INPUTS];
    rx_BufferLayoutDesc vertexBuffers[RX_MAX_INPUT_BUFFERS];
	rx_VertexAttributeDesc vertexAttributes[RX_MAX_INPUT_ATTRIBUTES];
    const char* label;
} rx_InputLayoutDesc;

typedef struct rx_RenderTargetDesc {
	rx_pixelFormat format;
	rx_b32 blend;

    rx_blendFactor srcFactorRgb;
    rx_blendFactor dstFactorRgb;
    rx_blendOp opRgb;

    rx_blendFactor srcFactorAlpha;
    rx_blendFactor dstFactorAlpha;
    rx_blendOp opAlpha;

	rx_bitOperation bitOperation;
	rx_u8 colorWriteMask;
} rx_RenderTargetDesc;

/*
    rx_cullMode
    The face-culling mode, this is used in the
    sg_pipeline_desc.rasterizer.cull_mode member when creating a
    pipeline object.
    The default cull mode is rx_cullMode_none
*/
typedef enum rx_cullMode {
    rx_cullMode__default,   /* value 0 reserved for default-init */
    rx_cullMode_none,
    rx_cullMode_front,
    rx_cullMode_back,
    rx_cullMode__count,
    rx_cullMode__forceU32 = 0x7FFFFFFF
} rx_cullMode;

/*
    rx_faceWinding
    The vertex-winding rule that determines a front-facing primitive. This
    is used in the member sg_pipeline_desc.rasterizer.face_winding
    when creating a pipeline object.
    The default winding is rx_faceWinding_clockWise (clockwise)
*/
typedef enum rx_faceWinding {
    rx_faceWinding__default,    /* value 0 reserved for default-init */
    rx_faceWinding_counterClockWise,
    rx_faceWinding_clockWise,
    rx_faceWinding__count,
    rx_faceWinding__forceU32 = 0x7FFFFFFF
} rx_faceWinding;

typedef enum rx_fillMode {
    rx_fillMode_solid,
    rx_fillMode_wireFrame,
	rx_fillMode__forceU32 = 0x7FFFFFFF
} rx_fillMode;

typedef struct rx_RasterizerState {
    bool alphaToCoverageEnabled;
    rx_fillMode fillMode;
    rx_cullMode cullMode;
    rx_faceWinding faceWinding;
    int sampleCount;
    float depthBias;
    float depthBiasSlopeScale;
    float depthBiasClamp;
} rx_RasterizerState;

/*
    rx_compareFunc
    The compare-function for depth- and stencil-ref tests.
    This is used when creating pipeline objects in the members:

    The default compare func for depth- and stencil-tests is
    rx_compareFunc_always.
*/
typedef enum rx_compareFunc {
    rx_compareFunc__default,    /* value 0 reserved for default-init */
    rx_compareFunc_never,
    rx_compareFunc_less,
    rx_compareFunc_equal,
    rx_compareFunc_lessEqual,
    rx_compareFunc_greater,
    rx_compareFunc_notEqual,
    rx_compareFunc_greaterEqual,
    rx_compareFunc_always,      /* default */
    rx_compareFunc__count,
    rx_compareFunc__forceU32 = 0x7FFFFFFF
} rx_compareFunc;

/*
    rx_stencilOp
    The operation performed on a currently stored stencil-value when a
    comparison test passes or fails. This is used when creating a pipeline
    The default value is rx_stencilOp_keep.
*/
typedef enum rx_stencilOp {
    rx_stencilOp__default,      /* value 0 reserved for default-init */
    rx_stencilOp_keep,          /* default */
    rx_stencilOp_zero,
    rx_stencilOp_replace,
    rx_stencilOp_incrementClamp,
    rx_stencilOp_decrementClamp,
    rx_stencilOp_invert,
    rx_stencilOp_incrementWrap,
    rx_stencilOp_decrementWrap,
    rx_stencilOp__count,
    rx_stencilOp_forceU32 = 0x7FFFFFFF
} rx_stencilOp;


typedef struct rx_stencilState {
    rx_stencilOp failOp;
    rx_stencilOp depthFailOp;
    rx_stencilOp passOp;
    rx_compareFunc compareFunc;
} rx_stencilState;

typedef struct rx_DepthStencilState {
    rx_stencilState stencilFront;
    rx_stencilState stencilBack;
    rx_compareFunc depthCompareFunc;
    bool depthWriteEnabled;
    bool stencilEnabled;
    uint8_t stencilReadMask;
    uint8_t stencilWriteMask;
    uint8_t stencilRef;
} rx_DepthStencilState;


/*
    rx_primitiveType
    This is the common subset of 3D primitive types supported across all 3D
    APIs. This is used in the sg_pipeline_desc.primitive_type member when
    creating a pipeline object.
    The default primitive type is rx_primitiveType_triangles.
*/
typedef enum rx_primitiveType {
    rx_primitiveType__default,  /* value 0 reserved for default-init */
    rx_primitiveType_points,
    rx_primitiveType_lines,
    // rx_primitiveType_lineStrip,
    rx_primitiveType_triangles,
    // rx_primitiveType_triangleStrips,
    rx_primitiveType__count,
    rx_primitiveType__forceU32 = 0x7FFFFFFF
} rx_primitiveType;

typedef struct rx_GraphicsPipelineDesc {
	uint32_t _startCanary;
    rx_InputLayoutDesc input;
	rx_BlendState blend;
    rx_RasterizerState rasterizer;
    rx_primitiveType primitiveType;
    rx_indexType indexType;
    rx_pixelFormat depthFormat; /* DXGI_FORMAT_D32_FLOAT */
    rx_DepthStencilState depthStencil;
    rx_GraphicsProgramDesc program;
    rx_RenderTargetDesc renderTargets[RX_MAX_RENDERTARGETS];
	const char* label;
	uint32_t _endCanary;
} rx_GraphicsPipelineDesc;

typedef enum rx_bufferType {
    rx_bufferType__default,         /* value 0 reserved for default-init */
    rx_bufferType_vertexBuffer,
    rx_bufferType_indexBuffer,
    rx_bufferType__count,
    rx_bufferType__forceU32 = 0x7FFFFFFF
} rx_bufferType;

typedef enum rx_bufferUsage {
	rx_bufferUsage__default,
	rx_bufferUsage_vertices,
	rx_bufferUsage_constants,
	rx_bufferUsage_indices,
	rx_bufferUsage_structures, // Structured data readable in pixel shaders.
	rx_bufferUsage_structuresNonPixelStage,  // Structured data readable in non-pixel shaders.
	rx_bufferUsage_structuresAnyStage, // Structured data readable at all shader stages.
	rx_bufferUsage_edit, // Directly editable in shaders.
	rx_bufferUsage_copySource,
	rx_bufferUsage_copyDestination,
	rx_bufferUsage_copyQueue, // Owned by the copy command queue (which doesn't have the concept of usages).
	rx_bufferUsage_cpuWrite, // CPU-writable or upload buffer.
	rx_bufferUsage__count,
	rx_bufferUsage__forceU32 = 0x7FFFFFFF
} rx_bufferUsage;

typedef enum rx_bufferAccess {
	rx_bufferAccess__default,
	rx_bufferAccess_gpuInternal,
	rx_bufferAccess_cpuWrite,
	rx_bufferAccess_upload,
	rx_bufferAccess__forceU32 = 0x7FFFFFFF
} rx_bufferAccess;

typedef struct rx_BufferDesc {
    uint32_t _startCanary;
    int size;
    rx_bufferType type;
    //sg_usage usage;
    const void* content;
    const char* label;
	rx_bufferAccess access;
	rx_bufferUsage usage;
    /* GL specific */
    //uint32_t gl_buffers[SG_NUM_INFLIGHT_FRAMES];
    /* Metal specific */
    //const void* mtl_buffers[SG_NUM_INFLIGHT_FRAMES];
    /* D3D11 specific */
    //const void* d3d11_buffer;
    uint32_t _endCanary;
} rx_BufferDesc;

typedef struct rx_SetVertexBufferViewsDesc {
    rx_u32 startSlot;
    rx_buffer buffers[RX_SET_BUFFERS_COUNT];
    rx_u32 strides[RX_SET_BUFFERS_COUNT];
    rx_u32 offsets[RX_SET_BUFFERS_COUNT];
} rx_SetVertexBufferViewsDesc;

typedef struct rx_IndexBufferView {
  rx_buffer buffer;
  rx_indexType format;
  rx_u32 offset;
} rx_IndexBufferView;

typedef struct rx_VertexBufferView {
  rx_buffer buffer;
  rx_indexType format;
  rx_u32 offset;
  rx_u32 sizeInBytes;
  rx_u32 strideInBytes;
} rx_VertexBufferView;
#ifndef RX_MAX_RENDER_TARGETS
#define RX_MAX_RENDER_TARGETS 8
#endif
typedef struct rx_GraphicsListStateDesc {
	unsigned int colorCount;
#if 0
	abGPU_RT color[abGPU_RT_Count];
	abGPU_RT depth;
	abGPU_RT_PrePostAction stencilPrePostAction;

	#if defined(abBuild_GPUi_D3D)
	D3D12_CPU_DESCRIPTOR_HANDLE i_descriptorHandles[abGPU_RT_Count + 2u]; // Depth and stencil (duplicated) at abGPU_RT_Count.
	ID3D12Resource * i_resources[abGPU_RT_Count + 2u]; // Depth and stencil (duplicated) at abGPU_RT_Count.
	unsigned int i_subresources[abGPU_RT_Count + 2u]; // Depth and stencil at abGPU_RT_Count.
	unsigned int i_preDiscardBits, i_preClearBits, i_postDiscardBits; // Depth and stencil at (1...2)<<abGPU_RT_Count.
	#endif
#endif
} rx_GraphicsListStateDesc;
/* graphics program */
// RX_API rx_graphicsProgram rx_makeGraphicsProgram(RX_APIDEF rx_GraphicsProgramDesc* desc);
/* buffer */
RX_API rx_buffer rx_makeBuffer(RX_APIDEF const rx_BufferDesc* desc);
//RX_API void rx_updateBuffer(rx_buffer buf, const void* ptr, int numBytes);
/* graphics list */
RX_API rx_graphicsPipeline rx_makeGraphicsPipeline(RX_APIDEF rx_GraphicsPipelineDesc* desc);

RX_API rx_graphicsList rx_makeGraphicsList(RX_APIDEF rx_graphicsPipeline pipeline);
RX_API void rx_graphicsListBegin(RX_APIDEF rx_graphicsList list, rx_GraphicsListStateDesc* config);
RX_API void rx_graphicsListEnd(RX_APIDEF rx_graphicsList list);
RX_API void rx_graphicsListSetVertexBuffers(RX_APIDEF rx_graphicsList list, rx_VertexBufferView* buffersViews, rx_u32 length);
RX_API void rx_graphicsListSetIndexBuffer(RX_APIDEF rx_graphicsList list, rx_IndexBufferView* indexbufferView);
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
