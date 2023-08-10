#pragma once

#include "graphics.hpp"
#include "signalprovider.hpp"

class Application
{
	HWND m_mainWindow;
	Graphics m_graphics;
	ComPtr<ID2D1SolidColorBrush> m_brush;

	Transform<float2> m_signalTransform;
	int2 m_resolution;
	int2 m_prevCursor;

	SignalProvider m_signalProvider;

private:
	void AskRedraw() const;
	void MouseMove(int x, int y, WPARAM flags);
	void MouseWheel(int delta, WPARAM flags);
	void Resize(int width, int height);
	void Paint() const;

public:
	Application();
	void Init(const wchar_t* title, int width, int height);
	void Run();
	LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam);
};