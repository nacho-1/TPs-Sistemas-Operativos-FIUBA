#include "bitmap.h"

void
set_bit(bitmap_t *bitmap, int n)
{
	bitmap->words[WORD_OFFSET(n)] |= (1 << BIT_OFFSET(n));
}

void
clear_bit(bitmap_t *bitmap, int n)
{
	bitmap->words[WORD_OFFSET(n)] &= ~(1 << BIT_OFFSET(n));
}

int
get_bit(bitmap_t *bitmap, int n)
{
	word_t bit = bitmap->words[WORD_OFFSET(n)] & (1 << BIT_OFFSET(n));
	return bit != 0;
}

long
get_free_bit(bitmap_t *bitmap)
{
	uint32_t bitno;
	for (uint32_t i = 0; i < bitmap->nwords; i++) {
		for (uint32_t j = 0; j < BITS_PER_WORD; j++) {
			bitno = j + (i * BITS_PER_WORD);
			if (get_bit(bitmap, bitno) == 0) {
				printf("[debug] get_free_bit - First free bit "
				       "in position: "
				       "%d\n",
				       bitno);
				return bitno;
			}
		}
	}

	printf("[debug] get_free_bit - No free bit found\n");

	return -1;
}
