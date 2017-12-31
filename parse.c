/*
 *  parse.c - Code for parsing the input file
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
#include <stdio.h>
#include <string.h>

#include "parse.h"
#include "linked_list.h"
#include "list.h"
#include "main.h"

#define LINE_INCREMENT 4000000


typedef struct _Property_ {
	char name[PARSE_MAX_PROPERTY_NAME_LENGTH];
	int64_t value;
} Property;

char *prop_rectangle[] = {
	"id",
	"color",
	"x",
	"y",
	"width",
	"height",
	""
};

char *prop_circle[] = {
	"id",
	"color",
	"x",
	"y",
	"radius",
	""
};

char *prop_triangle[] = {
	"id",
	"color",
	"ax",
	"ay",
	"bx",
	"by",
	"cx",
	"cy",
	""
};

//-----------------------------------------------------------------------------
///
/// Read a line from file, reading stops after newline or EOF, if newline
/// is read, then it's included in the buffer, buffer of sufficient size will
/// be allocated (and has to be freed later on if function returns
/// PARSE_SUCCESS)
///
/// @param line_buffer pointer to a char pointer, this char pointer will point
///                    to a char array containing the read line if
///                    function returns PARSE_SUCCESS
/// @param n           pointer to an integer that will store size of the data
///                    data array if function returns PARSE_SUCCESS
/// @param file        file structure to read from
///
/// @return PARSE_SUCCESS on success, PARSE_ERR_OUT_OF_MEM or PARSE_ERR_EOF
///         (if end of file is reached)
//
static int readline(char **line_buffer, int *n, FILE *file)
{
	int ret;

	/* allocate buffer */
	int buffer_size = LINE_INCREMENT;
	char *buffer = malloc(buffer_size);
	int index = 0;
	if (buffer == NULL)
	{
		return PARSE_ERR_OUT_OF_MEM;
	}

	buffer[index] = fgetc(file);
	while (buffer[index] != EOF && buffer[index] != '\n')
	{
		index++;

		/* if necessary realloc memory */
		if (index >= (buffer_size - 1)) /* (buffer_size - 1) is necessary so
										 * that always a byte for terminating
										 * zero is available
										 */
		{
			buffer_size += LINE_INCREMENT;
			char *buffer_new = realloc(buffer, buffer_size);
			if (buffer_new == NULL)
			{
				ret = PARSE_ERR_OUT_OF_MEM;
				goto readline_cleanup;
			}
			buffer = buffer_new;
		}

		buffer[index] = fgetc(file);
	}

	if (index == 0 && buffer[index] == EOF)
	{
		ret = PARSE_ERR_EOF;
		goto readline_cleanup;
	}

	buffer[index + 1] = 0; /* terminate the string */
	*n = buffer_size;
	*line_buffer = buffer;

	return PARSE_SUCCESS;

readline_cleanup:
	free(buffer);
	return ret;
}

//-----------------------------------------------------------------------------
///
/// Helper function for adding a part of a string to a linked list
///
/// @param list        list where the part of the string will be appended
/// @param line        pointer to a char array containing a string
/// @param index_start index of the first char which will be appended to list
/// @param index_end   index of the first char that won't be appended any more
///
/// @return PARSE_SUCCESS on success, PARSE_ERR_OUT_OF_MEM or PARSE_ERR_LIST
///         (for errors given back from linked_list library)
//
static int insert_split_to_list(LinkedList *list,
								char *line, int index_start, int index_end)
{
	/* check index bounds */
	if (index_end > index_start)
	{
		/* calculate size of string that will be appended to list */
		int size = index_end - index_start + 1;

		/* allocate temporary buffer for string that will be appended to list */
		char *buffer = malloc(size);
		if (buffer == NULL)
		{
			return PARSE_ERR_OUT_OF_MEM;
		}

		/* copy string to the temporary buffer */
		strncpy(buffer, line + index_start, size - 1);
		buffer[size - 1] = 0; /* to ensure that there is terminating zero at end */

		/* append string to list */
		int ret = linked_list_append(list, buffer, size);
		int error_success = PARSE_SUCCESS;
		if (ret != LINKED_LIST_SUCCESS)
		{
			if (ret == LINKED_LIST_ERR_OUT_OF_MEMORY)
			{
				error_success = PARSE_ERR_OUT_OF_MEM;
			}
			else
			{
				error_success = PARSE_ERR_LIST;
			}
		}

		/* free temporary buffer */
		free(buffer);
		return error_success;
	}
	return PARSE_ERR_INDEX_OUT_OF_BOUND;
}

//-----------------------------------------------------------------------------
///
/// Split a char array into a list of smaller char arrays.
/// String will be split at spaces (removing the spaces from the result)
/// and at every equal sign (including the equal sign as a separate split part
/// in the result)
///
/// @param line     pointer to char array containing the string to be split
///
/// @return pointer to a LinkedList that contains the split string parts or
///         return NULL if memory allocation failed or NULL if list has grown
///         too long
//
static LinkedList *split_line(char *line)
{
	/* create new linked list for splitted strings */
	LinkedList *split = linked_list_new();
	if (split == NULL)
	{
		return NULL;
	}

	/* search line for spaces and equal signs */
	int index_end = 0, index_start = 0;
	int ret;
	/* iterate through string until end of string */
	while (line[index_end] != 0 && line[index_end] != '\n')
	{
		/* split at space */
		if (line[index_end] == ' ')
		{
			/*
			 * Check whether the immediate next character after space or '='
			 * is also an equal sign because then the "last part" oft the string
			 * is empty and hence should not be copied to the list
			 */
			if (index_start != index_end) {
				/* insert the last part of the string to the list */
				ret = insert_split_to_list(split, line, index_start, index_end);
				if (ret != PARSE_SUCCESS)
				{
					return NULL;
				}
			}

			/* jump over remaining spaces */
			while (line[index_end] == ' ')
			{
				index_end++;
			}
			index_start = index_end;
		}
		else if (line[index_end] == '=')
		{
			/*
			 * Check whether the immediate next character after space or '='
			 * is also an equal sign because then the "last part" oft the string
			 * is empty and hence should not be copied to the list
			 */
			if (index_start != index_end) {
				/* insert the last part of the string to the list */
				ret = insert_split_to_list(split, line, index_start, index_end);
				if (ret != PARSE_SUCCESS)
				{
					return NULL;
				}
			}

			/* insert the equal sign to the string */
			index_start = index_end;
			index_end++;
			ret = insert_split_to_list(split, line, index_start, index_end);
			if (ret != PARSE_SUCCESS)
			{
				return NULL;
			}

			/* update index_start to character after '=' */
			index_start = index_end;
		}
		else
		{
			index_end++;
		}
	}

	/*
	 * If there is remaining part in the string which was not appended to list,
     * then append it
	 */
	if (index_start != index_end)
	{
		ret = insert_split_to_list(split, line, index_start, index_end);
		if (ret != PARSE_SUCCESS)
		{
			return NULL;
		}
	}

	return split;
}

//-----------------------------------------------------------------------------
///
/// Converts the string containing a property value to an integer value
///
/// @param value_string  string containing the value, starting and ending with "
///                      (double quote)
/// @param value         Pointer to an integer in which the converted value will
///                      be written
/// @param base          the base of the represented string (hexadecimal,
///                      decimal...)
///
/// @return PARSE_SUCCESS on success, PARSE_ERR_INVALID_INPUT otherwise
//
static int convert_to_value(char *value_string, int64_t *value, int base)
{
	/* check double quote at beginning of string */
	if (value_string[0] != '"')
	{
		return PARSE_ERR_INVALID_INPUT;
	}

	/* search for end of string */
	int index = 0;
	int secure_counter = PARSE_MAX_VALUE_STRING_LENGTH;
	while (value_string[index] != 0 || secure_counter < 0)
	{
		index++;
		secure_counter--;
	}
	if (index <= 2)
	{
		return PARSE_ERR_INVALID_INPUT;
	}

	/* check double quote at end of string */
	if (value_string[index - 1] != '"')
	{
		return PARSE_ERR_INVALID_INPUT;
	}
	value_string[index - 1] = 0; /* temporarily remove the '"' by a \0 */

	/* convert to integer */
	char *endptr;
	int num = strtol(value_string + 1, &endptr, base);
	if (*endptr != 0)
	{
		return PARSE_ERR_INVALID_INPUT;
	}

	value_string[index - 1] = '"'; /* restore the '"' instead of the \0 */

	*value = num;
	return PARSE_SUCCESS;
}

//-----------------------------------------------------------------------------
///
/// Takes a list coming from split_line function and translates it to a list
/// of Property structures
///
/// @param split_list   List as outputted by split_line function
/// @param prop_list    Pointer to a pointer to a list that later will be set
///                     pointing to the list with properties
///
/// @return PARSE_SUCCESS on success, PARSE_ERR_PASSED_NULL_POINTER,
///         PARSE_ERR_OUT_OF_MEM, PARSE_ERR_INVALID_INPUT or
///         PARSE_ERR_PROPERTY_NAME_TOO_LONG otherwise
//
static int split_to_properties(LinkedList *split_list, List **prop_list)
{
	/* check whether parameters were passed correctly */
	if (prop_list == NULL)
	{
		return PARSE_ERR_PASSED_NULL_POINTER;
	}

	if (split_list == NULL)
	{
		return PARSE_ERR_PASSED_NULL_POINTER;
	}

	/* create property list */
	List *list = list_new(sizeof(Property));
	if (list == NULL)
	{
		return PARSE_ERR_OUT_OF_MEM;
	}

	Property prop;
	int ret;

	/* get first property which doesn't have a value */
	int data_size;
	char *command = linked_list_get(split_list, 0, &data_size);
	if (command == NULL)
	{
		return PARSE_ERR_INVALID_INPUT;
	}
	if (data_size >= PARSE_MAX_PROPERTY_NAME_LENGTH)
	{
		return PARSE_ERR_PROPERTY_NAME_TOO_LONG;
	}

	strncpy(prop.name, command, data_size);
	prop.name[PARSE_MAX_PROPERTY_NAME_LENGTH - 1] = 0;
	prop.value = 0;

	/* append the command to list */
	ret = list_append(list, &prop);
	if (ret != LIST_SUCCESS)
	{
		if (ret == LIST_ERR_OUT_OF_MEMORY)
		{
			return PARSE_ERR_OUT_OF_MEM;
		}
		else
		{
			return PARSE_ERR_LIST;
		}
	}

	/* get all other properties */
	int index = 1;
	command = linked_list_get(split_list, index, &data_size);
	while (command != NULL)
	{
		/* get property name */
		if (data_size >= PARSE_MAX_PROPERTY_NAME_LENGTH)
		{
			return PARSE_ERR_PROPERTY_NAME_TOO_LONG;
		}
		strncpy(prop.name, command, data_size);
		prop.name[PARSE_MAX_PROPERTY_NAME_LENGTH - 1] = 0;

		index++;

		/* next command must be equal sign */
		command = linked_list_get(split_list, index, &data_size);
		if (command == NULL)
		{
			return PARSE_ERR_INVALID_INPUT;
		}
		if (strncmp(command, "=", data_size) != 0)
		{
			return PARSE_ERR_INVALID_INPUT;
		}

		index++;

		/* next command is the value */
		command = linked_list_get(split_list, index, &data_size);
		if (command == NULL)
		{
			return PARSE_ERR_INVALID_INPUT;
		}

		/* convert value */
		int base = 10;
		if (strcmp(prop.name, "color") == 0)
		{
			base = 16;
		}
		ret = convert_to_value(command, &prop.value, base);
		if (ret != PARSE_SUCCESS)
		{
			return ret;
		}

		/* append the command to list */
		ret = list_append(list, &prop);
		if (ret != LIST_SUCCESS)
		{
			if (ret == LIST_ERR_OUT_OF_MEMORY)
			{
				return PARSE_ERR_OUT_OF_MEM;
			}
			else
			{
				return PARSE_ERR_LIST;
			}
		}

		/* next command for next property */
		index++;
		command = linked_list_get(split_list, index, &data_size);
	}
	*prop_list = list;
	return PARSE_SUCCESS;
}

//-----------------------------------------------------------------------------
///
/// Find a specific property in a property list
///
/// @param list    pointer to the list as returned by split_to_properties
/// @param name    name of the property that is searched for
/// @param value   pointer to an integer in which the value of the property will
///                be stored
///
/// @return PARSE_SUCCESS on success or PARSE_ERR_PASSED_NULL_POINTER,
/// PARSE_ERR_INVALID_INPUT (if property isn't found) otherwise
//
static int find_property(List *list, char *name, int *value)
{
	/* check whether list is NULL */
	if (list == NULL)
	{
		return PARSE_ERR_PASSED_NULL_POINTER;
	}

	/* search list for correct command */
	int index_command = 0;
	Property *prop_command = list_get(list, index_command);
	while (prop_command != NULL)
	{
		if (strcmp(prop_command->name, name) == 0)
		{
			*value = prop_command->value;
			break;
		}

		index_command++;
		prop_command = list_get(list, index_command);
	}

	/* if prop_command after while is NULL, then property wasn't found */
	if (prop_command == NULL)
	{
		return PARSE_ERR_INVALID_INPUT;
	}

	return PARSE_SUCCESS;
}

//-----------------------------------------------------------------------------
///
/// Converts a list containing the properties to an command structure of the
/// given shape
/// If (and only if) function returns PARSE_SUCCESS, then caller is responsible
/// to properly free the Command pointed to by command pointer (for Command the
/// command->obj and command itself have to be freed!)
///
/// @param list    pointer to the list as returned by split_to_properties
/// @param command pointer to an pointer to an command structure. After
///                calling the function this pointer will point to the newly
///                created command
///
/// @return PARSE_SUCCESS on success, PARSE_ERR_PASSED_NULL_POINTER,
///         PARSE_ERR_OUT_OF_MEM or PARSE_ERR_INVALID_INPUT otherwise
//
static int properties_to_command(List *list, Command **command)
{
	int ret;

	/* check whether list is NULL */
	if (list == NULL)
	{
		return PARSE_ERR_PASSED_NULL_POINTER;
	}

	/* get properties */
	Property *prop_command = list_get(list, 0);
	int value;
	if (prop_command == NULL)
	{
		return PARSE_ERR_INVALID_INPUT;
	}

	Command *comm = malloc(sizeof(Command));
	if (comm == NULL)
	{
		return PARSE_ERR_OUT_OF_MEM;
	}
	memset(comm, 0, sizeof(Command));

	/* check shape of the command */
	if (strcmp(prop_command->name, "rectangle") == 0)
	{
		/* rectangle */

		/* allocate memory for rectangle */
		Rectangle *rect = malloc(sizeof(Rectangle));
		if (rect == NULL)
		{
			ret = PARSE_ERR_OUT_OF_MEM;
			goto properties_to_command_cleanup1;
		}
		memset(rect, 0, sizeof(Rectangle));

		comm->shape = SH_RECTANGLE;
		comm->obj = rect;

		/* read and set property values */
		ret = find_property(list, "id", &value);
		rect->id = value;
		comm->id = value;
		if (ret != PARSE_SUCCESS)
		{
			goto properties_to_command_cleanup2;
		}

		ret = find_property(list, "color", &value);
		rect->color = value;
		if (ret != PARSE_SUCCESS)
		{
			goto properties_to_command_cleanup2;
		}

		ret = find_property(list, "x", &value);
		rect->x = value;
		if (ret != PARSE_SUCCESS)
		{
			goto properties_to_command_cleanup2;
		}

		ret = find_property(list, "y", &value);
		rect->y = value;
		if (ret != PARSE_SUCCESS)
		{
			goto properties_to_command_cleanup2;
		}

		ret = find_property(list, "width", &value);
		rect->width = value;
		if (ret != PARSE_SUCCESS)
		{
			goto properties_to_command_cleanup2;
		}

		ret = find_property(list, "height", &value);
		rect->height = value;
		if (ret != PARSE_SUCCESS)
		{
			goto properties_to_command_cleanup2;
		}
	}
	else if (strcmp(prop_command->name, "triangle") == 0)
	{
		/* triangle */

		/* allocate memory for triangle */
		Triangle *triangle = malloc(sizeof(Triangle));
		if (triangle == NULL)
		{
			ret = PARSE_ERR_OUT_OF_MEM;
			goto properties_to_command_cleanup1;
		}
		memset(triangle, 0, sizeof(Triangle));

		comm->shape = SH_TRIANGLE;
		comm->obj = triangle;

		/* read and set properties */
		ret = find_property(list, "id", &value);
		triangle->id = value;
		comm->id = value;
		if (ret != PARSE_SUCCESS)
		{
			goto properties_to_command_cleanup2;
		}

		ret = find_property(list, "color", &value);
		triangle->color = value;
		if (ret != PARSE_SUCCESS)
		{
			goto properties_to_command_cleanup2;
		}

		ret = find_property(list, "ax", &value);
		triangle->ax = value;
		if (ret != PARSE_SUCCESS)
		{
			goto properties_to_command_cleanup2;
		}

		ret = find_property(list, "ay", &value);
		triangle->ay = value;
		if (ret != PARSE_SUCCESS)
		{
			goto properties_to_command_cleanup2;
		}

		ret = find_property(list, "bx", &value);
		triangle->bx = value;
		if (ret != PARSE_SUCCESS)
		{
			goto properties_to_command_cleanup2;
		}

		ret = find_property(list, "by", &value);
		triangle->by = value;
		if (ret != PARSE_SUCCESS)
		{
			goto properties_to_command_cleanup2;
		}

		ret = find_property(list, "cx", &value);
		triangle->cx = value;
		if (ret != PARSE_SUCCESS)
		{
			goto properties_to_command_cleanup2;
		}

		ret = find_property(list, "cy", &value);
		triangle->cy = value;
		if (ret != PARSE_SUCCESS)
		{
			goto properties_to_command_cleanup2;
		}
	}
	else if (strcmp(prop_command->name, "circle") == 0)
	{
		/* circle */

		/* allocate memory for circle */
		Circle *circle = malloc(sizeof(Circle));
		if (circle == NULL)
		{
			ret = PARSE_ERR_OUT_OF_MEM;
			goto properties_to_command_cleanup1;
		}
		memset(circle, 0, sizeof(Circle));

		comm->shape = SH_CIRCLE;
		comm->obj = circle;

		/* read and set properties */
		ret = find_property(list, "id", &value);
		circle->id = value;
		comm->id = value;
		if (ret != PARSE_SUCCESS)
		{
			goto properties_to_command_cleanup2;
		}

		ret = find_property(list, "color", &value);
		circle->color = value;
		if (ret != PARSE_SUCCESS)
		{
			goto properties_to_command_cleanup2;
		}

		ret = find_property(list, "x", &value);
		circle->x = value;
		if (ret != PARSE_SUCCESS)
		{
			goto properties_to_command_cleanup2;
		}

		ret = find_property(list, "y", &value);
		circle->y = value;
		if (ret != PARSE_SUCCESS)
		{
			goto properties_to_command_cleanup2;
		}

		ret = find_property(list, "radius", &value);
		circle->radius = value;
		if (ret != PARSE_SUCCESS)
		{
			goto properties_to_command_cleanup2;
		}
	}
	else
	{
		/* no shape recognised */
		ret = PARSE_ERR_INVALID_INPUT;
		goto properties_to_command_cleanup1;
	}

	/* return command via pointer */
	*command = comm;

	return PARSE_SUCCESS;

properties_to_command_cleanup2:
	free(comm->obj);
properties_to_command_cleanup1:
	free(comm);
	return ret;
}

//-----------------------------------------------------------------------------
///
/// Parses command given as a string to the Command structure
/// If and only if the function returns PARSE_SUCCESS, the caller is responsible
/// to free the Command (comm->obj and comm) properly!
///
/// @param line    The command line read from input file, given as char array
/// @param comm    Pointer to a pointer to a command structure. After calling
///                this function the pointer will point to the newly created
///                command structure
///
/// @return PARSE_SUCCESS on success, PARSE_ERR_PASSED_NULL_POINTER,
///         PARSE_ERR_OUT_OF_MEM or PARSE_ERR_INVALID_INPUT otherwise
//
int parse_line(char *line, Command **comm)
{
	int ret;

	LinkedList *split = split_line(line);

	if (split == NULL) {
		ret = PARSE_ERR_OUT_OF_MEM;
		goto parse_line_cleanup1;
	}

	List *property;
	ret = split_to_properties(split, &property);
	if (ret != PARSE_SUCCESS)
	{
		goto parse_line_cleanup1;
	}

	ret = properties_to_command(property, comm);
	if (ret != PARSE_SUCCESS)
	{
		goto parse_line_cleanup2;
	}

	ret = PARSE_SUCCESS;
parse_line_cleanup2:
	list_delete(property);
parse_line_cleanup1:
	linked_list_delete(split);
	return ret;
}

//-----------------------------------------------------------------------------
///
/// Parses an input file and returns a list containing all commands from file
/// If and only if the function returns SUCCESS, the caller is responsible
/// to free the list pointed to by list (the function
/// parse_delete_command_list should be used)
/// Function also outputs error messages if there is a problem with file
///
/// @param input_path  path to input file
/// @param list        pointer to pointer to list, the pointer will point
///                    to created list with commands if functions returns
///                    successfully, caller responsible for freeing the list and
///                    the commands contained in the list!
///
/// @return SUCCESS on success, ERR_READ_INPUT, ERR_OUT_OF_MEM, ERR_UNRECOGNISED
///         or ERR_DUPLICATE_ID or ERR_INVALID_INPUT otherwise
//
int parse_file(char *input_path, LinkedList **list)
{
	int ret;

	/* try to open input file */
	FILE *input = fopen(input_path, "r");
	if (input == NULL) {
		printf(err_msg_read_input, input_path);
		return(ERR_READ_INPUT);
	}

	/* create list for commands */
	LinkedList *command_list = linked_list_new();
	if (command_list == NULL)
	{
		printf(err_msg_out_of_mem);
		ret = ERR_OUT_OF_MEM;
		goto parse_file_cleanup1;
	}

	/* read input file and parse to command list */
	char *line_buffer = NULL;
	int line_buffer_size = 0;
	int line_number = 1;
	Command *command = NULL; /*
							  * set to NULL to ensure that it will only be freed
							  * if it points to actual Command!
							  */
	Command *search_id_command = NULL;

	/* read each line */
	ret = readline(&line_buffer, &line_buffer_size, input);
	while (ret == PARSE_SUCCESS) /* break if EOF is reached or if out of mem */
	{
		/* parse line and get command structure */
		ret = parse_line(line_buffer, &command);
		if (ret != PARSE_SUCCESS)
		{
			if (ret == PARSE_ERR_OUT_OF_MEM)
			{
				printf(err_msg_out_of_mem);
				ret = ERR_OUT_OF_MEM;
				goto parse_file_cleanup1;
			}
			else if (ret == PARSE_ERR_INVALID_INPUT)
			{
				printf(err_msg_invalid_input, line_number);
				ret = ERR_INVALID_INPUT;
				goto parse_file_cleanup1;
			}
			else
			{
				printf(err_msg_unrecognised);
				ret = ERR_UNRECOGNISED;
				goto parse_file_cleanup1;
			}
		}

		/* get id of command element */
		int id = command->id;

		/* search right place for id in the list */
		int index_search_list = 0, data_size;
		search_id_command = linked_list_get(command_list,
											index_search_list, &data_size);
		while (search_id_command != NULL)
		{
			if (data_size != sizeof(Command))
			{
				printf(err_msg_unrecognised);
				ret = ERR_UNRECOGNISED;
				goto parse_file_cleanup2;
			}
			if (id < search_id_command->id)
			{
				/*
				 * search_id_command is index of position where element
				 * should be inserted
				 */
				break;
			}
			else if (id == search_id_command->id)
			{
				/* this id is already in the list -> duplicate */
				printf(err_msg_duplicate_id, id);
				ret = ERR_DUPLICATE_ID;
				goto parse_file_cleanup2;
			}

			/* increment index and get next command of list */
			index_search_list++;
			search_id_command = linked_list_get(command_list,
												index_search_list, &data_size);
		}

		/* insert command to correct index in list */
		ret = linked_list_insert(command_list, index_search_list,
								 command, sizeof(Command));
		if (ret != LINKED_LIST_SUCCESS)
		{
			if (ret == LINKED_LIST_ERR_OUT_OF_MEMORY)
			{
				printf(err_msg_out_of_mem);
				ret = ERR_OUT_OF_MEM;
				goto parse_file_cleanup2;
			}
			else
			{
				printf(err_msg_unrecognised);
				ret = ERR_UNRECOGNISED;
				goto parse_file_cleanup2;
			}
		}

		/* increase line_number (line number only needed for error output */
		line_number++;
		/* setting line_buffer and line_buffer_size to empty */
		free(line_buffer);
		line_buffer = NULL;
		line_buffer_size = 0;
		/*
		 * free current comm element, before a new one will be created next
		 * iteration, since comm element is copied into list, the obj part must
		 * not be freed!
		 */
		free(command);
		command = NULL; /* set to NULL to ensure that it won't be freed twice */

		/* read next line */
		ret = readline(&line_buffer, &line_buffer_size, input);
	}
	if (ret == PARSE_ERR_OUT_OF_MEM)
	{
		goto parse_file_cleanup1;
	}

	ret = SUCCESS;
	*list = command_list;
	goto parse_file_end;

parse_file_cleanup2:
	/* delete comm element */
	if (command != NULL)
	{
		if (command->obj != NULL)
		{
			free(command->obj);
		}
		free(command);
		command = NULL;
	}
parse_file_cleanup1:
	/* delete command list */
	parse_delete_command_list(command_list);
	/* delete line buffer */
	if (line_buffer != NULL)
	{
		free(line_buffer);
		line_buffer = NULL;
	}
parse_file_end:
	/* close input file */
	fclose(input);
	return ret;
}

//-----------------------------------------------------------------------------
///
/// For deleting the command list created by parse_file
///
/// @param command_list  command list created by parse_file
//
void parse_delete_command_list(LinkedList *command_list)
{
	if (command_list == NULL)
	{
		return;
	}

	/*
	 * free the obj of the commands (because linked_list_delete will
	 * only delete the commands, but not the objects the commands point to!
	 */
	int index = 0;
	int data_size;
	Command *comm = linked_list_get(command_list, index, &data_size);
	while (comm != NULL)
	{
		if (data_size == sizeof(Command))
		{
			if (comm->obj != NULL)
			{
				free(comm->obj);
			}
		}
		index++;
		comm = linked_list_get(command_list, index, &data_size);
	}

	/* free the list */
	linked_list_delete(command_list);
}
