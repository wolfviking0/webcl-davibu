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

#include <stdlib.h>

#include <istream>
#include <stdexcept>
#include <sstream>

#include "scene.h"

using namespace std;

Scene::Scene(const bool lowLatency, const string &fileName, Film *film) {
	maxPathDepth = 3;
	shadowRayCount = 1;

	cerr << "Reading scene: " << fileName << endl;

	ifstream file;
	file.exceptions(ifstream::eofbit | ifstream::failbit | ifstream::badbit);
	file.open(fileName.c_str(), ios::in);

	//--------------------------------------------------------------------------
	// Read light position and radius
	//--------------------------------------------------------------------------

	Spectrum lightGain;
	file >> lightGain.r;
	file >> lightGain.g;
	file >> lightGain.b;

	cerr << "Light gain: (" << lightGain.r << ", " << lightGain.g << ", " << lightGain.b << ")" << endl;

	//--------------------------------------------------------------------------
	// Read camera position and target
	//--------------------------------------------------------------------------

	Point o;
	file >> o.x;
	file >> o.y;
	file >> o.z;

	Point t;
	file >> t.x;
	file >> t.y;
	file >> t.z;

	cerr << "Camera postion: " << o << endl;
	cerr << "Camera target: " << t << endl;

	camera = new PerspectiveCamera(lowLatency, o, t, film);

	//--------------------------------------------------------------------------
	// Read objects .ply file
	//--------------------------------------------------------------------------

	string plyFileName;
	file >> plyFileName;
	cerr << "PLY objects file name: " << plyFileName << endl;

	TriangleMesh objects(plyFileName);

	//--------------------------------------------------------------------------
	// Read lights .ply file
	//--------------------------------------------------------------------------

	file >> plyFileName;
	cerr << "PLY lights file name: " << plyFileName << endl;

	TriangleMesh meshLights(plyFileName);
	// Scale lights intensity
	for (unsigned int i = 0; i < meshLights.vertexCount; ++i)
		meshLights.vertColors[i] *= lightGain;

	//--------------------------------------------------------------------------
	// Join the ply objects
	//--------------------------------------------------------------------------

	meshLightOffset = objects.triangleCount;
	mesh = new TriangleMesh(objects, meshLights);

	cerr << "Vertex count: " << mesh->vertexCount << " (" << (mesh->vertexCount * sizeof(Point) / 1024) << "Kb)" << endl;
	cerr << "Triangle count: " << mesh->triangleCount << " (" << (mesh->triangleCount * sizeof(Triangle) / 1024) << "Kb)" << endl;

	//--------------------------------------------------------------------------
	// Create light sources list
	//--------------------------------------------------------------------------

	nLights = mesh->triangleCount - meshLightOffset;
	lights = new TriangleLight[nLights];
	for (size_t i = 0; i < nLights; ++i)
		new (&lights[i]) TriangleLight(i + meshLightOffset, mesh);


	//--------------------------------------------------------------------------
	// Create BVH
	//--------------------------------------------------------------------------

	const int maxPrimsPerLeaf = 4;
	const int fullSweepThreshold = 4 * maxPrimsPerLeaf;
	const int skipFactor = 1;

	qbvh = new QBVHAccel(mesh->triangleCount, mesh->triangles, mesh->vertices,
			maxPrimsPerLeaf, fullSweepThreshold, skipFactor);
}
