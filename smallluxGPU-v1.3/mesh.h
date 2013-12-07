/***************************************************************************
 *   Copyright (C) 1998-2009 by David Bucciarelli (davibu@interfree.it)    *
 *                                                                         *
 *   This file is part of SmallLuxGPU.                                     *
 *                                                                         *
 *   SmallLuxGPU is free software; you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  SmallLuxGPU is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 *   This project is based on PBRT ; see http://www.pbrt.org               *
 *   and Lux Renderer website : http://www.luxrender.net                   *
 ***************************************************************************/

#ifndef _PLYOBJECT_H
#define	_PLYOBJECT_H

#include "point.h"
#include "normal.h"
#include "spectrum.h"
#include "triangle.h"
#include "plymesh/rply.h"

class TriangleMesh {
public:
	TriangleMesh(const string &fileName);
	TriangleMesh(const TriangleMesh &tm0, const TriangleMesh &tm1);
	~TriangleMesh();

	unsigned int vertexCount;
	unsigned int triangleCount;
	Point *vertices;
	Normal *vertNormals;
	Spectrum *vertColors;
	Triangle *triangles;
};

#endif	/* _PLYOBJECT_H */

