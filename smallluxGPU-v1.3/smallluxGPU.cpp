/*
Copyright (c) 2009 David Bucciarelli (davibu@interfree.it)

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>

#include <boost/thread.hpp>

#include "displayfunc.h"
#include "path.h"
#include "intersectiondevice.h"

static int BatchMode(double stopTime) {
	const double startTime = WallClockTime();

	double sampleSec = 0.0;
	char buff[512];
	for (;;) {
		boost::this_thread::sleep(boost::posix_time::millisec(1000));
		double elapsedTime = WallClockTime() - startTime;
		if (elapsedTime > stopTime)
			break;

		double raysSec = 0.0;
		const vector<IntersectionDevice *> interscetionDevices = config->GetIntersectionDevices();
		for (size_t i = 0; i < interscetionDevices.size(); ++i)
			raysSec += interscetionDevices[i]->GetPerformance();

		sampleSec = config->scene->camera->film->GetAvgSampleSec();
		sprintf(buff, "[Elapsed time: %3d/%dsec][Avg. samples/sec % 4dK][Avg. rays/sec % 4dK on %.1fK tris]",
				int(elapsedTime), int(stopTime), int(sampleSec/ 1000.0),
				int(raysSec / 1000.0), config->scene->mesh->triangleCount / 1000.0);
		std::cerr << buff << std::endl;

	}

	std::cerr << "Saving image.ppm" << std::endl;
	config->scene->camera->film->SavePPM("image.ppm");

	sprintf(buff, "LuxMark index: %.3f", sampleSec / 1000000.0);
	std::cerr << buff << std::endl;

	delete config;
	std::cerr << "Done." << std::endl;

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
	try {
		std::cerr << "Usage (easy mode): " << argv[0] << std::endl;
		std::cerr << "Usage (benchmark mode): " << argv[0] << " <native thread count> <use CPU device (0 or 1)> <use GPU device (0 or 1)> <GPU workgroup size (0=default value or anything > 0)>" << std::endl;
		std::cerr << "Usage (interactive mode): " << argv[0] << " <low latency mode enabled (0 or 1)> <native thread count> <use CPU device (0 or 1)> <use GPU device (0 or 1)> <GPU workgroup size (0=default value or anything > 0)> <window width> <window height> <scene file>" << std::endl;
		std::cerr << "Usage (batch mode): " << argv[0] << " <low latency mode enabled (0 or 1)> <native thread count> <use CPU device (0 or 1)> <use GPU device (0 or 1)> <GPU workgroup size (0=default value or anything > 0)> <window width> <window height> <halt time in secs> <scene file>" << std::endl;
		std::cerr << "Usage (with configuration file mode): " << argv[0] << " <configuration file name>" << std::endl;

		// It is important to initialize OpenGL before OpenCL
		unsigned int width;
		unsigned int height;
		if (argc == 10) {
			width = atoi(argv[6]);
			height = atoi(argv[7]);
			const bool lowLatencyMode = (atoi(argv[1]) == 1);
			config = new RenderingConfig(lowLatencyMode, argv[9], width, height, atoi(argv[2]), (atoi(argv[3]) == 1), (atoi(argv[4]) == 1), atoi(argv[5]));
			return BatchMode(atoi(argv[8]));
		} else if (argc == 9) {
			width = atoi(argv[6]);
			height = atoi(argv[7]);
		} else if (argc == 5) {
			config = new RenderingConfig(false, "scenes/luxball.scn", 384, 384, atoi(argv[1]), (atoi(argv[2]) == 1), (atoi(argv[3]) == 1), atoi(argv[4]));
			return BatchMode(180.0);
		} else if (argc == 2) {
			config = new RenderingConfig(argv[1]);
			width = atoi(config->cfg.find("image.width")->second.c_str());
			height = atoi(config->cfg.find("image.height")->second.c_str());

			const unsigned int halttime = atoi(config->cfg.find("batch.halttime")->second.c_str());
			if (halttime > 0) {
				config->Init();
				return BatchMode(halttime);
			}
		} else  if (argc == 1) {
			width = 640;
			height = 480;
		} else
			exit(-1);

		InitGlut(argc, argv, width, height);

		if (argc == 9) {
			const bool lowLatencyMode = (atoi(argv[1]) == 1);
			config = new RenderingConfig(lowLatencyMode, argv[8], width, height, atoi(argv[2]), (atoi(argv[3]) == 1), (atoi(argv[4]) == 1), atoi(argv[5]));

			if (!lowLatencyMode)
				config->screenRefreshInterval = 2000;
		} else if (argc == 2) {
			config->Init();
		} else if (argc == 1)
			config = new RenderingConfig(true, "scenes/simple.scn", width, height, boost::thread::hardware_concurrency(), false, true, 0);
		else
			exit(-1);

		RunGlut();
	} catch (cl::Error err) {
		std::cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << std::endl;
	}

	return EXIT_SUCCESS;
}
