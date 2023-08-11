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
	unsigned m_mouseFlags;
	void (Application::* m_MouseMoveHandler)(int2 cursor, int2 delta, WPARAM flags);

	SignalProvider m_signalProvider;

private:
	void PlotMoverMouseMove(int2 cursor, int2 delta, WPARAM flags);

	void AskRedraw() const;
	void MouseMove(int x, int y, WPARAM flags);
	void MouseWheel(int delta, WPARAM flags);
	void LButtonDown(int x, int y, WPARAM flags);
	void MButtonDown(int x, int y, WPARAM flags);
	void RButtonDown(int x, int y, WPARAM flags);
	void LButtonUp(int x, int y, WPARAM flags);
	void MButtonUp(int x, int y, WPARAM flags);
	void RButtonUp(int x, int y, WPARAM flags);
	void Resize(int width, int height);
	void Paint() const;

public:
	Application();
	void Init(const wchar_t* title, int width, int height);
	void Run();
	LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam);
};