/*
 *  main.c - Entry point for program
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "linked_list.h"
#include "parse.h"
#include "bitmap.h"
#include "main.h"
#include "draw.h"

const char *err_msg_usage =
	"Usage: ./bitmap <input> <output> <width> <height>\n";
const char *err_msg_read_input =
	"Error: could not read input file \"%s\".\n";
const char *err_msg_invalid_input =
	"Error: invalid entry on line %d.\n";
const char *err_msg_duplicate_id =
	"Error: duplicate ID \"%d\".\n";
const char *err_msg_write_file =
	"Error: could not write file \"%s\".\n";
const char *err_msg_out_of_mem =
	"Error: out of memory.\n";
const char *err_msg_unrecognised =
	"Error: Unrecognised error.\n";

int main(int argc, char *argv[])
{
	int ret;

	/* check whether there is a correct number of arguments */
	if (argc != 5) {
		printf(err_msg_usage);
		exit(ERR_USAGE);
	}

	/* parsing arguments */
	char *input_path = argv[1];
	char *output_path = argv[2];

	char *endptr;
	int width = strtol(argv[3], &endptr, 10);
	if (*endptr != 0 || width < 0)
	{
		/* the image width is not a number */
		printf(err_msg_usage);
		exit(ERR_USAGE);
	}
	int height = strtol(argv[4], &endptr, 10);
	if (*endptr != 0 || height < 0)
	{
		/* the image width is not a number */
		printf(err_msg_usage);
		exit(ERR_USAGE);
	}

	/* parse input file */
	LinkedList *command_list;
	ret = parse_file(input_path, &command_list);
	if (ret != SUCCESS)
	{
		exit(ret);
	}

	/* create pixel buffer */
	PixelBuffer *pix_buffer;
	pix_buffer = bitmap_pixel_buffer_new(width, height);
	if (pix_buffer == NULL)
	{
		printf(err_msg_out_of_mem);
		ret = ERR_OUT_OF_MEM;
		goto main_cleanup1;
	}

	/* set pixel buffer white */
	Command comm_white;
	Rectangle rect_white;
	comm_white.shape = SH_RECTANGLE;
	comm_white.obj = &rect_white;
	rect_white.x = 0;
	rect_white.y = 0;
	rect_white.width = width;
	rect_white.height = height;
	rect_white.color = 0xffffff;
	if (draw_command(pix_buffer, &comm_white) != DRAW_SUCCESS)
	{
		printf(err_msg_unrecognised);
		ret = ERR_UNRECOGNISED;
		goto main_cleanup2;
	}

	/* draw commands */
	int index_command = 0;
	int data_size;
	Command *comm = linked_list_get(command_list, index_command, &data_size);

	while (comm != NULL)
	{
		if (data_size != sizeof(Command))
		{
			printf(err_msg_unrecognised);
			ret = ERR_UNRECOGNISED;
			goto main_cleanup2;
		}
		if (draw_command(pix_buffer, comm) != DRAW_SUCCESS)
		{
			printf(err_msg_unrecognised);
			ret = ERR_UNRECOGNISED;
			goto  main_cleanup2;
		}

		index_command++;
		comm = linked_list_get(command_list, index_command, &data_size);
	}

	/* write output file */
	FILE *of = fopen(output_path, "w");
	if (of == NULL)
	{
		printf(err_msg_write_file, output_path);
		ret = ERR_WRITE_FILE;
		goto main_cleanup2;
	}

	/* write file header */
	int header_size = 0;
	char *file_header = bitmap_file_header_new(width, height, &header_size);
	if (file_header == NULL)
	{
		printf(err_msg_out_of_mem);
		ret = ERR_OUT_OF_MEM;
		goto main_cleanup3;
	}
	ret = fwrite(file_header, 1, header_size, of);
	if (ret != header_size)
	{
		printf(err_msg_write_file, output_path);
		ret = ERR_WRITE_FILE;
		goto main_cleanup4;
	}

	/* write pixel array from pixel buffer to bitmap file */
	int pixel_array_size;
	char *pixel_array = bitmap_get_pixel_array(pix_buffer, &pixel_array_size);
	if (pixel_array == NULL)
	{
		printf(err_msg_out_of_mem);
		ret = ERR_OUT_OF_MEM;
		goto main_cleanup4;
	}
	ret = fwrite(pixel_array, 1, pixel_array_size, of);
	if (ret != pixel_array_size)
	{
		printf(err_msg_write_file, output_path);
		ret = ERR_WRITE_FILE;
		goto main_cleanup4;
	}

	ret = SUCCESS;

main_cleanup4:
	/* delete file header */
	bitmap_file_header_delete(file_header);
main_cleanup3:
	/* close output file */
	fclose(of);
main_cleanup2:
	/* delete pixel buffer */
	bitmap_pixel_buffer_delete(pix_buffer);
main_cleanup1:
	/* delete command list */
	parse_delete_command_list(command_list);
	return ret;
}
