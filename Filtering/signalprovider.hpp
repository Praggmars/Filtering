#include <vector>

class SignalProvider
{
	std::vector<float> m_inputSignal;
	std::vector<float> m_outputSignal;

public:
	void InitSignals(std::size_t length);

	const inline std::vector<float> InputSignal() const { return m_inputSignal; }
	const inline std::vector<float> OutputSignal() const { return m_outputSignal; }
};