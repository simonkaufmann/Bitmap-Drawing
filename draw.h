/*
 *  draw.h - Definitions for drawing different shapes
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

#ifndef DRAW_H
#define DRAW_H

#include "parse.h"
#include "bitmap.h"

#define DRAW_SUCCESS 0
#define DRAW_ERR_NULL_POINTER_PASSED 1
#define DRAW_ERR_COMMAND_INVALID 2

int draw_command(PixelBuffer *pix_buffer, Command *comm);

#endif
