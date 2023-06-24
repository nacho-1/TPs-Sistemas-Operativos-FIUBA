
#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <limits.h> /* for CHAR_BIT */
#include <stdint.h> /* for uint32_t */

#include <stdio.h>

#define BITS_PER_WORD (sizeof(word_t) * CHAR_BIT)
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b) ((b) % BITS_PER_WORD)
#define NWORDS_PER_BLOCK (BLOCK_SIZE / sizeof(word_t))

typedef uint32_t word_t;

typedef struct bitmap {
	word_t *words;
	uint32_t nwords;
} bitmap_t;


void set_bit(bitmap_t *bitmap, int n);

void clear_bit(bitmap_t *bitmap, int n);

int get_bit(bitmap_t *bitmap, int n);

long get_free_bit(bitmap_t *bitmap);

#endif  // _BITMAP_H_
