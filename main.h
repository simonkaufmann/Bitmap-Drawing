/*
 *  main.h - Definitions for the program
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

#ifndef MAIN_H
#define MAIN_H

#define TRUE 1
#define FALSE 0

#define SUCCESS 0
#define ERR_USAGE 1
#define ERR_READ_INPUT 2
#define ERR_INVALID_INPUT 3
#define ERR_DUPLICATE_ID 4
#define ERR_WRITE_FILE 5
#define ERR_OUT_OF_MEM 6
#define ERR_UNRECOGNISED 7

extern const char *err_msg_usage;
extern const char *err_msg_read_input;
extern const char *err_msg_invalid_input;
extern const char *err_msg_duplicate_id;
extern const char *err_msg_write_file;
extern const char *err_msg_out_of_mem;
extern const char *err_msg_unrecognised;


#endif
