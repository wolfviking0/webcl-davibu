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

#ifndef _LIGHT_H
#define	_LIGHT_H

#include "point.h"
#include "spectrum.h"
#include "mesh.h"

class TriangleLight {
public:
	TriangleLight() { }

	TriangleLight(const unsigned int index, const TriangleMesh *objs) {
		triIndex = index;
		area = objs->triangles[triIndex].Area(objs->vertices);
	}

	Spectrum Sample_L(const TriangleMesh *objs, const Point &p, const Normal &N,
		const float u0, const float u1, float *pdf, Ray *shadowRay) const {
		const Triangle &tri = objs->triangles[triIndex];

		Point samplePoint;
		float b0, b1, b2;
		tri.Sample(objs->vertices, u0, u1, &samplePoint, &b0, &b1, &b2);
		Normal sampleN = objs->vertNormals[tri.v[0]]; // Light sources are supposed to be flat

		Vector wi = samplePoint - p;
		const float distanceSquared = wi.LengthSquared();
		const float distance = sqrtf(distanceSquared);
		wi /= distance;

		float SampleNdotMinusWi = Dot(sampleN, -wi);
		float NdotMinusWi = Dot(N, wi);
		if ((SampleNdotMinusWi <= 0.f) || (NdotMinusWi <= 0.f)) {
			*pdf = 0.f;
			return Spectrum(0.f, 0.f, 0.f);
		}

		*shadowRay = Ray(p, wi, RAY_EPSILON, distance - RAY_EPSILON);
		*pdf = distanceSquared / (SampleNdotMinusWi * NdotMinusWi * area);

		// Return interpolated color
		return tri.InterpolateColor(objs->vertColors, b0, b1, b2);
	}

private:
	unsigned int triIndex;
	float area;

};

#endif	/* _LIGHT_H */

