#ifndef _RX_INTERNAL_
#define _RX_INTERNAL_

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "rx_renderer.h"

#define RX_FRAMES 3

#ifndef RX_INTERN
#define RX_INTERN static
#endif

#ifndef RX_INLINE
#define RX_INLINE static inline
#endif

#define rx_null ((void*)0)
#define rx_true 1
#define rx_false 0
//typedef int rx_b32;
//typedef uint32_t rx_u32;
#define RX_ERR_PRINT(STR, ...) printf("Error: %s:%d: " STR, __FILE__, __LINE__, __VA_ARGS__)
#define RX_ERR_PRINTS(STR) printf("Error: %s:%d: " STR, __FILE__, __LINE__)

#define RX_ASSERT(r) assert(r)

#ifdef RX_VK_ENABLED
struct rx_VKContext;
#endif


#ifdef RX_DX12_ENABLED
struct rx_DXContext;
#endif

struct rx_Context {
	/* meta data */
    char applicationName[256];
	int applicationVersion;
    char engineName[256];
	int engineVersion;
	rx_renderBackend backend;
	/* vulkan */
#ifdef RX_VK_ENABLED
	struct rx_VKContext *vk;
#endif
	/* DX12 */
#ifdef RX_DX12_ENABLED
	struct rx_DX12Context *dx12;
#endif
	/* feature state */
	rx_b32 useVsync;
	/* size */
	int width;
	int height;
	/* debug */
	rx_b32 debug;
	/* error handling */
	char* errorText;
	rx_errorCode error;
};
#endif