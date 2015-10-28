__kernel void random_kernel(__global const ulong* rand, __global uchar4 *result)
{
	// pixel format is ARGB; vector notation is wxyz, so A = w, r = x, etc.
    int gid = get_global_id(0);
	result[gid].w = 255;
	result[gid].x = (uchar)(rand[gid * 3] % 255);
	result[gid].y = (uchar)(rand[{gid * 3} + 1] % 255);
	result[gid].z = (uchar)(rand[(gid * 3) + 2] % 255);
}
