/*
 *  parse.h - Definitions for parsing the input file
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

#ifndef PARSE_H
#define PARSE_H

#include <stdint.h>

#include "linked_list.h"

#define PARSE_SUCCESS 0
#define PARSE_ERR_OUT_OF_MEM 1
#define PARSE_ERR_INDEX_OUT_OF_BOUND 2
#define PARSE_ERR_LIST 3
#define PARSE_ERR_PASSED_NULL_POINTER 4
#define PARSE_ERR_PROPERTY_NAME_TOO_LONG 5
#define PARSE_ERR_INVALID_INPUT 6
#define PARSE_ERR_EOF 7

#define PARSE_MAX_PROPERTY_NAME_LENGTH 20
#define PARSE_MAX_VALUE_STRING_LENGTH 400

typedef uint32_t id_t;

typedef enum _Shape_ {SH_RECTANGLE, SH_CIRCLE, SH_TRIANGLE} Shape;

typedef struct _Rectangle_ {
	id_t id;
	int color;
	int x;
	int y;
	int width;
	int height;
} Rectangle;

typedef struct _Circle_ {
	id_t id;
	int color;
	int x;
	int y;
	int radius;
} Circle;

typedef struct _Triangle_ {
	id_t id;
	int color;
	int ax;
	int ay;
	int bx;
	int by;
	int cx;
	int cy;
} Triangle;

/*
 * element contains information about which object it is and the union
 * with the data itself
 */
typedef struct _Command_ {
	Shape shape;
	id_t id;
	void *obj;
} Command;

int parse_line(char *line, Command **com);
int parse_file(char *input_path, LinkedList **list);
void parse_delete_command_list(LinkedList *command_list);


#endif
