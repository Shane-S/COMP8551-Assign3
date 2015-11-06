#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#define FILTER_SIZE 3
#define SIGMA 15
#define GAUSSIAN true

#ifdef __APPLE__

#include <OpenCL/opencl.h>

#else

#include <CL/opencl.h>

#endif
#define ARRAY_SIZE (1024 * 640)

typedef enum CLProfile {
	EMBEDDED,
	FULL,
	UNKNOWN
} CLProfile;

typedef struct CLDevice {
	// A null-terminated string representing the device's name
	char* name;

	cl_device_id id;

	cl_device_type type;
	CLProfile profile;

	cl_bool available;
	cl_bool compilerAvailable;
} CLDevice;

typedef struct CLPlatform {
	// The list of devices on this platform
	cl_int numDevices;
	CLDevice* devices;

	char* name;   // A null-terminated string representing the platform's name (e.g. AMD Parallel Processing)
	char* vendor; // The platform vendor's info

	// The unique ID for the platform
	cl_platform_id id;
	
	// The platform profile (embedded or full) and the OpenCL version information
	CLProfile profile;
	struct {
		int major;
		int minor;
	} version;
} CLPlatform;

/// <summary>
/// Gets a list of OpenCL platforms available on this machine.
/// </summary>
/// <param name="platforms">Holds the list of platforms on successful completion.</param>
/// <param name="numPlatforms">Holds the number of platforms in the list.</param>
/// <returns>CL_SUCCESS on successful copmletion, or an OpenCL error code on failure.</returns>
cl_int GetCLPlatforms(CLPlatform** platforms, cl_uint* numPlatforms);

/// <summary>
/// Cleans up the CLPlatform and any CLDevices it may have.
/// </summary>
void FreeCLPlatform(CLPlatform* platform);

// Function to check return value of OpenCL calls and
// output custom error message to cerr
bool CheckOpenCLError(cl_int errNum, const char *errMsg);

/// <summary>
/// Creates an OpenCL context using the first available platform, if any.
/// </summary>
/// <returns>An OpenCL context or NULL on failure.</returns>
cl_context CreateContext(CLPlatform* platform);

/// <summary>
/// Compiles the specified OpenCL kernel for the given device.
/// </summary>
/// <param name="context">The context specifying the platform and other info.</param>
/// <param name="device">The device to target.</param>
/// <param name="fileName">The path to a file containing an OpenCL kernel to compile.</param>
/// <returns></returns>
cl_program CreateProgram(cl_context context, cl_device_id device, const char* fileName);

//  Create memory objects used as the arguments to the kernel
//  The kernel takes three arguments: result (output), a (input), and b (input)
bool CreateMemObjects(cl_context context, cl_float* filter, void* pixels, cl_mem memObjects[3]);

/// <summary>
/// Cleans up the OpenCL objects created throughout the program.
/// </summary>
/// <param name="context">The OpenCL context.</param>
/// <param name="commandQueue">The command queue for a given device.</param>
/// <param name="program">The OpenCL program compiled for a particular device.</param>
/// <param name="kernel">The OpenCL kernel.</param>
/// <param name="memObjects">The array holding OpenCL memory objects with kernel arguments.</param>
void Cleanup(cl_context context, cl_command_queue commandQueue, cl_program program, cl_kernel kernel, cl_mem memObjects[3]);

/// <summary>
/// Maps OpenCL error codes to strings.
/// </summary>
/// <param name="error">The OpenCL error code.</param>
/// <returns>A string representing the given error code.</returns>
char* CLErrorToString(cl_int error);

/// <summary>
/// Prints an error string representing the given error number.
/// </summary>
/// <param name="errNum">An OpenCL error code.</param>
void PrintCLError(cl_int errNum);

/// <summary>
/// Generates a Gaussian filter.
/// </summary>
/// <param name="filter">A 1D array to store the filter.</param>
void GaussianFilter(float* filter);

/// <summary>
/// Filters the image using Gaussian, serially
/// </summary>
/// <param name="width">The width of the image.</param>
/// <param name="height">The height of the image.</param>
/// <param name="filter">A 1D array to store the filter.</param>
/// <param name="input_image">A 1D array to store the input image.</param>
/// <param name="result">A 1D array to store the resulting image.</param>
void SerialGaussianBlur(int width, int height, const float* filter, cl_uchar4 *input_image, cl_uchar4 *result);

/// <summary>
/// Returns the position in a 1d array, given two indices and the width
/// </summary>
/// <param name="x">The x index in the 2d array</param>
/// <param name="y">The y index in the 2d array.</param>
/// <param name="width">The width of the 2d array.</param>
int pos(int x, int y, int width);