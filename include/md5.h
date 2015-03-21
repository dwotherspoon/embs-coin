#include <stdint.h>

typedef struct {
	uint8_t diff;
	uint32_t msg[32];
} MD5_ctx;

uint8_t md5_hash(MD5_ctx * ctx);
void md5_setup(MD5_ctx * , uint32_t start, uint8_t diff, uint32_t msg[15]);

#define ENDSWAP(x) (((x) & 0xFF000000) >> 24 | ((x) & 0xFF0000) >> 8 | ((x) & 0xFF00) << 8 | ((x) & 0xFF) << 24)
