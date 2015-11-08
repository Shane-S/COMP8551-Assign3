#include "CTimer.h"

// Helper class for timing calculations
void CTimer::Start() {
	start = std::chrono::high_resolution_clock::now();
}

void CTimer::End() { 
	end = std::chrono::high_resolution_clock::now();
}

bool CTimer::Diff(double &seconds) {
	std::chrono::duration<double> elapsed = end - start;
	seconds = elapsed.count();
	return false;
}
