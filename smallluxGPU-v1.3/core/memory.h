/***************************************************************************
 *   Copyright (C) 1998-2009 by authors (see AUTHORS.txt )                 *
 *                                                                         *
 *   This file is part of LuxRender.                                       *
 *                                                                         *
 *   Lux Renderer is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Lux Renderer is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 *   This project is based on PBRT ; see http://www.pbrt.org               *
 *   Lux Renderer website : http://www.luxrender.org                       *
 ***************************************************************************/

#ifndef LUX_MEMORY_H
#define LUX_MEMORY_H

#include <malloc.h>

#include <boost/cstdint.hpp>
using boost::int8_t;

// Memory Allocation Functions
#if !defined(LUX_ALIGNMENT)
#ifdef LUX_USE_SSE
#define LUX_ALIGNMENT 16
#endif
#endif

#ifndef L1_CACHE_LINE_SIZE
#define L1_CACHE_LINE_SIZE 64
#endif

template<class T> inline T *AllocAligned(size_t size, std::size_t N = L1_CACHE_LINE_SIZE)
{
#if defined(WIN32) && !defined(__CYGWIN__) // NOBOOK
	return static_cast<T *>(_aligned_malloc(size * sizeof(T), N));
#else // NOBOOK
	return static_cast<T *>(memalign(N, size * sizeof(T)));
#endif // NOBOOK
}
template<class T> inline void FreeAligned(T *ptr)
{
#if defined(WIN32) && !defined(__CYGWIN__) // NOBOOK
	_aligned_free(ptr);
#else // NOBOOK
	free(ptr);
#endif // NOBOOK
}

template <typename T, std::size_t N = 16> class AlignedAllocator
{
public:
	typedef T value_type;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	typedef T *pointer;
	typedef const T *const_pointer;

	typedef T &reference;
	typedef const T &const_reference;

public:
	inline AlignedAllocator() throw() { }

	template <typename T2> inline AlignedAllocator(const AlignedAllocator<T2, N> &) throw () { }

	inline ~AlignedAllocator() throw() { }

	inline pointer adress(reference r) { return &r; }

	inline const_pointer adress(const_reference r) const { return &r; }

	inline pointer allocate(size_type n)
	{
		return AllocAligned<value_type>(n, N);
	}

	inline void deallocate(pointer p, size_type)
	{
		FreeAligned(p);
	}

	inline void construct(pointer p, const value_type &wert)
	{
		new (p) value_type(wert);
	}

	inline void destroy(pointer p)
	{
		p->~value_type();
	}

	inline size_type max_size() const throw()
	{
		return size_type(-1) / sizeof(value_type);
	}

	template <typename T2> struct rebind
	{
		typedef AlignedAllocator<T2, N> other;
	};
};

#define P_CLASS_ATTR __attribute__
#define P_CLASS_ATTR __attribute__

#if defined(WIN32) && !defined(__CYGWIN__) // NOBOOK
class __declspec(align(16)) Aligned16 {
#else // NOBOOK
class Aligned16 {
#endif // NOBOOK
public:

	/*
	Aligned16(){
		if(((int)this & 15) != 0){
			printf("bad alloc\n");
			assert(0);
		}
	}
	*/

	void *operator new(size_t s) { return AllocAligned<char>(s, 16); }

	void *operator new (size_t s, void *q) { return q; }

	void operator delete(void *p) { FreeAligned(p); }
#if defined(WIN32) && !defined(__CYGWIN__) // NOBOOK
} ;
#else // NOBOOK
} __attribute__ ((aligned(16)));
#endif // NOBOOK

#endif // LUX_MEMORY_H

