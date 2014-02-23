/*	
    Copyright (C) <2012>  <huangweilook@21cn.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _AOI_H
#define _AOI_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "dlist.h"
#include "bitset.h"
#include "point.h"


/*
 *   基于网格管理的aoi模块，支持变长视距
 */


//地图网格管理单元
struct aoi_block
{
	struct dlist aoi_objs;//处于本网格的所有aoi对象
	uint32_t x;
	uint32_t y;
	uint32_t index;
};

struct aoi_object
{
	struct dnode block_node;             
	struct dnode super_node;     
	struct map_block *current_block;//当前所属的管理单元	
	uint32_t aoi_object_id; 
	struct bit_set view_objs;//在当前对象视野内的对象位图      
	struct point2D pos;        
	void   *ud;
	//使用者提供的函数，用于判断other是否在self的可视范围之内
	uint8_t (*in_myscope)(struct aoi_object *self,struct aoi_object *other);
	//other进入self视野之后的回调函数
	void (*cb_enter)(struct aoi_object *self,struct aoi_object *other);
	//other离开self视野之后的回调函数
	void (*cb_leave)(struct aoi_object *self,struct aoi_object *other);
};


struct aoi_map{
	struct aoi_block blocks[];
};

#endif
