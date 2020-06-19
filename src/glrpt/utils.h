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

/*****************************************************************************/

#ifndef GLRPT_UTILS_H
#define GLRPT_UTILS_H

/*****************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/*****************************************************************************/

/* Clamps a double value between min and max */
static inline double dClamp(double x, double min, double max) {
    if (x < min)
        return min;
    else if (x > max)
        return max;
    else
        return x;
}

/*****************************************************************************/

/* Clamps an integer value between min and max */
static inline int iClamp(int i, int min, int max) {
    if (i < min)
        return min;
    else if (i > max)
        return max;
    else
        return i;
}

/*****************************************************************************/

bool prepareDirectories(void);
void File_Name(char *file_name, uint32_t chn, const char *ext);
void Usage(void);
void Show_Message(const char *mesg, const char *attr);
/* TODO may be re-vise all functions below */
void mem_alloc(void **ptr, size_t req);
void mem_realloc(void **ptr, size_t req);
void free_ptr(void **ptr);
bool Open_File(FILE **fp, const char *fname, const char *mode);
void Save_Image_JPEG(
        const char *file_name,
        const int width,
        const int height,
        const bool grayscale,
        const uint8_t *img);
void Save_Image_Raw(
        char *fname,
        const char *type,
        uint32_t width,
        uint32_t height,
        uint32_t max_val,
        uint8_t *buffer);
void Cleanup(void);
int isFlagSet(int flag);
int isFlagClear(int flag);
void SetFlag(int flag);
void ClearFlag(int flag);
void Strlcpy(char *dest, const char *src, size_t n);

/*****************************************************************************/

#endif
