#pragma once

#include <vector>

class IIRFilter
{
	std::vector<float> m_poles;
	std::vector<float> m_zeros;
	std::vector<float> m_yHistory;
	std::vector<float> m_xHistory;

public:
	IIRFilter(const float poles[], const int poleCount, const float zeros[], const int zeroCount);
	float Update(const float x);
};

class ConvolutionalFilter
{
	std::vector<float> m_history;
	std::vector<float> m_filterTaps;

public:
	ConvolutionalFilter(const float filterTaps[], const int tapsCount);
	float Update(const float x);
};