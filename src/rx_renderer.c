#include "rx_renderer.h"
#include "rx_internal.h"
#include <string.h>
#include <stdlib.h> /* malloc */

#define RX_STATIC_ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define RX_ERR_COND(condition, error) { if (condition) return error; }
#define RX_ERR_FAIL(error) { return error; }
#define RX_ERROR(error) { return error; }
#define RX_ERROR_EXPLAIN(TEXT) { ctx->errorText = TEXT; }

#ifdef VK_USE_PLATFORM_IOS_MVK
#define RX_SYSTEM_SURFACE_EXTENSION VK_MVK_IOS_SURFACE_EXTENSION_NAME
#elif VK_USE_PLATFORM_MACOS_MVK
#define RX_SYSTEM_SURFACE_EXTENSION VK_MVK_MACOS_SURFACE_EXTENSION_NAME
#elif VK_USE_PLATFORM_WIN32_KHR
#define RX_SYSTEM_SURFACE_EXTENSION VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif VK_USE_PLATFORM_ANDROID_KHR
#define RX_SYSTEM_SURFACE_EXTENSION VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
#endif



#define MAX_EXTENSIONS 128
#define MAX_LAYERS 64
#define FRAME_LAG 2
#define RX_MAX_WINDOW_COUNT 16

#define _RX_INTERNAL_IMPLEMENTATION_
#ifdef RX_VK_ENABLED
#include "rx_vkrenderer.h"
#endif
#ifdef RX_DX12_ENABLED
#include "rx_dx12.h"
#endif
#undef _RX_INTERNAL_IMPLEMENTATION_

rx_Context* rx_createContext(const rx_Desc* desc) {
	rx_Context* ctx = malloc(sizeof(rx_Context));
	if (!ctx) {
		return rx_null;
	}
	*ctx = (rx_Context) {
		.backend = desc->renderBackend,
		.debug = desc->debug,
		.useVsync = !desc->vsyncDeactivated,
	};

	switch (desc->renderBackend) {
	#ifdef RX_DX12_ENABLED
		case rx_renderBackend_DX12: {
			if (rx_dx12CreateContext(ctx, desc) != rx_errorCode_ok) {
				free(ctx);
				return rx_null;
			}
			break;
		}
	#endif
	#ifdef RX_VK_ENABLED
		case rx_renderBackend_VK: {
			if (rx_vkCreateContext(ctx, desc) != rx_errorCode_ok) {
				free(ctx);
				return rx_null;
			}
			break;
		}
	#endif
	}
	return ctx;
}

void rx_commit(rx_Context* ctx) {
	switch (ctx->backend) {
	#ifdef RX_DX12_ENABLED
		case rx_renderBackend_DX12: {
			rx_dx12Commit(ctx);
			break;
		}
	#endif
	#ifdef RX_VK_ENABLED
		case rx_renderBackend_VK: {
			if (rx_vkCommit(ctx, desc) != rx_errorCode_ok) {
				free(ctx);
				return rx_null;
			}
			break;
		}
	#endif
	}

}


rx_graphicsPipeline rx_createGraphicsPipeline(RX_APIDEF rx_GraphicsPipelineDesc* desc) {
	switch (ctx->backend) {
#ifdef RX_DX12_ENABLED
		case rx_renderBackend_DX12: {
			return rx_dx12CreateGraphicsPipeline(ctx, desc);
		}
#endif
#ifdef RX_VK_ENABLED
		case rx_renderBackend_VK: {
			return rx_vkCreateGraphicsPipeline(ctx, desc);
		}
#endif
	}
}