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
 *   SmallLuxGPU is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 ***************************************************************************/

#ifndef _RENDERTHREAD_H
#define	_RENDERTHREAD_H

#include "scene.h"
#include "raybuffer.h"
#include "samplebuffer.h"
#include "intersectiondevice.h"

#include <boost/thread/thread.hpp>

using namespace std;

#define DEVICE_RENDER_BUFFER_COUNT 4

class RenderThread {
public:
	RenderThread(unsigned int index, Scene *scn);
	virtual ~RenderThread();

	virtual void Start() { started = true; }
    virtual void Interrupt() = 0;
	virtual void Stop() { started = false; }

	virtual void ClearPaths() = 0;
	virtual unsigned int GetPass() const = 0;

protected:
	unsigned int threadIndex;
	Scene *scene;

	bool started;
};

class NativeRenderThread : public RenderThread {
public:
	NativeRenderThread(unsigned int index, NativeIntersectionDevice *device, Scene *scn,
			const bool lowLatency);
	~NativeRenderThread();

	void Start();
    void Interrupt();
	void Stop();

	void ClearPaths();

	unsigned int GetPass() const { return sampler->GetPass(); }

private:
	static void RenderThreadImpl(NativeRenderThread *renderThread);

	NativeIntersectionDevice *intersectionDevice;

	Sampler *sampler;
	PathIntegrator *pathIntegrator;
	RayBuffer *rayBuffer;
	SampleBuffer *sampleBuffer;

	boost::thread *renderThread;
};

class DeviceRenderThread : public RenderThread {
public:
	DeviceRenderThread(unsigned int index, IntersectionDevice *device, Scene *scn,
			const bool lowLatency);
	~DeviceRenderThread();

	void Start();
    void Interrupt();
	void Stop();

	void ClearPaths();

	unsigned int GetPass() const { return sampler->GetPass(); }

private:
	static void RenderThreadImpl(DeviceRenderThread *renderThread);

	IntersectionDevice *intersectionDevice;

	Sampler *sampler;
	PathIntegrator *pathIntegrators[DEVICE_RENDER_BUFFER_COUNT];
	RayBuffer *rayBuffers[DEVICE_RENDER_BUFFER_COUNT];
	SampleBuffer *sampleBuffer;

	boost::thread *renderThread;
};

#endif	/* _RENDERTHREAD_H */
