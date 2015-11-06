#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <random>
#include "OpenCLHelpers.h"
#include "Gaussian.h"

#define SHANE

#ifdef TREVOR
#define IMAGE_PATH "D:\\Trevor\\Repos\\COMP8551-Assign3\\Assign3\\cat.png"
#define KERNAL_PATH "D:\\Trevor\\Repos\\COMP8551-Assign3\\Assign3\\src\\Filter.cl"
#endif

#ifdef SHANE
#define IMAGE_PATH "D:\\Documents\\School\\BCIT\\Assignments\\Term7\\COMP_8551\\Assign3\\Assign3\\cat.png"
#define KERNAL_PATH "D:\\Documents\\School\\BCIT\\Assignments\\Term7\\COMP_8551\\Assign3\\Assign3\\src\\Filter.cl"
#endif

#undef main

int main() {
	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	int initflags = IMG_INIT_PNG;
	int returnedflags = IMG_Init(initflags);
	if (returnedflags & initflags != initflags) {
		std::cerr << "Shit: " << IMG_GetError() << std::endl;
		return 2;
	}

	// load our cat into a surface
	SDL_Surface *image_raw;
	SDL_Surface *image_argb8888;
	SDL_RWops *rwop;
	rwop = SDL_RWFromFile(IMAGE_PATH, "rb");
	image_raw = IMG_LoadPNG_RW(rwop);
	if (!image_raw) {
		fprintf(stderr, "IMG_LoadPNG_RW: %s\n", IMG_GetError());
		return 1337;
	}

	image_argb8888 = SDL_ConvertSurfaceFormat(image_raw, SDL_PIXELFORMAT_ARGB8888, 0);
	if (!image_argb8888) {
		std::cerr << "You're fucked: " << SDL_GetError() << std::endl;
		return 1338;
	}

	SDL_Rect r;
	r.w = image_argb8888->w;
	r.h = image_argb8888->h;
	r.x = 0;
	r.y = 0;

	// create a window to display our procedural texture
	SDL_Window *win = SDL_CreateWindow("SDL_CreateTexture", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, image_argb8888->w, image_argb8888->h, SDL_WINDOW_RESIZABLE);
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

	SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, r.w, r.h);
	if (tex == nullptr){
		SDL_DestroyRenderer(ren);
		SDL_DestroyWindow(win);
		std::cout << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}
	
	CLPlatform* platforms;
	cl_uint numPlats;
	cl_int errNum = 0;
	errNum = GetCLPlatforms(&platforms, &numPlats);
	if (errNum != CL_SUCCESS) {
		PrintCLError(errNum);
		return 2;
	}
	
	for (int i = 0; i < numPlats; i++) {
		std::cout << "Vendor: " << platforms[i].vendor << "; Name: " << platforms[i].name << std::endl;
		for (int j = 0; j < platforms[i].numDevices; j++) {
			std::cout << " type " << platforms[i].devices[j].type << ":" << std::endl;
			if (platforms[i].devices[j].type & CL_DEVICE_TYPE_CPU)
				std::cout << "CPU ";
			if (platforms[i].devices[j].type & CL_DEVICE_TYPE_GPU)
				std::cout << "GPU ";
			if (platforms[i].devices[j].type & CL_DEVICE_TYPE_ACCELERATOR)
				std::cout << "accelerator ";
			if (platforms[i].devices[j].type & CL_DEVICE_TYPE_DEFAULT)
				std::cout << "default ";

			std::cout << " name=<" << platforms[i].devices[j].name << ">" << std::endl;
		}
	}

	cl_context ctx = CreateContext(&platforms[0]);
	cl_command_queue commandQueue = clCreateCommandQueue(ctx, platforms[0].devices[0].id, 0, NULL);
	cl_program program = 0;
	cl_kernel kernel = 0;
	cl_mem memObjects[3] = { 0 };

	// Allocate like 3MB of memory to fit our random values
	// Lol
	// There's no rand() in OpenCL C, but we could use one of these techniques from StackOverlow if this becomes an issue
	// http://stackoverflow.com/questions/9912143/how-to-get-a-random-number-in-opencl
	cl_uint2 resolution = { r.w, r.h };

	program = CreateProgram(ctx, platforms[0].devices[0].id, KERNAL_PATH); //"D:\\Trevor\\Repos\\COMP8551-Assign3\\Assign3\\src\\Filter.cl"
	kernel = clCreateKernel(program, "gaussian_kernel", NULL);

	if (errNum != CL_SUCCESS)
	{
		PrintCLError(errNum);
		Cleanup(ctx, commandQueue, program, kernel, memObjects);
		return 1;
	}

    // Set the work group sizes for our OpenCL kernel
	size_t globalWorkSize[2] = { r.w, r.h };
	size_t localWorkSize[2] = { 32, 32 };

	// Generate a Gaussian Filter
	float filter[FILTER_SIZE * FILTER_SIZE];
	GaussianFilter(filter);

    // Allocate a buffer to hold the serial results
    uint32_t *result_buf = (uint32_t*)malloc(sizeof(uint32_t) * image_argb8888->w * image_argb8888->h);

    // Get the pixels array for the texture; it will stay the same throughout the life of the program, so we can cache it
	void *pixels;
	int pitch;

	if (SDL_LockTexture(tex, NULL, &pixels, &pitch) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't lock texture: %s\n", SDL_GetError());
		return 1;
	}
	std::memcpy(pixels, image_argb8888->pixels, image_argb8888->w * image_argb8888->h * 4);
	CreateMemObjects(ctx, filter, pixels, memObjects);
	SDL_UnlockTexture(tex);

    bool done = false;
	while (!done)
	{
		SDL_Event event;
		SDL_PollEvent(&event);
        switch(event.type) {
            case SDL_QUIT:
                done = true;
                break;

            case SDL_KEYDOWN:
                switch(event.key.keysym.sym) {
                    case SDLK_g:
                        if (SDL_LockTexture(tex, NULL, &pixels, &pitch) < 0) {
				            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't lock texture: %s\n", SDL_GetError());
				            return 1;
			            }
	
    		            errNum |= clSetKernelArg(kernel, 0, sizeof(cl_uint2), &resolution);
			            errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[0]);
			            errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &memObjects[1]);
			            errNum |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &memObjects[2]);

			            errNum |= clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL,
				            globalWorkSize, localWorkSize,
				            0, NULL, NULL);
			            if (errNum != CL_SUCCESS)
			            {
				            std::cerr << "Error: " << CLErrorToString(errNum) << std::endl;
				            Cleanup(ctx, commandQueue, program, kernel, memObjects);
				            return 1;
			            }

			            // Read the output buffer back to the Host
			            errNum = clEnqueueReadBuffer(commandQueue, memObjects[2], CL_TRUE,
				            0, ARRAY_SIZE * sizeof(cl_uchar4), pixels,
				            0, NULL, NULL);
			            if (errNum != CL_SUCCESS)
			            {
				            std::cerr << "Error: " << CLErrorToString(errNum) << std::endl;
				            Cleanup(ctx, commandQueue, program, kernel, memObjects);
				            return 1;
			            }

			            SDL_UnlockTexture(tex);
                        break;

                    case SDLK_s:
                        if (SDL_LockTexture(tex, NULL, &pixels, &pitch) < 0) {
				            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't lock texture: %s\n", SDL_GetError());
				            return 1;
			            }

			            SerialGaussianBlur(resolution.s0, resolution.s1, filter, (cl_uchar4*)pixels, (cl_uchar4*)result_buf);
			            SDL_UnlockTexture(tex);
                }
        }

		SDL_RenderClear(ren);
		SDL_RenderCopy(ren, tex, NULL, NULL);
		SDL_RenderPresent(ren);
	}

	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();

	return 0;
}
