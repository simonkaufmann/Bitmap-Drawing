/*
 *  drawing.c - Code for drawing different shapes into pixel buffers
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
#include <math.h>

#include "draw.h"

//-----------------------------------------------------------------------------
///
/// Draws a horizontal line
/// Be careful, a correct pix_buffer must be passed, no checks
/// are performed!
///
/// @param pix_buffer pixel buffer struct where the rectangle will be drawn into
/// @param x1         first x coordinate for horizontal line
/// @param x2         second x coordinate for horizontal line
/// @param y          y coordinate for horizontal line
//  @param color      color of the line
//
static void horizline(PixelBuffer *pix_buffer, int x1, int x2, int y, int color)
{
	/* if x1 greater than x2 -> swap variables */
	if (x1 > x2)
	{
		int tmp = x2;
		x2 = x1;
		x1 = tmp;
	}

	/* draw line */
	int x;
	if (y >= 0 && y < pix_buffer->height)
	{
		for (x = x1; x < x2; x++)
		{
			if (x >= 0 && x < pix_buffer->width)
			{
				bitmap_write_pixel(pix_buffer, x, y, color);
			}
		}
	}
}

//-----------------------------------------------------------------------------
///
/// Draws the given triangle to the pixel buffer
/// Be careful, a correct pix_buffer and triangle must be passed, no checks
/// are performed!
///
/// @param pix_buffer pixel buffer struct where the rectangle will be drawn into
/// @param triangle   Triangle struct that shall be drawn
//
static void draw_triangle(PixelBuffer *pix_buffer, Triangle *triangle)
{
	int color = triangle->color;
	int ax = triangle->ax;
	int ay = triangle->ay;
	int bx = triangle->bx;
	int by = triangle->by;
	int cx = triangle->cx;
	int cy = triangle->cy;

	double dx1, dx2, dx3;
	double sx, sy, ex, ey;

	int tmp_x, tmp_y;

	/* sort points so that ay >= by >= cy */
	if (ay > by)
	{
		tmp_x = ax;
		tmp_y = ay;
		ax = bx;
		ay = by;
		bx = tmp_x;
		by = tmp_y;
	}
	if (by > cy)
	{
		tmp_x = cx;
		tmp_y = cy;
		cx = bx;
		cy = by;
		bx = tmp_x;
		by = tmp_y;
	}
	if (ay > by)
	{
		tmp_x = ax;
		tmp_y = ay;
		ax = bx;
		ay = by;
		bx = tmp_x;
		by = tmp_y;
	}

	/* calculate slopes */
	if (by - ay > 0)
	{
		dx1 = ((double)bx - ax) / ((double)by - ay);
	}
	else
	{
		dx1 = 0;
	}

	if (cy - ay > 0)
	{
		dx2 = ((double)cx - ax) / ((double)cy - ay);
	}
	else
	{
		dx2 = 0;
	}

	if (cy - by > 0)
	{
		dx3 = ((double)cx - bx) / ((double)cy - by);
	}
	else
	{
		dx3 = 0;
	}

	/* triangle filler */
	sx = ax;
	sy = ay;
	ex = ax;
	ey = ay;

	if (dx1 > dx2)
	{
		while (sy <= by)
		{
			horizline(pix_buffer, sx, ex, sy, color);
			sy++;
			ey++;
			sx += dx2;
			ex += dx1;
		}
		ex = bx;
		ey = by;
		while (sy <= cy)
		{
			horizline(pix_buffer, sx, ex, sy, color);
			sy++;
			ey++;
			sx += dx2;
			ex += dx3;
		}
	}
	else
	{
		while (sy <= by)
		{
			horizline(pix_buffer, sx, ex, sy, color);
			sy++;
			ey++;
			sx += dx1;
			ex += dx2;
		}
		sx = bx;
		sy = by;
		while (sy <= cy)
		{
			horizline(pix_buffer, sx, ex, sy, color);
			sy++;
			ey++;
			sx += dx3;
			ex += dx2;
		}
	}
}

//-----------------------------------------------------------------------------
///
/// Draws the given circle to the pixel buffer
/// Be careful, a correct pix_buffer and circle must be passed, no checks
/// are performed!
///
/// @param pix_buffer pixel buffer struct where the rectangle will be drawn into
/// @param circle     Circle struct that shall be drawn
//
static void draw_circle(PixelBuffer *pix_buffer, Circle *circle)
{
	/* prepare variables and get information from circle struct */
	int color = circle->color;
	int index_x, index_y;
	int x = circle->x;
	int y = circle->y;
	int y_offset;
	int radius = circle->radius;
	int upper_x, lower_x;
	int upper_y, lower_y;
	int ret;

	/* draw circle */
	for (index_x = 0; index_x < radius; index_x++)
	{
		/*
		 * Each iteration corresponds to one vertical  bar left and right
		 * from circle center, index_x is the distance between circle center
		 * and the vertical bar that will be drawn
		 */
		upper_x = x + index_x;
		lower_x = x - index_x;
		y_offset = sqrt(radius * radius - index_x * index_x);

		if (upper_x >= 0 && upper_x < pix_buffer->width)
		{
			/*
			 * Only draw the vertical bar right from center if it is inside
			 * the image
			 */
			for (index_y = 0; index_y < y_offset; index_y++)
			{
				/* draw the parts of the vertical bar that lie inside image */
				upper_y = y + index_y;
				lower_y = y - index_y;
				if (upper_y >= 0 && upper_y < pix_buffer->height)
				{
					bitmap_write_pixel(pix_buffer, upper_x, upper_y, color);
				}
				if (lower_y >= 0 && lower_y < pix_buffer->height)
				{
					bitmap_write_pixel(pix_buffer, upper_x, lower_y, color);
				}
			}
		}

		if (lower_x > 0 && lower_x < pix_buffer->width)
		{
			/*
			 * Only draw the vertical bar left from center if it is inside
			 * the image
			 */
			for (index_y = 0; index_y < y_offset; index_y++)
			{
				/* draw the parts of the vertical bar that lie inside image */
				upper_y = y + index_y;
				lower_y = y - index_y;
				if (upper_y >= 0 && upper_y < pix_buffer->height)
				{
					ret = bitmap_write_pixel(pix_buffer, lower_x, upper_y, color);
					if (ret != BITMAP_SUCCESS)
					{
						printf("Err bitmap\n");
					}
				}
				if (lower_y >= 0 && lower_y < pix_buffer->height)
				{
					ret = bitmap_write_pixel(pix_buffer, lower_x, lower_y, color);
					if (ret != BITMAP_SUCCESS)
					{
						printf("Err bitmap\n");
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
///
/// Draws the given rectangle to the pixel buffer
/// Be careful, a correct pix_buffer and rectangle must be passed, no checks
/// are performed!
///
/// @param pix_buffer pixel buffer struct where the rectangle will be drawn into
/// @param rectangle  Rectangle struct that shall be drawn
//
static void draw_rectangle(PixelBuffer *pix_buffer, Rectangle *rectangle)
{
	int index_x, index_y;
	int x_orig = rectangle->x;
	int y_orig = rectangle->y;
	int width = rectangle->width;
	int height = rectangle->height;
	int color = rectangle->color;
	int x, y;

	for (index_x = 0; index_x < width; index_x++)
	{
		/* each iteration corresponds to one vertical bar */
		x = x_orig + index_x;
		if (x >= 0 && x < width)
		{
			/* only continue if this vertical bar is inside image */

			/* draw the vertical bar */
			for (index_y = 0; index_y < height; index_y++)
			{
				y = y_orig + index_y;
				if (y >= 0 && y < height)
				{
					/* draw only if inside image */
					bitmap_write_pixel(pix_buffer, x, y, color);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
///
/// Executes the drawing command and writes the shape to the pixel buffer
///
/// @param pix_buffer  pixel buffer struct where the command will be drawn into
/// @param comm        command which will be executed
///
/// @return DRAW_SUCCESS on success, DRAW_ERR_NULL_POINTER_PASSED or
///         DRAW_ERR_COMMAND_INVALID otherwise
//
int draw_command(PixelBuffer *pix_buffer, Command *comm)
{
	if (pix_buffer == NULL)
	{
		return DRAW_ERR_NULL_POINTER_PASSED;
	}
	if (comm == NULL)
	{
		return DRAW_ERR_NULL_POINTER_PASSED;
	}
	if (comm->obj == NULL)
	{
		return DRAW_ERR_COMMAND_INVALID;
	}

	if (comm->shape == SH_TRIANGLE)
	{
		draw_triangle(pix_buffer, (Triangle *)comm->obj);
	}
	else if (comm->shape == SH_CIRCLE)
	{
		draw_circle(pix_buffer, (Circle *)comm->obj);
	}
	else if (comm->shape == SH_RECTANGLE)
	{
		draw_rectangle(pix_buffer, (Rectangle *)comm->obj);
	}
	else
	{
		return DRAW_ERR_COMMAND_INVALID;
	}

	return DRAW_SUCCESS;
}

