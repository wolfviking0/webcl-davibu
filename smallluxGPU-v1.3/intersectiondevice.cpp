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

#include <cstring>
#include <limits>

#include "renderthread.h"
#include "samplebuffer.h"
#include "displayfunc.h"

//------------------------------------------------------------------------------
// IntersectionDevice
//------------------------------------------------------------------------------

IntersectionDevice::IntersectionDevice(Scene *scn, const unsigned int index) {
	deviceIndex = index;
	scene = scn;
	statsTotalRayCount = 0.0;
	statsStartTime = WallClockTime();
	started = false;
}

IntersectionDevice::~IntersectionDevice() {
}

double IntersectionDevice::GetPerformance() const {
	double statsTotalRayTime = WallClockTime() - statsStartTime;
	return (statsTotalRayTime == 0.0) ?	1.0 : (statsTotalRayCount / statsTotalRayTime);
}

//------------------------------------------------------------------------------
// NativeIntersectionDevice
//------------------------------------------------------------------------------

NativeIntersectionDevice::NativeIntersectionDevice(Scene *scn, const bool lowLatency,
		const unsigned int index) : IntersectionDevice(scn, index) {
	char buff[64];
	sprintf(buff, "Thread-%03d", deviceIndex);
	deviceName = string(buff);
}

NativeIntersectionDevice::~NativeIntersectionDevice() {
}

void NativeIntersectionDevice::Start() {
	started = true;
}

void NativeIntersectionDevice::Interrupt() {
}

void NativeIntersectionDevice::Stop() {
	started = false;
	while (!doneRayBufferQueue.empty())
		doneRayBufferQueue.pop();
}

void NativeIntersectionDevice::PushRayBuffer(RayBuffer *rayBuffer) {
	this->TraceRays(rayBuffer);

	doneRayBufferQueue.push(rayBuffer);
}

void NativeIntersectionDevice::TraceRays(RayBuffer *rayBuffer) {
	const Ray *rb = rayBuffer->GetRayBuffer();
	RayHit *hb = rayBuffer->GetHitBuffer();
	const size_t rayCount = rayBuffer->GetRayCount();
	for (unsigned int i = 0; i < rayCount; ++i)
		scene->Intersect(rb[i], &hb[i]);

	statsTotalRayCount += rayCount;
}

RayBuffer *NativeIntersectionDevice::PopRayBuffer() {
	RayBuffer *result = doneRayBufferQueue.front();
	doneRayBufferQueue.pop();

	return result;
}

//------------------------------------------------------------------------------
// OpenCLIntersectionDevice
//------------------------------------------------------------------------------

OpenCLIntersectionDevice::OpenCLIntersectionDevice(Scene *scn, const bool lowLatency,
	unsigned int index, const cl::Device &device,
	const unsigned int forceGPUWorkSize) :
	IntersectionDevice(scn, index) {
	deviceName = device.getInfo<CL_DEVICE_NAME > ().c_str();

	// Allocate a context with the selected device
	cl::Platform platform = device.getInfo<CL_DEVICE_PLATFORM>();
	VECTOR_CLASS<cl::Device> devices;
	devices.push_back(device);
	cl_context_properties cps[3] = {
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform(), 0
	};
	context = new cl::Context(devices, cps);

	// Allocate the queue for this device
	queue = new cl::CommandQueue(*context, device);

	//--------------------------------------------------------------------------
	// Allocate buffers

	const size_t rayBufferSize = lowLatency ? (RAY_BUFFER_SIZE / 8) : RAY_BUFFER_SIZE;

	cerr << "[Device::" << deviceName << "] rays buffer size: " << (sizeof(Ray) * rayBufferSize / 1024) << "Kb" <<endl;
	raysBuff = new cl::Buffer(*context,
			CL_MEM_READ_ONLY,
			sizeof(Ray) * rayBufferSize);
	cerr << "[Device::" << deviceName << "] ray hits buffer size: " << (sizeof(RayHit) * rayBufferSize / 1024) << "Kb" <<endl;
	hitsBuff = new cl::Buffer(*context,
			CL_MEM_WRITE_ONLY,
			sizeof(RayHit) * rayBufferSize);

	cerr << "[Device::" << deviceName << "] QBVH buffer size: " << (sizeof(QBVHNode) * scene->qbvh->nNodes / 1024) << "Kb" <<endl;
	qbvhBuff = new cl::Buffer(*context,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(QBVHNode) * scene->qbvh->nNodes,
			scene->qbvh->nodes);

	cerr << "[Device::" << deviceName << "] QuadTriangle buffer size: " << (sizeof(QuadTriangle) * scene->qbvh->nQuads / 1024) << "Kb" <<endl;
	qbvhTrisBuff = new cl::Buffer(*context,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(QuadTriangle) * scene->qbvh->nQuads,
			scene->qbvh->prims);

	//--------------------------------------------------------------------------
	// QBVH kernel
	//--------------------------------------------------------------------------

	bvhKernel = SetUpKernel(deviceName, "Intersect", *context, device, "qbvh_kernel.cl");
	bvhKernel->getWorkGroupInfo<size_t>(device, CL_KERNEL_WORK_GROUP_SIZE, &qbvhWorkGroupSize);
	cerr << "[Device::" << deviceName << "] QBVH kernel work group size: " << qbvhWorkGroupSize << endl;
	cl_ulong memSize;
	bvhKernel->getWorkGroupInfo<cl_ulong>(device, CL_KERNEL_LOCAL_MEM_SIZE, &memSize);
	cerr << "[Device::" << deviceName << "] QBVH kernel memory footprint: " << memSize << endl;

	bvhKernel->getWorkGroupInfo<size_t>(device, CL_KERNEL_WORK_GROUP_SIZE, &qbvhWorkGroupSize);
	cerr << "[Device::" << deviceName << "]" << " Suggested work group size: " << qbvhWorkGroupSize << endl;

	// Force workgroup size if applicable and required
	if ((forceGPUWorkSize > 0) && (device.getInfo<CL_DEVICE_TYPE>() == CL_DEVICE_TYPE_GPU)) {
		qbvhWorkGroupSize = forceGPUWorkSize;
		cerr << "[Device::" << deviceName << "]" << " Forced work group size for QBVH: " << qbvhWorkGroupSize << endl;
	}

	// Set Arguments
	bvhKernel->setArg(0, *raysBuff);
	bvhKernel->setArg(1, *hitsBuff);
	bvhKernel->setArg(2, *qbvhBuff);
	bvhKernel->setArg(3, *qbvhTrisBuff);

	rayIntersectionThread = NULL;
}

OpenCLIntersectionDevice::~OpenCLIntersectionDevice() {
	if (started)
		Stop();

	delete bvhKernel;

	delete raysBuff;
	delete hitsBuff;
	delete qbvhBuff;
	delete qbvhTrisBuff;

	delete queue;
	delete context;
}

void OpenCLIntersectionDevice::Start() {
	started = true;

	statsDeviceIdleTime = 0.0;
	statsDeviceTotalTime = 0.0;

	// Create the thread for the ray intersections
	rayIntersectionThread = new boost::thread(boost::bind(OpenCLIntersectionDevice::RayIntersectionThread, this));
}

void OpenCLIntersectionDevice::Interrupt() {
	if (rayIntersectionThread)
		rayIntersectionThread->interrupt();
}

void OpenCLIntersectionDevice::Stop() {
	started = false;

	if (rayIntersectionThread) {
		rayIntersectionThread->interrupt();
		rayIntersectionThread->join();
		delete rayIntersectionThread;
		rayIntersectionThread = NULL;
	}

	todoRayBufferQueue.Clear();
	doneRayBufferQueue.Clear();
}

void OpenCLIntersectionDevice::PushRayBuffer(RayBuffer *rayBuffer) {
	todoRayBufferQueue.Push(rayBuffer);
}

RayBuffer *OpenCLIntersectionDevice::PopRayBuffer() {
	return doneRayBufferQueue.Pop();
}

void OpenCLIntersectionDevice::RayIntersectionThread(OpenCLIntersectionDevice *intersectionDevice) {
	cerr << "[Device::" << intersectionDevice->GetName() << "] RayIntersection thread started" << endl;

	try {
		while (!boost::this_thread::interruption_requested()) {
			const double t1 = WallClockTime();
			RayBuffer *rayBuffer = intersectionDevice->todoRayBufferQueue.Pop();
			const double t2 = WallClockTime();

			// Trace rays

			// Download the rays to the GPU
			cl::Event writeBufferEvent;
			intersectionDevice->queue->enqueueWriteBuffer(
					*(intersectionDevice->raysBuff),
					CL_FALSE,
					0,
					sizeof(Ray) * rayBuffer->GetRayCount(),
					rayBuffer->GetRayBuffer(), NULL, &writeBufferEvent);

			intersectionDevice->bvhKernel->setArg(4, (unsigned int)rayBuffer->GetRayCount());
			cl::Event kernelEvent;
			VECTOR_CLASS<cl::Event> kernelWaitEvents(1, writeBufferEvent);
			intersectionDevice->queue->enqueueNDRangeKernel(*(intersectionDevice->bvhKernel), cl::NullRange,
					cl::NDRange(rayBuffer->GetSize()), cl::NDRange(intersectionDevice->qbvhWorkGroupSize),
					&kernelWaitEvents, &kernelEvent);

			// Upload the results
			VECTOR_CLASS<cl::Event> readBufferWaitEvents(1, kernelEvent);
			intersectionDevice->queue->enqueueReadBuffer(
					*(intersectionDevice->hitsBuff),
					CL_TRUE,
					0,
					sizeof(RayHit) * rayBuffer->GetRayCount(),
					rayBuffer->GetHitBuffer(),
					&readBufferWaitEvents);

			const double t3 = WallClockTime();

			intersectionDevice->statsDeviceIdleTime += t2 - t1;
			intersectionDevice->statsDeviceTotalTime += t3 - t1;
			intersectionDevice->statsTotalRayCount += rayBuffer->GetRayCount();

			intersectionDevice->doneRayBufferQueue.Push(rayBuffer);
		}

		cerr << "[Device::" << intersectionDevice->GetName() << "] RayIntersection thread halted" << endl;
	} catch (boost::thread_interrupted) {
		cerr << "[Device::" << intersectionDevice->GetName() << "] RayIntersection thread halted" << endl;
	} catch (cl::Error err) {
		cerr << "[Device::" << intersectionDevice->GetName() << "] RayIntersection ERROR: " << err.what() << "(" << err.err() << ")" << endl;
	}
}

//------------------------------------------------------------------------------
// VirtualM2MIntersectionDevice
//------------------------------------------------------------------------------

VirtualM2OIntersectionDevice::VirtualM2OIntersectionDevice(const size_t count,
		IntersectionDevice *device, Scene *scn) {
	virtualDeviceCount = count;
	realDevice = device;
	scene = scn;

	virtualDeviceInstances = new VirtualM2ODevInstance *[virtualDeviceCount];
	for (size_t i = 0; i < virtualDeviceCount; ++i)
		virtualDeviceInstances[i] = new VirtualM2ODevInstance(this, i);

	// Start the RayBuffer routing thread
	routerThread = new boost::thread(boost::bind(VirtualM2OIntersectionDevice::RayBufferRouter, this));
}

VirtualM2OIntersectionDevice::~VirtualM2OIntersectionDevice() {
	// Stop the router thread
	routerThread->interrupt();
	routerThread->join();

	for (size_t i = 0; i < virtualDeviceCount; ++i)
		delete virtualDeviceInstances[i];
	delete virtualDeviceInstances;
}

IntersectionDevice *VirtualM2OIntersectionDevice::GetVirtualDevice(size_t index) {
	return virtualDeviceInstances[index];
}

void VirtualM2OIntersectionDevice::RayBufferRouter(VirtualM2OIntersectionDevice *virtualDevice) {
	try {
		while (!boost::this_thread::interruption_requested()) {
			RayBuffer *rayBuffer = virtualDevice->realDevice->PopRayBuffer();

			size_t instanceIndex = rayBuffer->GetUserData();
			virtualDevice->virtualDeviceInstances[instanceIndex]->PushRayBufferDone(rayBuffer);
		}
	} catch (boost::thread_interrupted) {
		// Time to exit
	} catch (cl::Error err) {
		cerr << "[VirtualM2ODevice::" << virtualDevice->realDevice->GetName() << "] RayBufferRouter thread ERROR: " << err.what() << "(" << err.err() << ")" << endl;
	}
}

//------------------------------------------------------------------------------
// VirtualM2OIntersectionDevice class
//------------------------------------------------------------------------------

VirtualM2OIntersectionDevice::VirtualM2ODevInstance::VirtualM2ODevInstance(
	VirtualM2OIntersectionDevice *device, const size_t index) :
			IntersectionDevice(device->scene, index) {
	char buf[64];
	sprintf(buf, "VirtualM2ODevice-%03d-%s", (int)index, device->realDevice->GetName().c_str());
	deviceName = std::string(buf);

	instanceIndex = index;
	virtualDevice = device;
}

VirtualM2OIntersectionDevice::VirtualM2ODevInstance::~VirtualM2ODevInstance() {
	if (started)
		Stop();
}

void VirtualM2OIntersectionDevice::VirtualM2ODevInstance::Start() {
	boost::mutex::scoped_lock lock(virtualDevice->virtualDeviceMutex);

	started = true;
	pendingRayBuffers = 0;

	// Start the real device if required
	if (!virtualDevice->realDevice->IsRunning()) {
		cerr << "[VirtualM2ODevice::" << deviceName << "] Starting real device" << endl;
		virtualDevice->realDevice->Start();
	}
}

void VirtualM2OIntersectionDevice::VirtualM2ODevInstance::Interrupt() {
}

void VirtualM2OIntersectionDevice::VirtualM2ODevInstance::Stop() {
	boost::mutex::scoped_lock lock(virtualDevice->virtualDeviceMutex);

	started = false;

	// Need to wait for all my pending RayBuffer
	while (pendingRayBuffers > 0)
		PopRayBuffer();

	// Check if I'm the last one running
	bool lastOne = true;
	for (size_t i = 0; i < virtualDevice->virtualDeviceCount; ++i) {
		if ((i != instanceIndex) && (virtualDevice->virtualDeviceInstances[i]->IsRunning())) {
			lastOne = false;
			break;
		}
	}

	if (lastOne) {
		cerr << "[VirtualM2ODevice::" << deviceName << "] Stopping real device" << endl;
		virtualDevice->realDevice->Stop();
	}
}

void VirtualM2OIntersectionDevice::VirtualM2ODevInstance::PushRayBuffer(RayBuffer *rayBuffer) {
	rayBuffer->PushUserData(instanceIndex);

	virtualDevice->realDevice->PushRayBuffer(rayBuffer);
	++pendingRayBuffers;
}

RayBuffer *VirtualM2OIntersectionDevice::VirtualM2ODevInstance::PopRayBuffer() {
	RayBuffer *rayBuffer = doneRayBufferQueue.Pop();
	rayBuffer->PopUserData();
	--pendingRayBuffers;

	return rayBuffer;
}

void VirtualM2OIntersectionDevice::VirtualM2ODevInstance::PushRayBufferDone(RayBuffer *rayBuffer) {
	doneRayBufferQueue.Push(rayBuffer);
}

//------------------------------------------------------------------------------
// Virtual One to Many device
//------------------------------------------------------------------------------

VirtualO2MIntersectionDevice::VirtualO2MIntersectionDevice(vector<IntersectionDevice *> devices,
		Scene *scn, const size_t index) : IntersectionDevice(scn, index) {
	char buf[64];
	sprintf(buf, "VirtualO2MDevice-%03d", (int)deviceIndex);
	deviceName = std::string(buf);

	realDevices = devices;
	pusherThread = NULL;
}

VirtualO2MIntersectionDevice::~VirtualO2MIntersectionDevice() {
	if (started)
		Stop();
}

void VirtualO2MIntersectionDevice::Start() {
	started = true;

	// Start all real devices
	for (size_t i = 0; i < realDevices.size(); ++i)
		realDevices[i]->Start();

	// Start the RayBuffer routing threads
	pusherThread = new boost::thread(boost::bind(VirtualO2MIntersectionDevice::PusherRouter, this));
	popperThread.resize(realDevices.size(), NULL);
	for (size_t i = 0; i < realDevices.size(); ++i)
		popperThread[i] = new boost::thread(boost::bind(VirtualO2MIntersectionDevice::PopperRouter, this, i));
}

void VirtualO2MIntersectionDevice::Interrupt() {
	if (pusherThread)
		pusherThread->interrupt();
	if (popperThread.size() > 0) {
		for (size_t i = 0; i < realDevices.size(); ++i)
			popperThread[i]->interrupt();
	}

	for (size_t i = 0; i < realDevices.size(); ++i)
		realDevices[i]->Interrupt();
}

void VirtualO2MIntersectionDevice::Stop() {
	if (pusherThread) {
		pusherThread->interrupt();
		pusherThread->join();
		delete pusherThread;
		pusherThread = NULL;
	}

	if (popperThread.size() > 0) {
		for (size_t i = 0; i < realDevices.size(); ++i) {
			popperThread[i]->interrupt();
			popperThread[i]->join();
			delete popperThread[i];
		}

		popperThread.clear();
	}

	if (started) {
		for (size_t i = 0; i < realDevices.size(); ++i)
			realDevices[i]->Stop();
	}

	started = false;
}

void VirtualO2MIntersectionDevice::PushRayBuffer(RayBuffer *rayBuffer) {
	todoRayBufferQueue.Push(rayBuffer);
}

RayBuffer *VirtualO2MIntersectionDevice::PopRayBuffer() {
	return doneRayBufferQueue.Pop();
}

void VirtualO2MIntersectionDevice::PusherRouter(VirtualO2MIntersectionDevice *virtualDevice) {
	try {
		while (!boost::this_thread::interruption_requested()) {
			RayBuffer *rayBuffer = virtualDevice->todoRayBufferQueue.Pop();

			// Look for the device with less work to do
			size_t index = 0;
// Windows crap ...
#undef max
			size_t minQueueSize = numeric_limits<size_t>::max();
			for (size_t i = 0; i < virtualDevice->realDevices.size(); ++i) {
				const size_t queueSize = virtualDevice->realDevices[i]->GetQueueSize();
				if (queueSize < minQueueSize) {
					index = i;
					minQueueSize = queueSize;
				}
			}

			virtualDevice->realDevices[index]->PushRayBuffer(rayBuffer);
		}
	} catch (boost::thread_interrupted) {
		// Time to exit
	} catch (cl::Error err) {
		cerr << "[VirtualO2MDevice::" << virtualDevice->deviceIndex << "] PusherRouter thread ERROR: " << err.what() << "(" << err.err() << ")" << endl;
	}
}

void VirtualO2MIntersectionDevice::PopperRouter(VirtualO2MIntersectionDevice *virtualDevice, size_t deviceIndex) {
	try {
		IntersectionDevice *device = virtualDevice->realDevices[deviceIndex];

		while (!boost::this_thread::interruption_requested()) {
			RayBuffer *rayBuffer = device->PopRayBuffer();

			virtualDevice->doneRayBufferQueue.Push(rayBuffer);
		}
	} catch (boost::thread_interrupted) {
		// Time to exit
	} catch (cl::Error err) {
		cerr << "[VirtualO2MDevice::" << virtualDevice->deviceIndex << "::" << deviceIndex << "] PopperRouter thread ERROR: " << err.what() << "(" << err.err() << ")" << endl;
	}
}
