#ifndef GAUSSIAN_H
#define GAUSSIAN_H

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif

#define FILTER_SIZE 15
#define SIGMA 15
#define GAUSSIAN true

///<summary/// <summary>
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

#endif // GAUSSIAN_H