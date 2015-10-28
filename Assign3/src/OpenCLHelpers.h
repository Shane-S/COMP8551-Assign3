#define CL_USE_DEPRECATED_OPENCL_2_0_APIS

#include <CL/opencl.h>

// Function to check return value of OpenCL calls and
// output custom error message to cerr
bool CheckOpenCLError(cl_int errNum, const char *errMsg);

//  Create an OpenCL context on the first available platform using
//  either a GPU or CPU depending on what is available.
cl_context CreateContext();

//  Create a command queue on the first device available on the context
cl_command_queue CreateCommandQueue(cl_context context, cl_device_id *device);

//  Create an OpenCL program from the kernel source file
cl_program CreateProgram(cl_context context, cl_device_id device, const char* fileName);

//  Create memory objects used as the arguments to the kernel
//  The kernel takes three arguments: result (output), a (input), and b (input)
bool CreateMemObjects(cl_context context, cl_mem memObjects[1], cl_uint *randoms);

//  Cleanup any created OpenCL resources
void Cleanup(cl_context context, cl_command_queue commandQueue,
	cl_program program, cl_kernel kernel, cl_mem memObjects[1]);