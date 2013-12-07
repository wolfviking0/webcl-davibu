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

#ifndef _INTERSECTIONDEVICE_H
#define	_INTERSECTIONDEVICE_H

#include <queue>

#include "smalllux.h"
#include "raybuffer.h"

class IntersectionDevice {
public:
	IntersectionDevice(Scene *scene, const unsigned int index);
	virtual ~IntersectionDevice();

	virtual void Start() = 0;
	virtual void Interrupt() = 0;
	virtual void Stop() = 0;
	virtual bool IsRunning() const { return started; };

	virtual void PushRayBuffer(RayBuffer *rayBuffer) = 0;
	virtual RayBuffer *PopRayBuffer() = 0;
	virtual size_t GetQueueSize() = 0;

	const string &GetName() const { return deviceName; }

	double GetPerformance() const;

	virtual double GetLoad() const = 0;

protected:
	string deviceName;
	unsigned int deviceIndex;

	Scene *scene;

	// Execution profiling
	double statsTotalRayCount;
	double statsStartTime;

	bool started;
};

class NativeIntersectionDevice : public IntersectionDevice {
public:
	NativeIntersectionDevice(Scene *scene, const bool lowLatency, const unsigned int index);
	~NativeIntersectionDevice();

	void Start();
	void Interrupt();
	void Stop();

	void PushRayBuffer(RayBuffer *rayBuffer);
	RayBuffer *PopRayBuffer();
	size_t GetQueueSize() { return 0; }

	double GetLoad() const {
		return 1.0;
	}

	// A short-cut
	void TraceRays(RayBuffer *rayBuffer);

private:
	queue<RayBuffer *> doneRayBufferQueue;
};

class OpenCLIntersectionDevice : public IntersectionDevice {
public:
	OpenCLIntersectionDevice(Scene *scene, const bool lowLatency, unsigned int index,
			const cl::Device &dev, const unsigned int forceGPUWorkSize);
	~OpenCLIntersectionDevice();

	void Start();
	void Interrupt();
	void Stop();

	void PushRayBuffer(RayBuffer *rayBuffer);
	RayBuffer *PopRayBuffer();
	size_t GetQueueSize() { return todoRayBufferQueue.Size(); }

	double GetLoad() const {
		return (statsDeviceTotalTime == 0.0) ? 0.0 : (1.0 - statsDeviceIdleTime / statsDeviceTotalTime);
	}

private:
	static void RayIntersectionThread(OpenCLIntersectionDevice *intersectionDevice);

	boost::thread *rayIntersectionThread;
	RayBufferQueue todoRayBufferQueue;
	RayBufferQueue doneRayBufferQueue;

	// OpenCL fields
	cl::Context *context;
	cl::CommandQueue *queue;

	cl::Kernel *bvhKernel;
	size_t qbvhWorkGroupSize;

	// Buffers
	cl::Buffer *raysBuff;
	cl::Buffer *hitsBuff;
	cl::Buffer *qbvhBuff;
	cl::Buffer *qbvhTrisBuff;

	double statsDeviceIdleTime;
	double statsDeviceTotalTime;
};

//------------------------------------------------------------------------------
// Virtual Many to One device
//------------------------------------------------------------------------------

class VirtualM2OIntersectionDevice {
public:
	VirtualM2OIntersectionDevice(const size_t count, IntersectionDevice *device, Scene *scn);
	~VirtualM2OIntersectionDevice();

	IntersectionDevice *GetVirtualDevice(size_t index);

private:
	class VirtualM2ODevInstance : public IntersectionDevice {
	public:
		VirtualM2ODevInstance(VirtualM2OIntersectionDevice * device, const size_t index);
		~VirtualM2ODevInstance();

		void Start();
		void Interrupt();
		void Stop();

		void PushRayBuffer(RayBuffer *rayBuffer);
		RayBuffer *PopRayBuffer();
		size_t GetQueueSize() { return virtualDevice->realDevice->GetQueueSize(); }

		void PushRayBufferDone(RayBuffer *rayBuffer);

		double GetLoad() const { return 1.0; }

	private:
		size_t instanceIndex;
		VirtualM2OIntersectionDevice *virtualDevice;

		size_t pendingRayBuffers;
		RayBufferQueue doneRayBufferQueue;
	};

	static void RayBufferRouter(VirtualM2OIntersectionDevice *virtualDevice);

	size_t virtualDeviceCount;
	IntersectionDevice *realDevice;
	Scene *scene;

	boost::mutex virtualDeviceMutex;
	VirtualM2ODevInstance **virtualDeviceInstances;

	boost::thread *routerThread;
};

//------------------------------------------------------------------------------
// Virtual One to Many device
//------------------------------------------------------------------------------

class VirtualO2MIntersectionDevice : public IntersectionDevice {
public:
	VirtualO2MIntersectionDevice(vector<IntersectionDevice *> devices, Scene *scn,
			const size_t index);
	~VirtualO2MIntersectionDevice();

	void Start();
	void Interrupt();
	void Stop();

	void PushRayBuffer(RayBuffer *rayBuffer);
	RayBuffer *PopRayBuffer();
	size_t GetQueueSize() { return todoRayBufferQueue.Size(); }

	double GetLoad() const { return 1.0; }

private:
	// Funny names ...
	static void PusherRouter(VirtualO2MIntersectionDevice *virtualDevice);
	static void PopperRouter(VirtualO2MIntersectionDevice *virtualDevice, size_t deviceIndex);

	vector<IntersectionDevice *> realDevices;
	Scene *scene;

	RayBufferQueue todoRayBufferQueue;
	RayBufferQueue doneRayBufferQueue;

	boost::thread *pusherThread;
	vector<boost::thread *> popperThread;
};

#endif	/* _INTERSECTIONDEVICE_H */
