#include "application.hpp"
#include <cmath>

void Application::AskRedraw() const
{
	InvalidateRect(m_mainWindow, nullptr, false);
}

void Application::MouseMove(int x, int y, WPARAM flags)
{
	const int2 cursor{ x, y };
	const int2 delta = cursor - m_prevCursor;

	if ((MK_LBUTTON | MK_RBUTTON)& flags)
	{
		m_signalTransform.offset.x -= static_cast<float>(delta.x) / static_cast<float>(m_resolution.x - 1) * m_signalTransform.scaler.x;
		m_signalTransform.offset.y += static_cast<float>(delta.y) / static_cast<float>(m_resolution.y - 6) * 2.0f * m_signalTransform.scaler.y;
		AskRedraw();
	}

	m_prevCursor = cursor;
}

void Application::MouseWheel(int delta, WPARAM flags)
{
	float scale = 1.25f;
	if (delta > 0)
		scale = 1.0f / scale;

	if (MK_RBUTTON & flags)
	{
		const float2 srcSect = m_prevCursor.y < (m_resolution.y / 2) ?
			float2{ 0.0f, static_cast<float>(m_resolution.y - 6) * 0.5f } :
			float2{ static_cast<float>(m_resolution.y + 4) * 0.5f, static_cast<float>(m_resolution.y - 1) };
		const float cy = Transform<float>(srcSect.x, srcSect.y, 1.0f, 0.0f).Forward(static_cast<float>(m_prevCursor.y));
		const float cs = m_signalTransform.Forward({ 0.0f, cy }).y;
		m_signalTransform.scaler.y *= scale;
		m_signalTransform.offset.y = cs - (cy * m_signalTransform.scaler.y);
	}
	else
	{
		const float cx = static_cast<float>(m_prevCursor.x) / static_cast<float>(m_resolution.x - 1);
		const float cs = m_signalTransform.Forward({ cx, 0.0f }).x;
		m_signalTransform.scaler.x *= scale;
		m_signalTransform.offset.x = cs - (cx * m_signalTransform.scaler.x);
	}

	AskRedraw();
}

void Application::Resize(int width, int height)
{
	m_resolution = { width, height };
	m_graphics.Resize(width, height);
}

void Application::Paint() const
{
	ConstBuffer signalDataBuffer;
	signalDataBuffer.signalTransform = m_signalTransform;
	signalDataBuffer.samplingRate = 55.0f;
	signalDataBuffer.padding = 0;

	const float2 samplingStepBase = signalDataBuffer.signalTransform.scaler / (m_resolution.As<float>() * signalDataBuffer.samplingRate) * 10.0f;

	m_graphics.BeginDraw();

	signalDataBuffer.rectTransform = Transform<float2>({ 0.0f, 1.0f }, { 1.0f, 0.0f }, { -1.0f, 1.0f }, { 1.0f, 2.5f / static_cast<float>(m_resolution.y) });
	signalDataBuffer.samplingStep = samplingStepBase / signalDataBuffer.rectTransform.scaler;
	m_graphics.RenderFunc(signalDataBuffer, m_signalProvider.InputSignal().data(), m_signalProvider.InputSignal().size());

	signalDataBuffer.rectTransform = Transform<float2>({ 0.0f, 1.0f }, { 1.0f, 0.0f }, { -1.0f, -2.5f / static_cast<float>(m_resolution.y) }, { 1.0f, -1.0f });
	signalDataBuffer.samplingStep = samplingStepBase / signalDataBuffer.rectTransform.scaler;
	m_graphics.RenderFunc(signalDataBuffer, m_signalProvider.OutputSignal().data(), m_signalProvider.OutputSignal().size());

	//m_graphics.Context2D()->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(100.0f, 100.0f), 80.0f, 80.0f), m_brush.Get(), 10.0f);

	m_graphics.EndDraw();
	ValidateRect(m_mainWindow, nullptr);
}

Application::Application() : m_mainWindow{} {}

void Application::Init(const wchar_t* title, int width, int height)
{
	WNDCLASSEXW wc{};
	wc.cbSize = sizeof(wc);
	wc.hInstance = GetModuleHandleW(nullptr);
	wc.lpszClassName = title;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = DefWindowProcW;
	RegisterClassExW(&wc);

	m_mainWindow = CreateWindowExW(0, title, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, nullptr, nullptr);
	m_graphics.Init(m_mainWindow);
	ThrowIfFailed(m_graphics.Context2D()->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.5f, 0.0f, 1.0f), &m_brush), "Failed to create brush");

	m_signalTransform = Transform<float2>({ 0.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 1.1f }, { 1.0f, -1.1f });
	m_signalProvider.InitSignals(16384);

	SetWindowLongPtrW(m_mainWindow, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	SetWindowLongPtrW(m_mainWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(static_cast<LRESULT(*)(HWND, UINT, WPARAM, LPARAM)>([](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)->LRESULT {
		return reinterpret_cast<Application*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))->MessageHandler(msg, wparam, lparam);
		})));
	ShowWindow(m_mainWindow, SW_SHOWDEFAULT);
	UpdateWindow(m_mainWindow);
}

void Application::Run()
{
	MSG msg{};
	while (GetMessageW(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}

LRESULT Application::MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_MOUSEMOVE:
		MouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
		return 0;
	case WM_MOUSEWHEEL:
		MouseWheel(GET_WHEEL_DELTA_WPARAM(wParam), wParam);
		return 0;
	case WM_PAINT:
		Paint();
		return 0;
	case WM_SIZE:
		Resize(LOWORD(lParam), HIWORD(lParam));
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(m_mainWindow, msg, wParam, lParam);
}