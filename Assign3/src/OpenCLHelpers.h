#define CL_USE_DEPRECATED_OPENCL_2_0_APIS

#include <CL/opencl.h>
#define ARRAY_SIZE (1024 * 768)

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
bool CreateMemObjects(cl_context context, cl_mem memObjects[2], cl_uint *randoms);

/// <summary>
/// Cleans up the OpenCL objects created throughout the program.
/// </summary>
/// <param name="context">The OpenCL context.</param>
/// <param name="commandQueue">The command queue for a given device.</param>
/// <param name="program">The OpenCL program compiled for a particular device.</param>
/// <param name="kernel">The OpenCL kernel.</param>
/// <param name="memObjects">The array holding OpenCL memory objects with kernel arguments.</param>
void Cleanup(cl_context context, cl_command_queue commandQueue, cl_program program, cl_kernel kernel, cl_mem memObjects[2]);

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
