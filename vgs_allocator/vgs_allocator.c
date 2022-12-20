// vgs_allocator.c - v1.0.1
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

#ifndef VGS_ALLOCATOR_IMPLEMENTATION
#define VGS_ALLOCATOR_IMPLEMENTATION

#include "vgs_allocator.h"
#include <assert.h> // _assert
#include <string.h> // memcpy, memset

// quick bool type implementation
typedef char bool;
#define true 1
#define false 0

struct vgs_block_info {
	size_t size;
	size_t u_size;
	struct vgs_block_info *prev;
	struct vgs_block_info *next;
};

#define BLOCK_INFO_SIZE sizeof(struct vgs_block_info)

typedef char byte;
static byte heap_[HEAP_SIZE] = { 0 };
static void *heap_ptr_ = heap_;

static struct vgs_block_info *alc_head_ = NULL;
static struct vgs_block_info *free_head_ = NULL;
static unsigned alc_count_ = 0;

static void set_block_info(struct vgs_block_info *, size_t, size_t, struct vgs_block_info *,
						   struct vgs_block_info *);
static void *alloc_block(size_t);

/// @brief Reserves a block of memory with the specified size in bytes if available.
/// @param size The size in bytes of the memory block to be used.
/// @return A pointer to a block of memory with the specified size available or NULL.
VGS_ALC_DEF void *vgs_malloc(size_t size)
{
	void *address = NULL;
	if (size == 0)
		return NULL;
#if defined(VGS_DEBUG)
	if (alc_head_ == NULL) {
		alc_head_ = (struct vgs_block_info *)heap_ptr_;
		set_block_info(alc_head_, size, size, NULL, NULL);
		address = alc_head_ + 1;
		heap_ptr_ = (byte *)heap_ptr_ + BLOCK_INFO_SIZE + size;
	}
#endif
	struct vgs_block_info *i = free_head_;
	for (; i != NULL && i->size < size; i = i->next) {}
	if (i == NULL) {
		address = alloc_block(size);
	} else {
		if (free_head_ == i) {
			free_head_ = i->next;
		}
		if (i->size >= size + BLOCK_INFO_SIZE + MIN_BLOCK_SIZE) {
			struct vgs_block_info *tmp = (struct vgs_block_info *)((byte *)(i + 1) + size);
			set_block_info(tmp, i->size - (size + BLOCK_INFO_SIZE * 2), 0, i->prev, i->next);
			if (i->prev)
				i->prev->next = tmp;
			if (i->next)
				i->next->prev = tmp;
			i->size = size;
		} else {
			if (i->prev)
				i->prev->next = i->next;
			if (i->next)
				i->next->prev = i->prev;
		}
		i->u_size = size;
		i->prev = NULL;
		i->next = alc_head_;
		if (alc_head_)
			alc_head_->prev = i;
		alc_head_ = i;
		address = i + 1;
	}
	if (address != NULL)
		alc_count_++;
	return address;
}

/// @brief Frees the block of memory specified.
/// @param block A pointer to a block of memory allocated by vgs_malloc.
VGS_ALC_DEF void vgs_free(void *block)
{
	struct vgs_block_info *i = alc_head_, *j = free_head_, *aux = free_head_;
	for (; i != NULL; i = i->next, j = j == NULL ? NULL : j->next) {
		if (aux < j && (byte *)(j + 1) < (byte *)block) {
			aux = j;
		} else if (j + 1 == block) {
			_assert("vgs_free(): Double free detected", __FILE__, __LINE__ - 1);
		}
		if (i + 1 == block)
			break;
	}
	if (i != NULL) {
		if (alc_head_ == i)
			alc_head_ = i->next;
		else if (i->prev)
			i->prev->next = i->next;
		if (i->next)
			i->next->prev = i->prev;
		if (aux != NULL && (struct vgs_block_info *)((byte *)(aux + 1) + aux->size) == i) {
			aux->size = aux->size + i->size + BLOCK_INFO_SIZE;
			i = aux;
			if (i->prev)
				i->prev->next = i->next;
			if (i->next)
				i->next->prev = i->prev;
		}
		aux = (struct vgs_block_info *)((byte *)(i + 1) + i->size);
		if (aux->u_size == 0) {
			i->size = aux->size + i->size + BLOCK_INFO_SIZE;
			if (aux->prev)
				aux->prev->next = aux->next;
			if (aux->next)
				aux->next->prev = aux->prev;
		}
		i->prev = NULL;
		i->u_size = 0;
		if (free_head_ != i) {
			i->next = free_head_;
			if (free_head_)
				free_head_->prev = i;
			free_head_ = i;
		}
		alc_count_ = alc_count_ > 0 ? alc_count_ - 1 : 0;
		block = NULL;
	}
}

/// @brief Reserves a new block of memory for the specified data with the new specified size,
/// then copies the first 'new_size' bytes that fits in the new block.
/// @param block A pointer to a block of memory allocated by vgs_malloc.
/// @param new_size The new size in bytes of the memory to be used.
/// @return A pointer to the block with the first 'new_size' bytes in the old block or NULL.
VGS_ALC_DEF void *vgs_realloc(void *block, size_t new_size)
{
	void *address = NULL;
	if ((byte *)block >= heap_ + BLOCK_INFO_SIZE && (byte *)block < (byte *)heap_ptr_) {
		struct vgs_block_info *bi = (struct vgs_block_info *)((byte *)block - BLOCK_INFO_SIZE);
		if (bi->u_size == 0)
			_assert("vgs_realloc(): Free block detected", __FILE__, __LINE__ - 1);
		size_t old_size = bi->size;
		byte data[old_size];
		memcpy(data, block, old_size);
		vgs_free(block);
		address = vgs_malloc(new_size);
		if (address != NULL) {
			memcpy(address, data, old_size < new_size ? old_size : new_size);
			block = NULL;
		} else {
			block = vgs_malloc(old_size);
			if (block == NULL)
				_assert("vgs_realloc(): Heap corrupted", __FILE__, __LINE__ - 1);
			memcpy(block, data, old_size);
		}
	}
	return address;
}

/// @brief Reserves a block of memory with the specified size in bytes if available. The memory is set to zero.
/// @param size The size in bytes of the memory block to be used.
/// @return A pointer to a block of memory with the specified size available or NULL.
VGS_ALC_DEF void *vgs_calloc(size_t size)
{
	void *address = vgs_malloc(size);
	if (address != NULL)
		memset(address, 0, size);
	return address;
}

static inline void set_block_info(struct vgs_block_info *bi, size_t size, size_t u_size,
								  struct vgs_block_info *prev, struct vgs_block_info *next)
{
	bi->size = size;
	bi->u_size = u_size;
	bi->prev = prev;
	bi->next = next;
}

// Creates a new block with the specified size
// and returns a pointer to where the data will be stored
static void *alloc_block(size_t size)
{
	struct vgs_block_info *tmp = NULL;
	if (((byte *)heap_ptr_ + BLOCK_INFO_SIZE + size) > heap_ + HEAP_SIZE)
		return NULL;
	tmp = heap_ptr_;
	set_block_info(tmp, size, size, NULL, alc_head_);
	if (alc_head_)
		alc_head_->prev = tmp;
	alc_head_ = tmp;
	heap_ptr_ = ((byte *)(tmp + 1) + size);
	return tmp + 1;
}

#endif // VGS_ALLOCATOR_IMPLEMENTATION