#ifndef CTIMER_H
#define CTIMER_H

#ifdef WIN32

#include <windows.h>
#include <cstdint>
#include <chrono>

#endif

class CTimer
{
public:
	CTimer() {}
	~CTimer() {}

	void Start();
	void End();
	bool Diff(double &seconds);

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
};

#endif