#include "rx_internal.h"

typedef struct rx_DX12Context rx_DX12Context;

RX_API void* rx_dx12CreateDevice(int useWarp);
RX_API rx_errorCode rx_dx12CreateContext(rx_Context* ctx, const rx_Desc* desc);
RX_API void rx_dx12Commit(rx_Context* ctx);
// RX_INTERN rx_errorCode rx_dx12CreateSwapChain(rx_Context* ctx, rx_u32 width, rx_u32 height);
RX_API rx_errorCode rx_dx12EnableDebugLayer(void);
RX_API char const* rx_dx12ErrorText(rx_errorCode error);
RX_API void rx_dx12GraphicsListBegin(rx_Context* ctx, rx_graphicsList list, rx_GraphicsListStateDesc* config);
RX_API rx_buffer rx_dx12MakeBuffer(rx_Context* ctx, const rx_BufferDesc* desc);
RX_API rx_graphicsPipeline rx_dx12MakeGraphicsPipeline(rx_Context* ctx, rx_GraphicsPipelineDesc* desc);
RX_API rx_graphicsList rx_dx12MakeGraphicsList(rx_Context* ctx, const rx_graphicsPipeline pipeline);