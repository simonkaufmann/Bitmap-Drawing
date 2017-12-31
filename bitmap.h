/*
 *  bitmap.h - Definitions for creating bitmaps
 *  Copyright (C) 2017  Simon Kaufmann, HeKa
 *
 *  This file is part of Bitmap Drawing.
 *
 *  Bitmap Drawing is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Bitmap Drawing is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>

#define BITMAP_SUCCESS 0
#define BITMAP_ERR_NULL_POINTER_PASSED 1
#define BITMAP_ERR_WIDTH_HEIGHT_OUT_OF_BOUND 2

#define BITMAP_HEADER_SIZE (BITMAP_FILE_HEADER_SIZE + BITMAP_INFO_HEADER_SIZE)
#define BITMAP_FILE_HEADER_SIZE 14
#define BITMAP_INFO_HEADER_SIZE 40

#define BITMAP_RGB_COLOR_SIZE 3
#define BITMAP_ALIGNMENT 4

typedef struct _PixelBuffer_ {
	char *data;
	int data_size;
	uint32_t width;
	uint32_t height;
} PixelBuffer;

typedef struct _BitmapFileHeader_ {
	uint16_t bf_type;
	uint32_t bf_size;
	uint16_t bf_reserved1;
	uint16_t bf_reserved2;
	uint32_t bf_off_bits;
} BitmapFileHeader;

typedef struct _BitmapInfoHeader_ {
	uint32_t bi_size;
	int32_t bi_width;
	int32_t bi_height;
	uint16_t bi_planes;
	uint16_t bi_bit_count;
	uint32_t bi_compression;
	uint32_t bi_size_image;
	int32_t bi_x_pels_per_meter;
	int32_t bi_y_pels_per_meter;
	uint32_t bi_clr_used;
	uint32_t bi_clr_important;
} BitmapInfoHeader;

typedef struct _BitmapHeader_ {
	BitmapFileHeader bmfh;
	BitmapInfoHeader bmih;
} BitmapHeader;

char *bitmap_file_header_new(uint32_t width, uint32_t height, int *data_size);
void bitmap_file_header_delete(char *file_header);

PixelBuffer *bitmap_pixel_buffer_new(uint32_t width, uint32_t height);
void bitmap_pixel_buffer_delete(PixelBuffer *pix_buffer);
int bitmap_write_pixel(PixelBuffer *pix_buffer,
							  uint32_t column, uint32_t row, uint32_t color);

char *bitmap_get_pixel_array(PixelBuffer *pix_buffer, int *data_size);

#endif
