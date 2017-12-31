/*
 *  linked_list.c - Functions to create linked list structures
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

#include "linked_list.h"

//-----------------------------------------------------------------------------
///
/// Find item corresponding to specific index of a list
///
/// @param list  pointer to a list
/// @param index index of element starting from 0 for first element
///              if index is set to -1, then the last element of the list is
///              returned
///
/// @return pointer to linked_list structure, NULL if list is not long enough
///         for given index or if list is longer than LINKED_LIST_MAX_LIST_SIZE
//
static LinkedList *linked_list_find_item(LinkedList *list, int index)
{
	/* check if list is null pointer */
	if (list == NULL)
	{
		return NULL;
	}

	if (list->next == NULL)
	{
		if (index < 0)
		{
			/*
			 * If looking for last element, then return the initial list
			 * structure (although this is not really an element of the list
			 * since the list is still empty)
			 */
			return list;
		}
		return NULL;
	}

	/* search for last element in list */
	LinkedList *l = list->next; /* first element of the list is element pointed
	                             * to by list->next
	                             */
	int i = index;
	if (index < 0)
	{
		i = LINKED_LIST_MAX_LIST_SIZE;
	}
	while (l->next != NULL && i > 0)
	{
		l = l->next;
		i--;
	}

	/* if index is larger than size of the list, then return NULL pointer */
	if (index >= 0 && i != 0)
	{
		return NULL;
	}

	return l;
}

//-----------------------------------------------------------------------------
///
/// Find last item of a list
///
/// @param list  pointer to a list
///
/// @return pointer to linked_list structure, NULL if list is longer than
///         LINKED_LIST_MAX_LIST_SIZE
//
static LinkedList *linked_list_find_last_item(LinkedList *list)
{
	return linked_list_find_item(list, -1);
}

//-----------------------------------------------------------------------------
///
/// Copy data to a list item
///
/// @param list      pointer to the list item which shall hold the given data
/// @param data      pointer to data that should be copied
/// @param data_size size of data_area that should be copied in bytes
///
/// @return LINKED_LIST_SUCCESS if successful, LINKED_LIST_ERR_OUT_OF_MEMORY
///         or LINKED_LIST_ERR_LIST_IS_NULL_POINTER or
///         LINKED_LIST_ERR_NO_DATA_GIVEN otherwise
//
static int linked_list_set_this_item(LinkedList *list, void *data,
                                     int data_size)
{
	/* check whether given list is null pointer */
	if (list == NULL)
	{
		return LINKED_LIST_ERR_LIST_IS_NULL_POINTER;
	}

	/* check whether data for the element was given */
	if (data != NULL)
	{
		/* if data was given, then allocate memory and copy data */
		void *new_data = malloc(data_size);
		if (new_data == NULL)
		{
			return LINKED_LIST_ERR_OUT_OF_MEMORY;
		}
		memcpy(new_data, data, data_size);

		/* if there is already data in the element, free this data */
		if (list->data != NULL)
		{
			free(list->data);
		}

		/* add newly created data element to newly created data area */
		list->data = new_data;
		list->data_size = data_size;
	}
	else
	{
		return LINKED_LIST_ERR_NO_DATA_GIVEN;
	}

	return LINKED_LIST_SUCCESS;
}

//-----------------------------------------------------------------------------
///
/// Create a new linked list, the first element of this list will contain no
/// data and can be set with linked_list_set()
///
/// @return pointer to linked_list structure, NULL if memory could not be
///         allocated
//
LinkedList *linked_list_new()
{
	/* allocate memory for data structure */
	LinkedList *list = malloc(sizeof(LinkedList));
	if (list == NULL)
	{
		return NULL;
	}
	memset(list, 0, sizeof(LinkedList));

	list->data = NULL;
	list->data_size = 0;
	list->next = NULL;

	return list;
}

//-----------------------------------------------------------------------------
///
/// Create a new linked list, the first element of this list will contain no
/// data and can be set with linked_list_set()
///
/// @return pointer to linked_list structure, NULL if memory could not be
///         allocated
//
void linked_list_delete(LinkedList *list)
{
	if (list == NULL)
	{
		return;
	}

	/* free all data areas and elements of the list */
	LinkedList *next_element;
	int i = LINKED_LIST_MAX_LIST_SIZE + 2; /* +2 because list-pointer has to be
											* freed, but does not count towards
											* length of list, and also list can
											* be made 1 element longer than
											* LINKED_LIST_MAX_LIST_SIZE by using
											* append
										    */
	next_element = list->next;
	while (next_element != NULL && i > 0)
	{
		if (next_element->data != NULL)
		{
			free(next_element->data);
		}
		void *temp = next_element->next;
		free(next_element);
		next_element = temp;
		i--;
	}

}


//-----------------------------------------------------------------------------
///
/// Append an element at the end of the list
///
/// @param list      pointer to the list
/// @param data      pointer to memory of data that should be copied into the
///                  list, the data will be copied to list, therefore data
///                  might have to be freed after calling linked_list_append
/// @param data_size size of the data area to be copied in bytes
///
/// @return LINKED_LIST_SUCCESS if successful,
///         LINKED_LIST_ERR_LIST_IS_NULL_POINTER or
///         LINKED_LIST_ERR_INDEX_OUT_OF_BOUND_OR_LIST_TOO_LONG or
///         LINKED_LIST_ERR_OUT_OF_MEMORY or LINKED_LIST_ERR_NO_DATA_GIVEN
///         otherwise
//
int linked_list_append(LinkedList *list, void *data, int data_size)
{
	int ret;

	/* check whether data is NULL */
	if (data == NULL)
	{
		return LINKED_LIST_ERR_NO_DATA_GIVEN;
	}

	/* check whether parameter is null pointer */
	if (list == NULL)
	{
		return LINKED_LIST_ERR_LIST_IS_NULL_POINTER;
	}

	/* search for last element in list */
	LinkedList *l = linked_list_find_last_item(list);
	if (l == NULL)
	{
		/* return NULL if list is too long */
		return LINKED_LIST_ERR_INDEX_OUT_OF_BOUND_OR_LIST_TOO_LONG;
	}

	/* create new list element */
	LinkedList *new_list = linked_list_new();
	if (new_list == NULL)
	{
		return LINKED_LIST_ERR_OUT_OF_MEMORY;
	}

	/* check whether data for the element was given */
	ret = linked_list_set_this_item(new_list, data, data_size);
	if (ret != LINKED_LIST_SUCCESS)
	{
		return ret;
	}

	new_list->next = NULL;

	/* last element of list shall point to the newly created item */
	l->next = new_list;

	return LINKED_LIST_SUCCESS;
}

//-----------------------------------------------------------------------------
///
/// Insert an element to the list
///
/// @param list      pointer to the list
/// @param index     index of the element that will be inserted
///                  element at this index before insertion will have
///                  (index + 1) as index afterwards
/// @param data      pointer to data area that will be copied to new element,
///                  the data will be copied to list, therefore data
///                  might have to be freed after calling linked_list_append
/// @param data_size size of data area that will be copied in bytes
///
/// @return LINKED_LIST_SUCCESS if successful,
///         LINKED_LIST_ERR_LIST_IS_NULL_POINTER or
///         LINKED_LIST_ERR_OUT_OF_MEMORY or
///         LINKED_LIST_ERR_INDEX_OUT_OF_BOUND_OR_LIST_TOO_LONG
///         otherwise
//
int linked_list_insert(LinkedList *list, int index, void *data, int data_size)
{
	/* check whether data given is NULL */
	if (data == NULL)
	{
		return LINKED_LIST_ERR_NO_DATA_GIVEN;
	}

	/* check whether given list is not NULL */
	if (list == NULL)
	{
		return LINKED_LIST_ERR_LIST_IS_NULL_POINTER;
	}

	/* check whether index is not negative */
	if (index < 0)
	{
		return LINKED_LIST_ERR_INDEX_OUT_OF_BOUND_OR_LIST_TOO_LONG;
	}

	/* create the new list element and copy data */
	LinkedList *new_element = linked_list_new();
	if (new_element == NULL)
	{
		return LINKED_LIST_ERR_OUT_OF_MEMORY;
	}
	linked_list_set_this_item(new_element, data, data_size);

	/* find element after which the new element will be inserted */
	LinkedList *preceding_element = NULL;
	if (index == 0)
	{
		preceding_element = list;
	}
	else
	{
		preceding_element = linked_list_find_item(list, index - 1);
		if (preceding_element == NULL)
		{
			return LINKED_LIST_ERR_INDEX_OUT_OF_BOUND_OR_LIST_TOO_LONG;
		}
	}

	/* insert element into list */
	new_element->next = preceding_element->next;
	preceding_element->next = new_element;

	return LINKED_LIST_SUCCESS;
}

//-----------------------------------------------------------------------------
///
/// Copy data to a specific element of the list
///
/// @param list      pointer to the list
/// @param index     index of the element to copy data to
///                  index must be greater than or equal to zero and smaller
///                  than or equal to the length of the list
/// @param data      pointer to data that should be copied; the data will be
///                  copied to list, therefore data might have to be freed
///                  after calling linked_list_append
/// @param data_size size of data area to be copied in bytes
///
/// @return LINKED_LIST_SUCCESS if successful,
///         LINKED_LIST_ERR_LIST_IS_NULL_POINTER or
///         LINKED_LIST_ERR_INDEX_OUT_OF_BOUND_OR_LIST_TOO_LONG or
///         LINKED_LIST_ERR_NO_DATA_GIVEN otherwise
//
int linked_list_set(LinkedList *list, int index, void *data, int data_size)
{
	/* check whether data is NULL */
	if (data == NULL)
	{
		return LINKED_LIST_ERR_NO_DATA_GIVEN;
	}

	/* check whether list is null pointer */
	if (list == NULL)
	{
		return LINKED_LIST_ERR_LIST_IS_NULL_POINTER;
	}

	/* check if index is not negative */
	if (index < 0)
	{
		return LINKED_LIST_ERR_INDEX_OUT_OF_BOUND_OR_LIST_TOO_LONG;
	}

	/* get element corresponding to index */
	LinkedList *element = linked_list_find_item(list, index);
	if (element == NULL)
	{
		return LINKED_LIST_ERR_INDEX_OUT_OF_BOUND_OR_LIST_TOO_LONG;
	}

	/* copy data to element */
	int ret = linked_list_set_this_item(element, data, data_size);

	return ret;
}

//-----------------------------------------------------------------------------
///
/// Remove an element from the list
///
/// @param list    pointer to the list
/// @param index   index of the element to remove
///                index must be greater than or equal to zero and smaller than
///                or equal to the length of the list
///
/// @return LINKED_LIST_SUCCESS if successful,
///         LINKED_LIST_ERR_LIST_IS_NULL_POINTER or
///         LINKED_LIST_ERR_INDEX_OUT_OF_BOUND_OR_LIST_TOO_LONG otherwise
//
int linked_list_remove(LinkedList *list, int index)
{
	/* check whether list is not NULL */
	if (list == NULL)
	{
		return LINKED_LIST_ERR_LIST_IS_NULL_POINTER;
	}

	/* check whether index is not negative */
	if (index < 0)
	{
		return LINKED_LIST_ERR_INDEX_OUT_OF_BOUND_OR_LIST_TOO_LONG;
	}

	/* find the element at given index (element to remove) */
	LinkedList *element = linked_list_find_item(list, index);
	if (element == NULL)
	{
		return LINKED_LIST_ERR_INDEX_OUT_OF_BOUND_OR_LIST_TOO_LONG;
	}

	/* find element before element to remove */
	LinkedList *preceding_element = NULL;
	if (index == 0)
	{
		preceding_element = list;
	}
	else
	{
		preceding_element = linked_list_find_item(list, index - 1);
		if (preceding_element == NULL)
		{
			return LINKED_LIST_ERR_INDEX_OUT_OF_BOUND_OR_LIST_TOO_LONG;
		}
	}

	/* link list together without removed item */
	preceding_element->next = element->next;

	/* free element to remove */
	free(element->data);
	free(element);

	return LINKED_LIST_SUCCESS;
}

//-----------------------------------------------------------------------------
///
/// Read data of an element from the list
///
/// @param list      pointer to the list
/// @param index     index of the element to read
///                  index must be greater than or equal to zero and smaller than
///                  or equal to the length of the list
/// @param data_size pointer to an integer which will contain size of data area
///                  of element data in bytes
///
/// @return pointer to data of the element or
///         returns NULL if index is out of bound and element is not found
//
void *linked_list_get(LinkedList *list, int index, int *data_size)
{
	/* check whether data_size is null pointer */
	if (data_size == NULL)
	{
		/* if it is NULL write data size rather to any temporary variable that
		 * will be deleted after calling the function
		 */
		int temp;
		data_size = &temp;
	}

	/* check whether list is null pointer */
	if (list == NULL)
	{
		*data_size = 0;
		return NULL;
	}

	/* check whether index is negative */
	if (index < 0)
	{
		*data_size = 0;
		return NULL;
	}

	LinkedList *element = linked_list_find_item(list, index);
	if (element == NULL)
	{
		*data_size = 0;
		return NULL;
	}

	*data_size = element->data_size;
	return element->data;
}
