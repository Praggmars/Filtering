#include "filter.hpp"

ConvolutionalFilter::ConvolutionalFilter(const float filterTaps[], const int tapsCount) :
	m_history(tapsCount, 0.0f),
	m_filterTaps(filterTaps, filterTaps + tapsCount) {}

float ConvolutionalFilter::Update(const float x)
{
	float y = 0.0f;

	for (std::size_t i = 1; i < m_history.size(); ++i)
		m_history[i - 1] = m_history[i];
	m_history[m_history.size() - 1] = x;

	for (std::size_t i = 0; i < m_filterTaps.size(); ++i)
		y += m_filterTaps[i] * m_history[i];

	return y;
}
