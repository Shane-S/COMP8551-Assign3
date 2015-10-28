__kernel void random_kernel(__global const uint* rand, __global uchar4 *result)
{
	// pixel format is ARGB, but since we're little endian, 0 = b, 1 = g, etc.
    int gid = get_global_id(0);
	
	//printf("%d, %d, %d\n", rand[gid * 3], rand[(gid * 3) + 1], rand[(gid * 3) + 2]);

	result[gid].s0 = (uchar)(rand[gid * 3] % 255);
	result[gid].s1 = (uchar)(rand[(gid * 3) + 1] % 255);
	result[gid].s2 = (uchar)(rand[(gid * 3) + 2] % 255);
	result[gid].s3 = 255;// gid % 255;//;
}
