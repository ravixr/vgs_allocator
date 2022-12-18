// vgs_allocator.h - v1.0
// -------------------------------------------------------------------------------
// MIT License
//
// Copyright (c) 2022 Vinicius G. Santos
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// -------------------------------------------------------------------------------

#ifndef INCLUDE_VGS_ALLOCATOR_H
#define INCLUDE_VGS_ALLOCATOR_H

#include <stddef.h>

#define HEAP_SIZE 64000
#define MIN_BLOCK_SIZE 32

#ifdef VGS_ALC_STATIC
#define VGS_ALC_DEF static
#else
#define VGS_ALC_DEF extern
#endif

VGS_ALC_DEF void *vgs_malloc(size_t);
VGS_ALC_DEF void vgs_free(void *);
VGS_ALC_DEF void *vgs_realloc(void *, size_t);
VGS_ALC_DEF void *vgs_calloc(size_t);

#endif // INCLUDE_VGS_ALLOCATOR_H