#pragma once

#include <vector>

class ConvolutionalFilter
{
	std::vector<float> m_history;
	std::vector<float> m_filterTaps;

public:
	ConvolutionalFilter(const float filterTaps[], const int tapsCount);
	float Update(const float x);
};