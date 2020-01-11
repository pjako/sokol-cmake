#ifndef _RX_INTERNAL_
#define _RX_INTERNAL_

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "rx_renderer.h"

#define RX_FRAMES 3

#ifndef RX_FORCE_INLINE
#define RX_FORCE_INLINE __forceinline
#endif

#ifndef RX_INTERN
#define RX_INTERN static
#endif

#ifndef RX_INLINE
#define RX_INLINE static inline
#endif


#ifndef RX_UNREACHABLE
    #define RX_UNREACHABLE RX_ASSERT(false)
#endif


typedef uint16_t rx_u16;

#define rx_null ((void*)0)
#define rx_true 1
#define rx_false 0
//typedef int rx_b32;
//typedef uint32_t rx_u32;
#define RX_ERR_PRINT(STR, ...) printf("Error: %s:%d: " STR, __FILE__, __LINE__, __VA_ARGS__)
#define RX_ERR_PRINTS(STR) printf("Error: %s:%d: " STR, __FILE__, __LINE__)


#define RX_LOG(s) { RX_ASSERT(s); puts(s); }

#define RX_ASSERT(r) assert(r)

#define RX_MAX(A, B) (A > B ? A : B)

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

typedef struct rx_id {
	union {
		rx_u32 index;
		rx_u32 generation;
	};
	rx_u64 id;
} rx_id;

RX_FORCE_INLINE rx_u32 rx__vertexFormatSize(rx_vertexFormat format) {
	switch (format) {
		case rx_vertexFormat_s8x4:
		case rx_vertexFormat_s8x4N:
		case rx_vertexFormat_u8x4:
		case rx_vertexFormat_u8x4N:
		case rx_vertexFormat_s16x2:
		case rx_vertexFormat_s16x2N:
		case rx_vertexFormat_f32x1:
		case rx_vertexFormat_u10x3U2x1N:
			return 4;
		case rx_vertexFormat_s16x4:
		case rx_vertexFormat_s16x4N:
		case rx_vertexFormat_u16x4N:
		case rx_vertexFormat_f32x2:
		case rx_vertexFormat_f16x4:
			return 8;
		case rx_vertexFormat_f32x3:
			return 12;
		case rx_vertexFormat_f32x4:
			return 16;
	}
	return 0;
}

#endif /* _RX_INTERNAL_ */