#include "application.hpp"
#include <stdexcept>

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d2d1.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")

int Main()
{
	Application app;
	try
	{
		app.Init(L"Filtering", 1000, 700);
		app.Run();
	}
	catch (const std::exception& ex)
	{
		MessageBoxA(nullptr, ex.what(), "Error", MB_OK);
	}
	return 0;
}

int main(int argc, char* argv[])
{
	return Main();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int nShowCmd)
{
	return Main();
}

void ThrowIfFailed(HRESULT result, const char* message)
{
	if (FAILED(result))
		throw std::runtime_error(message);
}