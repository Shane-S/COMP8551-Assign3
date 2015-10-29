#include "OpenCLHelpers.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <cstring>

char* CLErrorToString(cl_int error) {
	switch (error) {
	case CL_SUCCESS:                            return "Success!";
	case CL_DEVICE_NOT_FOUND:                   return "Device not found.";
	case CL_DEVICE_NOT_AVAILABLE:               return "Device not available";
	case CL_COMPILER_NOT_AVAILABLE:             return "Compiler not available";
	case CL_MEM_OBJECT_ALLOCATION_FAILURE:      return "Memory object allocation failure";
	case CL_OUT_OF_RESOURCES:                   return "Out of resources";
	case CL_OUT_OF_HOST_MEMORY:                 return "Out of host memory";
	case CL_PROFILING_INFO_NOT_AVAILABLE:       return "Profiling information not available";
	case CL_MEM_COPY_OVERLAP:                   return "Memory copy overlap";
	case CL_IMAGE_FORMAT_MISMATCH:              return "Image format mismatch";
	case CL_IMAGE_FORMAT_NOT_SUPPORTED:         return "Image format not supported";
	case CL_BUILD_PROGRAM_FAILURE:              return "Program build failure";
	case CL_MAP_FAILURE:                        return "Map failure";
	case CL_INVALID_VALUE:                      return "Invalid value";
	case CL_INVALID_DEVICE_TYPE:                return "Invalid device type";
	case CL_INVALID_PLATFORM:                   return "Invalid platform";
	case CL_INVALID_DEVICE:                     return "Invalid device";
	case CL_INVALID_CONTEXT:                    return "Invalid context";
	case CL_INVALID_QUEUE_PROPERTIES:           return "Invalid queue properties";
	case CL_INVALID_COMMAND_QUEUE:              return "Invalid command queue";
	case CL_INVALID_HOST_PTR:                   return "Invalid host pointer";
	case CL_INVALID_MEM_OBJECT:                 return "Invalid memory object";
	case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:    return "Invalid image format descriptor";
	case CL_INVALID_IMAGE_SIZE:                 return "Invalid image size";
	case CL_INVALID_SAMPLER:                    return "Invalid sampler";
	case CL_INVALID_BINARY:                     return "Invalid binary";
	case CL_INVALID_BUILD_OPTIONS:              return "Invalid build options";
	case CL_INVALID_PROGRAM:                    return "Invalid program";
	case CL_INVALID_PROGRAM_EXECUTABLE:         return "Invalid program executable";
	case CL_INVALID_KERNEL_NAME:                return "Invalid kernel name";
	case CL_INVALID_KERNEL_DEFINITION:          return "Invalid kernel definition";
	case CL_INVALID_KERNEL:                     return "Invalid kernel";
	case CL_INVALID_ARG_INDEX:                  return "Invalid argument index";
	case CL_INVALID_ARG_VALUE:                  return "Invalid argument value";
	case CL_INVALID_ARG_SIZE:                   return "Invalid argument size";
	case CL_INVALID_KERNEL_ARGS:                return "Invalid kernel arguments";
	case CL_INVALID_WORK_DIMENSION:             return "Invalid work dimension";
	case CL_INVALID_WORK_GROUP_SIZE:            return "Invalid work group size";
	case CL_INVALID_WORK_ITEM_SIZE:             return "Invalid work item size";
	case CL_INVALID_GLOBAL_OFFSET:              return "Invalid global offset";
	case CL_INVALID_EVENT_WAIT_LIST:            return "Invalid event wait list";
	case CL_INVALID_EVENT:                      return "Invalid event";
	case CL_INVALID_OPERATION:                  return "Invalid operation";
	case CL_INVALID_GL_OBJECT:                  return "Invalid OpenGL object";
	case CL_INVALID_BUFFER_SIZE:                return "Invalid buffer size";
	case CL_INVALID_MIP_LEVEL:                  return "Invalid mip-map level";
	default: return "Unknown";
	}
}


void PrintCLError(cl_int errNum) {
	std::fprintf(stderr, "Error: %s\n", CLErrorToString(errNum));
}

// Function to check return value of OpenCL calls and
// output custom error message to cerr
bool CheckOpenCLError(cl_int errNum, const char *errMsg) {
	if (errNum != CL_SUCCESS)
	{
		std::cerr << errMsg << std::endl;
		return false;
	}
	return true;
}

void FreeCLDevice(CLDevice* device) {
	free(device->name);
}

void FreeCLPlatform(CLPlatform* platform) {
	for (int i = 0; i < platform->numDevices; i++) {
		FreeCLDevice(&platform->devices[i]);
	}
	free(platform->devices);
	free(platform->name);
	free(platform->vendor);
}

cl_int GetCLDevices(CLPlatform* platform) {
	// Get number of devices
	cl_uint numDevices;
	size_t retSize;
	cl_int errNum = clGetDeviceIDs(platform->id, CL_DEVICE_TYPE_ALL, 0, nullptr, &numDevices);
	if (errNum != CL_SUCCESS) return errNum;

	cl_device_id *ids = (cl_device_id*)malloc(sizeof(cl_device_id)* numDevices);
	errNum = clGetDeviceIDs(platform->id, CL_DEVICE_TYPE_ALL, numDevices, ids, nullptr);
	if (errNum != CL_SUCCESS) {
		free(ids);
		return errNum;
	}

	// Get some information about the devices in this platform
	char param[1024];
	size_t retsize;
	platform->devices = (CLDevice*)malloc(sizeof(CLDevice)* numDevices);
	for (int i = 0; i < numDevices; i++) {
		platform->devices[i].id = ids[i];

		errNum = clGetDeviceInfo(ids[i], CL_DEVICE_PROFILE, sizeof(param), (void *)param, &retsize);
		if (errNum != CL_SUCCESS) {
			for (int j = 0; j < i; j++) FreeCLDevice(&platform->devices[j]);
			free(ids);
			return errNum;
		}
		param[retsize] = 0;
		if (strcmp("EMBEDDED_PROFILE", param) == 0)
			platform->devices[i].profile = EMBEDDED;
		else if (strcmp("FULL_PROFILE", param) == 0)
			platform->devices[i].profile = FULL;
		else
			platform->devices[i].profile = UNKNOWN;

		errNum = clGetDeviceInfo(ids[i], CL_DEVICE_AVAILABLE, sizeof(param), (void *)param, &retsize);
		if (errNum != CL_SUCCESS) {
			for (int j = 0; j < i; j++) FreeCLDevice(&platform->devices[j]);
			free(ids);
			return errNum;
		}
		platform->devices[i].available = *((cl_bool*)&param[0]);

		errNum = clGetDeviceInfo(ids[i], CL_DEVICE_COMPILER_AVAILABLE, sizeof(param), (void *)param, &retsize);
		if (errNum != CL_SUCCESS) {
			for (int j = 0; j < i; j++) FreeCLDevice(&platform->devices[j]);
			free(ids);
			return errNum;
		}
		platform->devices[i].compilerAvailable = *((cl_bool*)&param[0]);

		errNum = clGetDeviceInfo(ids[i], CL_DEVICE_TYPE, sizeof(param), (void *)param, &retsize);
		if (errNum != CL_SUCCESS) {
			for (int j = 0; j < i; j++) FreeCLDevice(&platform->devices[j]);
			free(ids);
			return errNum;
		}
		platform->devices[i].type = *((cl_device_type*)&param[0]);

		errNum = clGetDeviceInfo(ids[i], CL_DEVICE_NAME, sizeof(param), (void *)param, &retsize);
		if (errNum != CL_SUCCESS) {
			for (int j = 0; j < i; j++) FreeCLDevice(&platform->devices[j]);
			free(ids);
			return errNum;
		}
		platform->devices[i].name = (char*)malloc(retsize + 1);
		strncpy_s(platform->devices[i].name, retsize + 1, param, retsize);
		platform->devices[i].name[retsize] = 0;
	}
}


cl_int GetCLPlatforms(CLPlatform** platforms, cl_uint* numPlatforms) {
	cl_uint num;
	cl_int errNum = clGetPlatformIDs(0, nullptr, &num);
	if (errNum != CL_SUCCESS) return errNum;

	cl_platform_id *ids = (cl_platform_id*)malloc(sizeof(cl_platform_id)* num);
	CLPlatform *plats = (CLPlatform*)malloc(sizeof(CLPlatform*) * num);
	errNum = clGetPlatformIDs(num, ids, nullptr);
	if (errNum != CL_SUCCESS) {
		free(ids);
		return errNum;
	}
	
	// Gets a list of OpenCL platforms on this machine and a list of devices for each
	char param[1024];
	size_t retsize;
	for (int i = 0; i < num; i++) {
		
		// Get the platform profile. If the profile is EMBEDDED, then all devices on this machine will be EMBEDDED too;
		// if it's FULL, then they can be a mix of both.
		errNum = clGetPlatformInfo(ids[i], CL_PLATFORM_PROFILE, sizeof(param), (void *)param, &retsize);
		if (errNum != CL_SUCCESS) {
			for (int j = 0; j < i; j++) FreeCLPlatform(&plats[j]);
			free(ids);
			return errNum;
		}
		param[retsize] = 0;
		if (strcmp("EMBEDDED_PROFILE", param) == 0)
			plats[i].profile = EMBEDDED;
		else if (strcmp("FULL_PROFILE", param) == 0)
			plats[i].profile = FULL;
		else
			plats[i].profile = UNKNOWN;

		errNum = GetCLDevices(&plats[i]);
		if (errNum != CL_SUCCESS) {
			for (int j = 0; j < i; j++) FreeCLPlatform(&plats[j]);
			free(ids);
			return errNum;
		}

		errNum = clGetPlatformInfo(ids[i], CL_PLATFORM_VERSION, sizeof(param), (void *)param, &retsize);
		if (errNum != CL_SUCCESS) {
			for (int j = 0; j < i; j++) FreeCLPlatform(&plats[j]);
			free(ids);
			return errNum;
		}
		if (sscanf_s(param, "%d %d", &plats[i].version.major, &plats[i].version.minor) != 2) // The return value is formatted as a string containing <major>.<minor>
			fprintf(stderr, "GetOpenCLPlatforms: invalid version number encountered.");

		errNum = clGetPlatformInfo(ids[i], CL_PLATFORM_VENDOR, sizeof(param), (void *)param, &retsize);
		if (errNum != CL_SUCCESS) {
			for (int j = 0; j < i; j++) FreeCLPlatform(&plats[j]);
			free(ids);
			return errNum;
		}
		plats[i].vendor = (char*)malloc(retsize + 1);
		strncpy_s(plats[i].vendor, retsize + 1, param, retsize);
		plats[i].vendor[retsize] = 0;

		errNum = clGetPlatformInfo(ids[i], CL_PLATFORM_NAME, sizeof(param), (void *)param, &retsize);
		if (errNum != CL_SUCCESS) {
			for (int j = 0; j < i; j++) FreeCLPlatform(&plats[j]);
			free(plats[i].vendor);
			free(ids);
			return errNum;
		}
		plats[i].name = (char*)malloc(retsize + 1);
		strncpy_s(plats[i].name, retsize + 1, param, retsize);
		plats[i].name[retsize] = 0;
	}
}

//  Create an OpenCL context on the first available platform using
//  either a GPU or CPU depending on what is available.
cl_context CreateContext()
{
	// Get the number of available platforms
	// Note: should probably use a smart pointer here so we don't have to delete it whenever it goes out of scope
	cl_platform_id *platformIds;
	cl_uint numPlatforms;
	cl_int errNum = clGetPlatformIDs(0, nullptr, &numPlatforms);
	if (errNum != CL_SUCCESS) {
		PrintCLError(errNum);
		return NULL;
	}

	if (numPlatforms <= 0) {
		std::cerr << "Failed to find any OpenCL platforms." << std::endl;
		return NULL;
	}
	std::cout << std::endl << numPlatforms << " platforms in total" << std::endl;

	// Now get list of platforms
	platformIds = new cl_platform_id[numPlatforms];
	errNum = clGetPlatformIDs(numPlatforms, platformIds, nullptr);
	if (errNum != CL_SUCCESS) {
		PrintCLError(errNum);
		delete[] platformIds;
		return NULL;
	}

	// Get information about the platform
	char pname[1024];
	size_t retsize;
	for (int i = 0; i < numPlatforms; i++) {
		errNum = clGetPlatformInfo(platformIds[i], CL_PLATFORM_NAME, sizeof(pname), (void *)pname, &retsize);
		if (errNum != CL_SUCCESS) {
			PrintCLError(errNum);
			delete[] platformIds;
			return NULL;
		}
		std::cout << std::endl << "Platform " << i << " name: " << pname << std::endl;
	}

	// Create a context from the first platform (for now)
	cl_context_properties contextProperties[] =
	{
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)platformIds[0],
		0
	};
	cl_context context = NULL;
	context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_ALL, NULL, NULL, &errNum);

	if (errNum != CL_SUCCESS){
		std::cout << CLErrorToString(errNum) << std::endl;
		delete[] platformIds;
		return NULL;
	}

	delete[] platformIds;
	return context;
}

cl_command_queue CreateCommandQueue(cl_context context, cl_device_id *device) {
	
	// Get number of devices
	cl_int numDevices;
	size_t retSize;
	cl_int errNum = clGetContextInfo(context, CL_CONTEXT_NUM_DEVICES, sizeof(numDevices), (void *)&numDevices, &retSize);
	
	if (errNum != CL_SUCCESS) {
		PrintCLError(errNum);
		return NULL;
	}
	std::cout << std::endl << "There are " << numDevices << " devices." << std::endl;

	// Get list of devices
	cl_device_id *deviceList;
	deviceList = (cl_device_id *)malloc(numDevices * sizeof(cl_device_id));
	errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, numDevices * sizeof(cl_device_id), (void *)deviceList, &retSize);
	if (!CheckOpenCLError(errNum, "Could not get device list!"))
	{
		PrintCLError(errNum);
		std::cerr << " size = " << numDevices * sizeof(cl_device_id) << ";" << retSize << std::endl;
		return NULL;
	}


	// Get device information for each device
	cl_device_type devType;
	std::cout << std::endl << "Device list:" << std::endl;
	for (int i = 0; i < numDevices; i++)
	{

		std::cout << "   " << deviceList[i] << ": ";

		// device type
		errNum = clGetDeviceInfo(deviceList[i], CL_DEVICE_TYPE, sizeof(cl_device_type), (void *)&devType, &retSize);
		if (!CheckOpenCLError(errNum, "ERROR getting device info!"))
		{
			free(deviceList);
			return NULL;
		}
		std::cout << " type " << devType << ":" << std::endl;
		if (devType & CL_DEVICE_TYPE_CPU)
			std::cout << "CPU ";
		if (devType & CL_DEVICE_TYPE_GPU)
			std::cout << "GPU ";
		if (devType & CL_DEVICE_TYPE_ACCELERATOR)
			std::cout << "accelerator ";
		if (devType & CL_DEVICE_TYPE_DEFAULT)
			std::cout << "default ";

		// device name
		char devName[1024];
		errNum = clGetDeviceInfo(deviceList[i], CL_DEVICE_NAME, 1024, (void *)devName, &retSize);
		if (!CheckOpenCLError(errNum, "ERROR getting device name!"))
		{
			free(deviceList);
			return NULL;
		}
		std::cout << " name=<" << devName << ">" << std::endl;

	}
	std::cout << std::endl;


	// In this example, we just choose the first available device.  In a
	// real program, you would likely use all available devices or choose
	// the highest performance device based on OpenCL device queries
	cl_command_queue commandQueue = clCreateCommandQueue(context, deviceList[0], 0, NULL);
	if (commandQueue == NULL)
	{
		free(deviceList);
		std::cerr << "Failed to create commandQueue for device 0";
		return NULL;
	}

	*device = deviceList[0];

	free(deviceList);

	return commandQueue;
}



//  Create an OpenCL program from the kernel source file
cl_program CreateProgram(cl_context context, cl_device_id device, const char* fileName) {
	cl_int errNum;
	cl_program program;

	std::ifstream kernelFile(fileName, std::ios::in);
	if (!kernelFile.is_open())
	{
		std::cerr << "Failed to open file for reading: " << fileName << std::endl;
		return NULL;
	}

	std::ostringstream oss;
	oss << kernelFile.rdbuf();

	std::string srcStdStr = oss.str();
	const char *srcStr = srcStdStr.c_str();
	program = clCreateProgramWithSource(context, 1,
		(const char**)&srcStr,
		NULL, NULL);
	if (program == NULL)
	{
		std::cerr << "Failed to create CL program from source." << std::endl;
		return NULL;
	}

	errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (errNum != CL_SUCCESS)
	{
		// Determine the reason for the error
		char buildLog[16384];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
			sizeof(buildLog), buildLog, NULL);

		std::cerr << "Error in kernel: " << std::endl;
		std::cerr << buildLog;
		clReleaseProgram(program);
		return NULL;
	}

	return program;
}



//  Create memory objects used as the arguments to the kernel
//  The kernel takes three arguments: result (output), a (input), and b (input)
bool CreateMemObjects(cl_context context, cl_mem memObjects[2], cl_uint *randoms) {
	
	memObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * ARRAY_SIZE * 3, randoms, NULL);
	memObjects[1] = clCreateBuffer(context, CL_MEM_WRITE_ONLY, ARRAY_SIZE * sizeof(cl_uchar4), NULL, NULL);

	if (memObjects[0] == NULL || memObjects[1] == NULL)
	{
		std::cerr << "Error creating memory objects." << std::endl;
		return false;
	}

	return true;
}

//  Cleanup any created OpenCL resources
void Cleanup(cl_context context, cl_command_queue commandQueue,
	cl_program program, cl_kernel kernel, cl_mem memObjects[2]) {
	if (memObjects[0] != 0)
		clReleaseMemObject(memObjects[0]);

	if (memObjects[1] != 0)
		clReleaseMemObject(memObjects[1]);

	if (commandQueue != 0)
		clReleaseCommandQueue(commandQueue);

	if (kernel != 0)
		clReleaseKernel(kernel);

	if (program != 0)
		clReleaseProgram(program);

	if (context != 0)
		clReleaseContext(context);

}