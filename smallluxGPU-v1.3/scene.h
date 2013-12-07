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

#ifndef _SCENE_H
#define	_SCENE_H

#include <string>
#include <iostream>
#include <fstream>

#include "point.h"
#include "normal.h"
#include "spectrum.h"
#include "camera.h"
#include "triangle.h"
#include "qbvhaccel.h"
#include "ray.h"
#include "mesh.h"
#include "light.h"

using namespace std;

class Scene {
public:
	Scene(const bool lowLatency, const string &fileName, Film *film);
	~Scene() {
		delete camera;
		delete[] lights;
		delete mesh;
		delete qbvh;
	}

	void Intersect(const Ray &ray, RayHit *hit) const {
		hit->t = INFINITY;
		hit->index = 0xffffffffu;
		qbvh->Intersect(ray, hit);
	}

	unsigned int SampleLights(const float u) const {
		// One Uniform light strategy
		const unsigned int lightIndex = min(Floor2UInt(nLights * u), nLights - 1);

		return lightIndex;
	}

	bool IsLight(const unsigned int index) const {
		return (index >= meshLightOffset);
	}

	// Siggned because of the delta parameter
	int maxPathDepth;
	unsigned int shadowRayCount;

	PerspectiveCamera *camera;

	unsigned int nLights;
	unsigned int meshLightOffset;
	TriangleMesh *mesh;
	TriangleLight *lights;

	QBVHAccel *qbvh;
};

#endif	/* _SCENE_H */
