/*
 *  bitmap.c - Code for creating bitmaps
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

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "bitmap.h"



//-----------------------------------------------------------------------------
///
/// Calculate size of a row of the picture in bytes given a width of the picture
/// in pixel
///
/// @param width     width of the picture in pixel
///
/// @return size of row (in pixel buffer) in bytes (aligned to four bytes)
//
static uint32_t bitmap_pixel_array_row_size(uint32_t width)
{
	uint32_t line_width, line_width_align;
	line_width = width * BITMAP_RGB_COLOR_SIZE;
	line_width_align = line_width;
	if (line_width % BITMAP_ALIGNMENT != 0)
	{
		line_width_align = line_width + BITMAP_ALIGNMENT -
			line_width % BITMAP_ALIGNMENT;
	}

	return line_width_align;
}

//-----------------------------------------------------------------------------
///
/// Calculate size of pixel area in bytes given a width and height of picture
/// in pixel
///
/// @param width     width of the picture in pixel
/// @param height    height of the picture in pixel
///
/// @return size of pixel area in bitmap in bytes (aligned to four bytes)
//
static uint32_t bitmap_pixel_array_size(uint32_t width, uint32_t height)
{
	/* align to BITMAP_BYTES_ALIGNMENT */
	uint32_t line_width_align;
	line_width_align = bitmap_pixel_array_row_size(width);

	return (line_width_align * height);
}

//-----------------------------------------------------------------------------
///
/// Write into pixel buffer
///
/// @param column     column of the pixel to write (between 0 which is leftmost
///                   column and (width - 1) which is the rightmost column)
/// @param row        row of the pixel to write (between 0 which is the top row
///                   and (height - 1) which is the bottom column)
/// @param color      24 bit color data (Byte 0: not used, Byte 1: red,
///                   Byte 2: green, Byte 3: blue with Byte 0 being the most
///                   significant byte)
///
/// @return BITMAP_SUCCESS on success or BITMAP_ERR_NULL_POINTER_PASSED or
///         BITMAP_ERR_WIDTH_HEIGHT_OUT_OF_BOUND otherwise
//
int bitmap_write_pixel(PixelBuffer *pix_buffer,
								uint32_t column, uint32_t row, uint32_t color)
{
	if (pix_buffer == NULL)
	{
		return BITMAP_ERR_NULL_POINTER_PASSED;
	}

	if (pix_buffer->data == NULL)
	{
		return BITMAP_ERR_NULL_POINTER_PASSED;
	}

	if (row >= pix_buffer->height || column >= pix_buffer->width)
	{
		return BITMAP_ERR_WIDTH_HEIGHT_OUT_OF_BOUND;
	}

	/* bitmap is upside down, therefore swap row */
	row = pix_buffer->height - row;

	uint32_t line_width = bitmap_pixel_array_row_size(pix_buffer->width);
	char *data = pix_buffer->data;
	char *pixel = data + row * line_width + column * BITMAP_RGB_COLOR_SIZE;

	if ((pixel + 2) < (data + pix_buffer->data_size)) {
		pixel[0] = color & 0xff; /* red */
		pixel[1] = (color & 0xff00) >> 8; /* green */
		pixel[2] = (color & 0xff0000) >> 16; /* blue */
	}

	return BITMAP_SUCCESS;
}

//-----------------------------------------------------------------------------
///
/// Create a pixel buffer to write the color data in
///
/// @param width     width of the picture in pixel
/// @param height    height of the picture in pixel
///
/// @return pointer to pixel buffer or NULL if memory allocation failed
//
PixelBuffer *bitmap_pixel_buffer_new(uint32_t width, uint32_t height)
{
	/* allocate memory for pixel buffer */
	PixelBuffer *pix_buffer = malloc(sizeof(PixelBuffer));
	if (pix_buffer == NULL)
	{
		return NULL;
	}

	/* set the pixel buffer fields */
	pix_buffer->width = width;
	pix_buffer->height = height;
	pix_buffer->data_size = bitmap_pixel_array_size(width, height);
	pix_buffer->data = malloc(pix_buffer->data_size);
	if (pix_buffer->data == NULL)
	{
		free(pix_buffer);
		return NULL;
	}
	memset(pix_buffer->data, 0, pix_buffer->data_size);

	return pix_buffer;
}

//-----------------------------------------------------------------------------
///
/// Delete a pixel buffer element
///
/// @param pix_buffer  pixel buffer to delete
///
//
void bitmap_pixel_buffer_delete(PixelBuffer *pix_buffer)
{
	/* check wether pix_buffer is NULL */
	if (pix_buffer == NULL)
	{
		return;
	}

	/* free buffer */
	if (pix_buffer->data != NULL)
	{
		free(pix_buffer->data);
	}
	free(pix_buffer);
}

//-----------------------------------------------------------------------------
///
/// Get data array of pixel buffer
///
/// @param pix_buffer    pointer to pixel buffer data structure
/// @param data_size     pointer to integer in which size of the data array will
///                      be stored
///
/// @return address of data array or NULL if pix_buffer does not contain any
///         data
//
char *bitmap_get_pixel_array(PixelBuffer *pix_buffer, int *data_size)
{
	if (pix_buffer == NULL)
	{
		return NULL;
	}

	*data_size = pix_buffer->data_size;
	return pix_buffer->data;
}

//-----------------------------------------------------------------------------
///
/// Create file header for bitmap with inital values
///
/// @param list      pointer to an integer where size of file header will be
///                  stored
///
/// @return address of memory area containing file header or NULL if memory
///         could not be allocated
//
char *bitmap_file_header_new(uint32_t width, uint32_t height, int *data_size)
{
	/* allocate memory for file header */
	BitmapHeader bh;
	memset(&bh, 0, sizeof(BitmapHeader));

	/* set default values for BitmapFileHeader */
	BitmapFileHeader *bmfh = &(bh.bmfh);
	bmfh->bf_type = 0x4d42; /* "BM" for bitmap */
	bmfh->bf_size = BITMAP_HEADER_SIZE + bitmap_pixel_array_size(width, height);
	bmfh->bf_reserved1 = 0;
	bmfh->bf_reserved2 = 0;
	bmfh->bf_off_bits = BITMAP_HEADER_SIZE;

	/* set default values for BitmapInfoHeader */
	BitmapInfoHeader *bmih = &(bh.bmih);
	bmih->bi_size = BITMAP_INFO_HEADER_SIZE;
	bmih->bi_width = width;
	bmih->bi_height = height;
	bmih->bi_planes = 1;
	bmih->bi_bit_count = 24;
	bmih->bi_compression = 0; /* BI_RGB -> not compressed */
	bmih->bi_size_image = 0;
	bmih->bi_x_pels_per_meter = 0;
	bmih->bi_y_pels_per_meter = 0;
	bmih->bi_clr_used = 0;
	bmih->bi_clr_important = 0;

	/* allocate memory for data stream buffer */
	*data_size = BITMAP_HEADER_SIZE;
	char *header = malloc(*data_size);
	if (header == NULL)
	{
		*data_size = 0;
		return NULL;
	}

	/* copy data to data stream buffer */
	/* file header */
	header[0] = (bmfh->bf_type & 0x00ff) >> 0;
	header[1] = (bmfh->bf_type & 0xff00) >> 8;
	header[2] = (bmfh->bf_size & 0x000000ff) >> 0;
	header[3] = (bmfh->bf_size & 0x0000ff00) >> 8;
	header[4] = (bmfh->bf_size & 0x00ff0000) >> 16;
	header[5] = (bmfh->bf_size & 0xff000000) >> 24;
	header[6] = (bmfh->bf_reserved1 & 0x00ff) >> 0;
	header[7] = (bmfh->bf_reserved1 & 0xff00) >> 8;
	header[8] = (bmfh->bf_reserved2 & 0x00ff) >> 0;
	header[9] = (bmfh->bf_reserved2 & 0xff00) >> 8;
	header[10] = (bmfh->bf_off_bits & 0x000000ff) >> 0;
	header[11] = (bmfh->bf_off_bits & 0x0000ff00) >> 8;
	header[12] = (bmfh->bf_off_bits & 0x00ff0000) >> 16;
	header[13] = (bmfh->bf_off_bits & 0xff000000) >> 24;
	/* info header */
	header[14] = (bmih->bi_size & 0x000000ff) >> 0;
	header[15] = (bmih->bi_size & 0x0000ff00) >> 8;
	header[16] = (bmih->bi_size & 0x00ff0000) >> 16;
	header[17] = (bmih->bi_size & 0xff000000) >> 24;
	header[18] = (bmih->bi_width & 0x000000ff) >> 0;
	header[19] = (bmih->bi_width & 0x0000ff00) >> 8;
	header[20] = (bmih->bi_width & 0x00ff0000) >> 16;
	header[21] = (bmih->bi_width & 0xff000000) >> 24;
	header[22] = (bmih->bi_height & 0x000000ff) >> 0;
	header[23] = (bmih->bi_height & 0x0000ff00) >> 8;
	header[24] = (bmih->bi_height & 0x00ff0000) >> 16;
	header[25] = (bmih->bi_height & 0xff000000) >> 24;
	header[26] = (bmih->bi_planes & 0x00ff) >> 0;
	header[27] = (bmih->bi_planes & 0xff00) >> 8;
	header[28] = (bmih->bi_bit_count & 0x00ff) >> 0;
	header[29] = (bmih->bi_bit_count & 0xff00) >> 8;
	header[30] = (bmih->bi_compression & 0x000000ff) >> 0;
	header[31] = (bmih->bi_compression & 0x0000ff00) >> 8;
	header[32] = (bmih->bi_compression & 0x00ff0000) >> 16;
	header[33] = (bmih->bi_compression & 0xff000000) >> 24;
	header[34] = (bmih->bi_size_image & 0x000000ff) >> 0;
	header[35] = (bmih->bi_size_image & 0x0000ff00) >> 8;
	header[36] = (bmih->bi_size_image & 0x00ff0000) >> 16;
	header[37] = (bmih->bi_size_image & 0xff000000) >> 24;
	header[38] = (bmih->bi_x_pels_per_meter & 0x000000ff) >> 0;
	header[39] = (bmih->bi_x_pels_per_meter & 0x0000ff00) >> 8;
	header[40] = (bmih->bi_x_pels_per_meter & 0x00ff0000) >> 16;
	header[41] = (bmih->bi_x_pels_per_meter & 0xff000000) >> 24;
	header[42] = (bmih->bi_y_pels_per_meter & 0x000000ff) >> 0;
	header[43] = (bmih->bi_y_pels_per_meter & 0x0000ff00) >> 8;
	header[44] = (bmih->bi_y_pels_per_meter & 0x00ff0000) >> 16;
	header[45] = (bmih->bi_y_pels_per_meter & 0xff000000) >> 24;
	header[46] = (bmih->bi_clr_used & 0x000000ff) >> 0;
	header[47] = (bmih->bi_clr_used & 0x0000ff00) >> 8;
	header[48] = (bmih->bi_clr_used & 0x00ff0000) >> 16;
	header[49] = (bmih->bi_clr_used & 0xff000000) >> 24;
	header[50] = (bmih->bi_clr_important & 0x000000ff) >> 0;
	header[51] = (bmih->bi_clr_important & 0x0000ff00) >> 8;
	header[52] = (bmih->bi_clr_important & 0x00ff0000) >> 16;
	header[53] = (bmih->bi_clr_important & 0xff000000) >> 24;

	return header;
}

//-----------------------------------------------------------------------------
///
/// Deletes a file header array created by bitmap_file_header_new
///
/// @param file_header  pointer to file header array
///
//
void bitmap_file_header_delete(char *file_header)
{
 free(file_header);
}
