//	Copyright (c) 2011-2021 by Artem A. Gevorkyan (gevorkyan.org)
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.

#include <common/memory.h>

#include <new>
#include <stdexcept>
#include <sys/mman.h>

namespace micro_profiler
{
	scoped_unprotect::scoped_unprotect(byte_range region)
		: _region(region)
	{
		byte *page = reinterpret_cast<byte *>(reinterpret_cast<size_t>(region.begin()) & ~static_cast<size_t>(0x0FFF));
		size_t full_length = region.length() + (region.begin() - page);
		
		_region = byte_range(page, full_length);
		if (mprotect(_region.begin(), _region.length(), PROT_EXEC | PROT_READ | PROT_WRITE))
			throw std::runtime_error("Cannot change protection mode!");
	}

	scoped_unprotect::~scoped_unprotect()
	{	mprotect(_region.begin(), _region.length(), PROT_EXEC | PROT_READ);	}


	executable_memory_allocator::block::block(size_t size)
		: _region(static_cast<byte *>(::mmap(0, size, PROT_EXEC | PROT_READ | PROT_WRITE,
			 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)), size), _occupied(0)
	{
		if (!_region.begin())
			throw std::bad_alloc();
	}

	executable_memory_allocator::block::~block()
	{	::munmap(_region.begin(), _region.length());	}
}
