/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details:
 *
 *  http://www.gnu.org/copyleft/gpl.txt
 */

#ifndef GLRPT_UTILS_H
#define GLRPT_UTILS_H

#include "jpeg.h"

#include <glib.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

gboolean PrepareDirectories(void);
gboolean MkdirRecurse(const char *path);
void Usage(void);
void Show_Message(const char *mesg, const char *attr);
void File_Name(char *file_name, uint32_t chn, const char *ext);
void Save_Image_Raw(char *fname, const char *type, uint32_t width, uint32_t height, uint32_t max_val, uint8_t *buffer);
void Save_Image_JPEG(char *file_name, int width, int height, int num_channels, const uint8_t *pImage_data, compression_params_t *comp_params);
void mem_alloc(void **ptr, size_t req);
void mem_realloc(void **ptr, size_t req);
void free_ptr(void **ptr);
gboolean Open_File(FILE **fp, char *fname, const char *mode);
int isFlagSet(int flag);
int isFlagClear(int flag);
void SetFlag(int flag);
void ClearFlag(int flag);
void Strlcpy(char *dest, const char *src, size_t n);
double dClamp(double x, double min, double max);
int iClamp(int i, int min, int max);
void Cleanup(void);

#endif
