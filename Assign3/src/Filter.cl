#define FILTER_SIZE 9

/**
 * Gives the offset into a one-dimensional array given its x and y coordinates and the width.
 */
uint pos(uint x, uint y, uint width) {
	return x + (y * width);
}

__kernel void gaussian_kernel(const uint2 dimensions, __global const float* filter, __global uchar4 *input_image, __global uchar4 *result)
{
	uint x = get_global_id(0);
	uint y = get_global_id(1);
	uint offset = pos(x, y, dimensions.x);

	uint row;
	uint col;
	uint dim = FILTER_SIZE / 2;
	float3 sum = (float3)(0, 0, 0);

	for (col = 0; col < FILTER_SIZE; col++) {
		for (row = 0; row < FILTER_SIZE; row++) {
			uint offx = col - dim;
			uint offy = row - dim;
			if (!(x + offx < 0 || x + offx > dimensions.x - 1 ||
				y + offy < 0 || y + offy > dimensions.y - 1)) {
				uint off_img = pos(x + offx, y + offy, dimensions.x);
				uint off_filter = pos(col, row, FILTER_SIZE);
				sum.xyz += convert_float4(input_image[off_img]).xyz * filter[off_filter];
			}
		}
	}

	result[offset].s0 = (uchar)(sum.s0 * 255);
	result[offset].s1 = (uchar)(sum.s1 * 255);
	result[offset].s2 = (uchar)(sum.s2 * 255);
	result[offset].s3 = input_image[offset].s3;
}