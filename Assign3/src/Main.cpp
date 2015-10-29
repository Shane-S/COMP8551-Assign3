#include <CL/opencl.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <random>
#include "OpenCLHelpers.h"

#undef main

int main() {
	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Event event;
	SDL_Rect r;
	r.w = 1024;
	r.h = 768;
	r.x = 0;
	r.y = 0;

	// create a window to display our procedural texture
	SDL_Window *win = SDL_CreateWindow("SDL_CreateTexture", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 768, SDL_WINDOW_RESIZABLE);
	if (win == nullptr){
		std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}
	
	// create the renderer which will handle drawing to the window surface
	// -1 tells SDL to initalise the first rendering driver that supports the given flags
	SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == nullptr){
		SDL_DestroyWindow(win);
		std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}

	SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 1024, 768);
	if (tex == nullptr){
		SDL_DestroyRenderer(ren);
		SDL_DestroyWindow(win);
		std::cout << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}
	
	CLPlatform* plats;
	cl_uint numPlats;
	if (GetCLPlatforms(&plats, &numPlats) != CL_SUCCESS) {
		std::cout << "Shit" << std::endl;
	}

	cl_context ctx = CreateContext();
	cl_device_id dev;
	cl_command_queue commandQueue = 0;
	cl_program program = 0;
	cl_kernel kernel = 0;
	cl_mem memObjects[2] = { 0, 0 };
	cl_int errNum = 0;

	// Allocate like 3MB of memory to fit our random values
	// Lol
	// There's no rand() in OpenCL C, but we could use one of these techniques from StackOverlow if this becomes an issue
	// http://stackoverflow.com/questions/9912143/how-to-get-a-random-number-in-opencl
	cl_uint *randoms = (cl_uint*)std::malloc(sizeof(cl_uint) * ARRAY_SIZE * 3);
	
	// Get the pixels array for the texture; it will stay the same throughout the life of the program, so we can cache it
	CreateMemObjects(ctx, memObjects, randoms);

	commandQueue = CreateCommandQueue(ctx, &dev);
	program = CreateProgram(ctx, dev, "H:\\MyCourses\\COMP8551\\COMP8551-Assign3\\Assign3\\src\\Random.cl");
	kernel = clCreateKernel(program, "random_kernel", NULL);

	if (errNum != CL_SUCCESS)
	{
		PrintCLError(errNum);
		Cleanup(ctx, commandQueue, program, kernel, memObjects);
		return 1;
	}

	size_t globalWorkSize[1] = { ARRAY_SIZE };
	size_t localWorkSize[1] = { 1 };

	errNum |= clSetKernelArg(kernel, 0, sizeof(cl_mem), &memObjects[0]);
	errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[1]);

	while (1)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT)
			break;

		errNum |= clSetKernelArg(kernel, 0, sizeof(cl_mem), &memObjects[0]);
		errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[1]);

		for (int i = 0; i < ARRAY_SIZE; i++) {
			randoms[i * 3] = std::rand();
			randoms[(i * 3) + 1] = std::rand();
			randoms[(i * 3) + 2] = std::rand();
			//std::cout << randoms[i * 3] << " " << randoms[(i * 3) + 1] << " " << randoms[(i * 3) + 2] << std::endl;
		}
		clEnqueueWriteBuffer(commandQueue, memObjects[0], CL_TRUE, 0, ARRAY_SIZE * sizeof(cl_uint) * 3, randoms, 0, NULL, NULL);

		errNum |= clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL,
			globalWorkSize, localWorkSize,
			0, NULL, NULL);
		if (errNum != CL_SUCCESS)
		{
			std::cerr << "Error: " << CLErrorToString(errNum) << std::endl;
			Cleanup(ctx, commandQueue, program, kernel, memObjects);
			return 1;
		}

		void *pixels;
		int pitch;
		// this will hold our pixel array for manipulation
		if (SDL_LockTexture(tex, NULL, &pixels, &pitch) < 0) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't lock texture: %s\n", SDL_GetError());
			return 1;
		}

		// Read the output buffer back to the Host
		errNum = clEnqueueReadBuffer(commandQueue, memObjects[1], CL_TRUE,
			0, ARRAY_SIZE * sizeof(cl_uint), pixels,
			0, NULL, NULL);
		if (errNum != CL_SUCCESS)
		{
			std::cerr << "Error: " << CLErrorToString(errNum) << std::endl;
			Cleanup(ctx, commandQueue, program, kernel, memObjects);
			return 1;
		}

		SDL_UnlockTexture(tex);

		SDL_RenderClear(ren);
		SDL_RenderCopy(ren, tex, NULL, NULL);
		SDL_RenderPresent(ren);
		SDL_RenderPresent(ren);
	}

	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();

	return 0;
}