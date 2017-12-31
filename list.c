/*
 *  list.c - Code for array list structures
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

#include "list.h"

//-----------------------------------------------------------------------------
///
/// Creates a new list
///
/// @param element_size size of elements inside the list
///
/// @return pointer to the newly created list structure, NULL if failed to
///         allocate memory
//
List *list_new(int element_size)
{
	/* allocate memory for list struct */
	List *list = malloc(sizeof(List));
	if (list == NULL)
	{
		return NULL;
	}
	memset(list, 0, sizeof(List));

	/* set the elements in the list struct to initial values */
	list->element_size = element_size;
	list->length = 0;
	list->mem_size = element_size * LIST_STANDARD_LENGTH;

	/* allocate the list memory */
	list->mem = malloc(list->mem_size);
	if (list->mem == NULL)
	{
		return NULL;
	}
	memset(list->mem, 0, list->mem_size);
	return list;
}

//-----------------------------------------------------------------------------
///
/// Free the memory of a list
///
/// @param list pointer to the list structure that should be freed
///
//
void list_delete(List *list)
{
	if (list != NULL)
	{
		free(list->mem);
		free(list);
	}
}

//-----------------------------------------------------------------------------
///
/// Insert an element at the end of the list
///
/// @param list    pointer to the list
/// @param element pointer to the element that should be inserted
///
/// @return LIST_SUCCESS on success, LIST_ERR_INDEX_OUT_OF_BOUND or
///         LIST_ERR_OUT_OF_MEMORY or LIST_ERR_NULL_POINTER (if list- or
///         element-parameter is NULL) otherwise
//
int list_append(List *list, void *element)
{
	if (list == NULL)
	{
		return LIST_ERR_NULL_POINTER;
	}
	return list_insert(list, list->length, element);
}

//-----------------------------------------------------------------------------
///
/// Remove an element from the list
///
/// @param list  pointer to the list
/// @param index index of the element to remove starting with 0 for first element
///              index must be greater than or equal to zero and smaller than the
///              length of the list
///
/// @return LIST_SUCCESS on success, LIST_ERR_INDEX_OUT_OF_BOUND otherwise
//
int list_remove(List *list, int index)
{
	/* check whether list is NULL */
	if (list == NULL)
	{
		return LIST_ERR_NULL_POINTER;
	}

	void *mem = list->mem;
	int element_size = list->element_size;

	/* check index bounds */
	if (index < 0 || index >= list->length)
	{
		return LIST_ERR_INDEX_OUT_OF_BOUND;
	}

	/* overwrite removed element and move up subsequent elements */
	for (int i = index; i < list->length; i++)
	{
		void *smaller_addr = mem + element_size * i;
		void *bigger_addr = mem + element_size  * (i + 1);
		memcpy(smaller_addr, bigger_addr, element_size);
	}
	list->length--;
	return LIST_SUCCESS;
}

//-----------------------------------------------------------------------------
///
/// Insert an element, subsequent elements will be moved accordingly
///
/// @param list    pointer to the list
/// @param index   index of the element to remove starting with 0 for first
///                element, index must be greater than or equal to zero and
///                smaller than or equal to the length of the list
/// @param element pointer to the element to be inserted
///
/// @return LIST_SUCCESS on success, LIST_ERR_INDEX_OUT_OF_BOUND or
///         LIST_ERR_OUT_OF_MEMORY or LIST_ERR_NULL_POINTER (if list- or
///         element-parameter is NULL) otherwise
//
int list_insert(List *list, int index, void *element)
{
	/* check whether list is NULL */
	if (list == NULL)
	{
		return LIST_ERR_NULL_POINTER;
	}

	int element_size = list->element_size;

	/* check whether list or element is NULL */
	if (list == NULL || element == NULL)
	{
		return LIST_ERR_NULL_POINTER;
	}

	/* check index bounds */
	if (index < 0 || index > list->length)
	{
		return LIST_ERR_INDEX_OUT_OF_BOUND;
	}

	/* check if there is enough memory left */
	if (list->mem_size < (list->length + 1) * element_size)
	{
		/* reallocate more memory for list */
		void *mem_tmp = realloc(
			list->mem,
			(list->length + LIST_STANDARD_REALLOC_LENGTH) * element_size);
		if (mem_tmp == NULL)
		{
			return LIST_ERR_OUT_OF_MEMORY;
		}
		list->mem = mem_tmp;
	}

	/* move up the elements to make space for element to insert */
	for (int i = list->length; i > index; i--)
	{
		void *bigger_addr = list->mem + i * element_size;
		void *smaller_addr = list->mem + (i - 1) * element_size;
		memcpy(bigger_addr, smaller_addr, element_size);
	}

	/* insert element */
	void *index_addr = list->mem + index * element_size;
	memcpy(index_addr, element, element_size);

	list->length++;
	return LIST_SUCCESS;
}

//-----------------------------------------------------------------------------
///
/// Overwrites an element in the list by another element
///
/// @param list    pointer to the list
/// @param index   index of the element to read
///                index must be greater than or equal to zero and smaller than
///                or equal to the length of the list
/// @param element pointer to the element that should be written to the index in
///                the list
///
/// @return LIST_SUCCESS on success, LIST_ERR_INDEX_OUT_OF_BOUND or
///         LIST_ERR_NULL_POINTER (if list- or element-parameter is NULL)
///         otherwise
//
int list_set(List *list, int index, void *element)
{
	/* check whether list or element is NULL */
	if (list == NULL || element == NULL)
	{
		return LIST_ERR_NULL_POINTER;
	}

	/* check index bounds */
	if (index < list->length)
	{
		return LIST_ERR_INDEX_OUT_OF_BOUND;
	}

	/* overwrite element in the list */
	void *element_address = list->mem + index * list->element_size;
	memcpy(element_address, element, list->element_size);

	return LIST_SUCCESS;
}

//-----------------------------------------------------------------------------
///
/// Read an element from the list
///
/// @param list    pointer to the list
/// @param index   index of the element to read
///                index must be greater than or equal to zero and smaller than
///                or equal to the length of the list
///
/// @return pointer to element, NULL if index was out of bound
//
void *list_get(List *list, int index)
{
	/* check whether list is NULL */
	if (list == NULL)
	{
		return NULL;
	}

	/* check index bound */
	if (index >= list->length)
	{
		return NULL;
	}

	return (list->mem + index * list->element_size);
}
