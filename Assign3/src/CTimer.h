#ifndef CTIMER_H
#define CTIMER_H

#include <chrono>

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