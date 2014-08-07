/* 
 * This file is part of dcaenc.
 *
 * Copyright (c) 2008-2012 Alexander E. Patrakov <patrakov@gmail.com>
 *
 * dcaenc is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * dcaenc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with dcaenc; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef COMPILER_INFO_H
#define COMPILER_INFO_H

#ifndef _T
#define __T(X) #X
#define _T(X) __T(X)
#endif

#if defined(__INTEL_COMPILER)
	#if (__INTEL_COMPILER >= 1200)
		#define __COMPILER__ "ICL 12.x"
	#elif (__INTEL_COMPILER >= 1100)
		#define __COMPILER__ = "ICL 11.x"
	#elif (__INTEL_COMPILER >= 1000)
		#define __COMPILER__ = "ICL 10.x"
	#else
		#define __COMPILER__ "ICL Unknown"
	#endif
#elif defined(_MSC_VER)
	#if (_MSC_VER == 1600)
		#if (_MSC_FULL_VER >= 160040219)
			#define __COMPILER__ "MSVC 2010-SP1"
		#else
			#define __COMPILER__ "MSVC 2010"
		#endif
	#elif (_MSC_VER == 1500)
		#if (_MSC_FULL_VER >= 150030729)
			#define __COMPILER__ "MSVC 2008-SP1"
		#else
			#define __COMPILER__ "MSVC 2008"
		#endif
	#else
		#define __COMPILER__ "MSVC Unknown"
	#endif
#elif defined(__GNUC__)
	#define __COMPILER__ "GNU GCC " _T(__GNUC__) "." _T(__GNUC_MINOR__) "." _T(__GNUC_PATCHLEVEL__)
#else
	#define __COMPILER__ "Unknown"
#endif

#endif
