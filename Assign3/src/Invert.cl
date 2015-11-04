__kernel void invert_kernel(__global uchar4 *result)
{
	// pixel format is ARGB, but since we're little endian, 0 = b, 1 = g, etc.
	int x = get_global_id(0);
	int y = get_global_id(1);
	int offset = x + (1024 * y);

	float2 coords = (float2)(x, y);
	float2 p = coords.xy / resolution.xy;
	p *= 2.f;
	p -= 1.f;

	float mov0 = p.x + p.y + cos(sin(iGlobalTime) * 2.0) * 100.f + sin(p.x / 100.f) * 1000.f;
	float mov1 = p.y / 0.9 + iGlobalTime;
	float mov2 = p.x / 0.2;
	float c1 = fabs(sin(mov1 + iGlobalTime) / 2.f + mov2 / 2.f - mov1 - mov2 + iGlobalTime);
	float c2 = fabs(sin(c1 + sin(mov0 / 1000.f + iGlobalTime) + sin(p.y / 40.f + iGlobalTime) + sin((p.x + p.y) / 100.f) * 3.f));
	float c3 = fabs(sin(c2 + cos(mov1 + mov2 + c2) + cos(mov2) + sin(p.x / 1000.f)));

	result[offset].s0 = (uchar)(c1 * 255);
	result[offset].s1 = (uchar)(c2 * 255);
	result[offset].s2 = (uchar)(c3 * 255);
	result[offset].s3 = 255;
}