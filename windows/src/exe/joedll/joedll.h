/*
 *  This file is part of Joe's Own Editor for Windows.
 *  Copyright (c) 2014 John J. Jordan.
 *
 *  Joe's Own Editor for Windows is free software: you can redistribute it 
 *  and/or modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation, either version 2 of the 
 *  License, or (at your option) any later version.
 *
 *  Joe's Own Editor for Windows is distributed in the hope that it will
 *  be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Joe's Own Editor for Windows.  If not, see
 *  <http://www.gnu.org/licenses/>.
 */

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the JOEDLL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// JOEDLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef JOEDLL_EXPORTS
#define JOEDLL_API __declspec(dllexport)
#else
#define JOEDLL_API __declspec(dllimport)
#endif

JOEDLL_API int joewinmain(HINSTANCE hInstance, HINSTANCE hPrevInstance, int nCmdShow, int argc, wchar_t** argv);
