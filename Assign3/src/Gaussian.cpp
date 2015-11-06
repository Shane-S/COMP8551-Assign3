#include "Gaussian.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstring>

// Returns an index into a 1d array from a pair of 2d array indicies
int pos(int x, int y, int width) 
{
	return x + (y * width);
}

// Generates a Gaussian filter
void GaussianFilter(float* filter)
{
	// 2D representation of the filter
	double kernal[FILTER_SIZE][FILTER_SIZE];
	int dimension = FILTER_SIZE / 2;
	// sum is for normalization
	double sum = 0.0;

	// generate a kernal
	for (int x = -dimension; x <= dimension; x++)
	{
		for (int y = -dimension; y <= dimension; y++)
		{
			// G(x,y) = (1 / 2 * PI * sigma^2) * e^-((x^2 + y^2) / 2 * sigma^2)
			kernal[x + dimension][y + dimension] = (1 / (2 * M_PI * (SIGMA * SIGMA))) * exp(-((x * x + y * y) / (2 * SIGMA * SIGMA)));
			// Store the sum for normalization
			sum += kernal[x + dimension][y + dimension];
		}
	}

	// normalize the Kernel and store the kernal
	for (int i = 0; i < FILTER_SIZE; ++i)
	{
		for (int j = 0; j < FILTER_SIZE; ++j)
		{
			// Normalize
			kernal[i][j] /= sum;
			// Store in the 1D array
			filter[i * FILTER_SIZE + j] = kernal[i][j];

			// Just in case we want to test with out a filter
			if (!GAUSSIAN)
			{
				filter[i * FILTER_SIZE + j] = 0;
			}
		}
	}

	// If we dont want a filter, make the center of the filter 1 so that there is no change to the image
	if (!GAUSSIAN)
	{
		filter[dimension * FILTER_SIZE + dimension] = 1;
	}
}

// Run the gaussian filter serially
void SerialGaussianBlur(int width, int height, const float* filter, cl_uchar4 *input_image, cl_uchar4 *result)
{
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			int offset = pos(x, y, width);

			int dim = FILTER_SIZE / 2;
			cl_float3 sum = { 0.0f, 0.0f, 0.0f };

			for (int col = 0; col < FILTER_SIZE; col++)
			{
				for (int row = 0; row < FILTER_SIZE; row++) 
				{
					int offx = col - dim;
					int offy = row - dim;
					if (!(x + offx < 0 || x + offx > width - 1 ||
						y + offy < 0 || y + offy > height - 1)) 
					{
						int off_img = pos(x + offx, y + offy, width);
						int off_filter = pos(col, row, FILTER_SIZE);
						sum.x += (input_image[off_img].x) * filter[off_filter];
						sum.y += (input_image[off_img].y) * filter[off_filter];
						sum.z += (input_image[off_img].z) * filter[off_filter];
					}
				}
			}

			result[offset].s0 = (cl_uchar)(sum.s0);
			result[offset].s1 = (cl_uchar)(sum.s1);
			result[offset].s2 = (cl_uchar)(sum.s2);
			result[offset].s3 = input_image[offset].s3;
		}
	}

	// Copy the result back into the input
	std::memcpy(input_image, result, width * height * sizeof(cl_uchar4));
}
