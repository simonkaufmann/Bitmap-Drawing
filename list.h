/*
 *  list.h - Definitions for array list structures
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

#ifndef LIST_H
#define LIST_H

#define LIST_STANDARD_LENGTH 20
#define LIST_STANDARD_REALLOC_LENGTH 20

#define LIST_ERR_INDEX_OUT_OF_BOUND 1
#define LIST_ERR_OUT_OF_MEMORY 2
#define LIST_ERR_NULL_POINTER 3
#define LIST_SUCCESS 0

typedef struct _List_ {
	int element_size;
	int length;        /* how many elements are saved in the list */
	int mem_size;      /* size of allocated memory in bytes */
	void *mem;
} List;

List *list_new(int element_size);
void list_delete(List *list);
int list_append(List *list, void *element);
int list_remove(List *list, int index);
int list_insert(List *list, int index, void *element);
int list_set(List *list, int index, void *element);
void *list_get(List *list, int index);

#endif
