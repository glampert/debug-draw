
// ================================================================================================
// -*- C++ -*-
// File: defines.h
// Author: Guilherme R. Lampert
// Created on: 22/03/13
// Brief: Added this header with some defines to make the SSE library Unix friendly.
// ================================================================================================

#ifndef _VECTORMATH_DEFINES_H
#define _VECTORMATH_DEFINES_H

#if defined (_MSC_VER)
	// Visual Studio (MS compiler)
	#define _VECTORMATH_ALIGNED(type)  __declspec(align(16)) type
	#define _VECTORMATH_ALIGNED_TYPE_1 __declspec(align(16))
	#define _VECTORMATH_ALIGNED_TYPE_2
#elif defined (__GNUC__)
	// GCC
	#define _VECTORMATH_ALIGNED(type) type __attribute__((aligned(16)))
	#define _VECTORMATH_ALIGNED_TYPE_1
	#define _VECTORMATH_ALIGNED_TYPE_2 __attribute__((aligned(16)))
#else
	// Unknown
	#error "Define _VECTORMATH_ALIGNED for your compiler"
#endif

#endif // _VECTORMATH_DEFINES_H
