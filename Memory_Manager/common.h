/*
 * Binghamton CS 451/551 Project "Memory manager".
 * This file needs to be turned in.	
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#define BIT_PER_BYTE 8

#define BITMAP_OP_SUCCEED   0
#define BITMAP_OP_NOT_FOUND -1
#define BITMAP_OP_ERROR     -2

int bitmap_find_first_bit(unsigned char * bitmap, int size, int val);
int bitmap_set_bit(unsigned char * bitmap, int size, int target_pos);
int bitmap_clear_bit(unsigned char * bitmap, int size, int target_pos);
int bitmap_bit_is_set(unsigned char * bitmap, int size, int pos);
int bitmap_print_bitmap(unsigned char * bitmap, int size);

#endif //__COMMON_H__
