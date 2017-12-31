/*
 *  linked_list.h - Definitions for linked lists
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

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#define LINKED_LIST_MAX_LIST_SIZE 10000

#define LINKED_LIST_SUCCESS 0
#define LINKED_LIST_ERR_INDEX_OUT_OF_BOUND_OR_LIST_TOO_LONG 1
#define LINKED_LIST_ERR_OUT_OF_MEMORY 2
#define LINKED_LIST_ERR_LIST_IS_NULL_POINTER 3
#define LINKED_LIST_ERR_NO_DATA_GIVEN 4

typedef struct _LinkedList_ {
	void *data;
	int data_size;
	void *next;
} LinkedList;

LinkedList *linked_list_new(void);
void linked_list_delete(LinkedList *list);
int linked_list_append(LinkedList *list, void *data, int data_size);
int linked_list_insert(LinkedList *list, int index, void *data, int data_size);
int linked_list_set(LinkedList *list, int index, void *data, int data_size);
int linked_list_remove(LinkedList *list, int index);
void *linked_list_get(LinkedList *list, int index, int *data_size);

#endif
