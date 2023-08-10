#include "graphics.hpp"
#include <iostream>

#define FUNCTION_TEXTURE_SIZE 16384

namespace ShaderCode
{
	const char* g_VsCode = R"(
cbuffer ConstBuffer
{
	float2 destOffset;
	float2 destSize;
	float2 signalOffset;
	float2 signalSize;
	float2 samplingStep;
	float samplingRate;
	uint padding;
};
struct PsInputType
{
	float4 position : SV_POSITION;
	float2 coord : COORD;
};
PsInputType main(float2 input : POSITION)
{
	PsInputType output;
	output.coord = input * signalSize + signalOffset;
	output.position = float4(input * destSize + destOffset, 0.0f, 1.0f);
	return output;
}
)";

	const char* g_PsCode = R"(
Texture1D funcTexture;
SamplerState ss;
cbuffer ConstBuffer
{
	float2 destOffset;
	float2 destSize;
	float2 signalOffset;
	float2 signalSize;
	float2 samplingStep;
	float samplingRate;
	uint padding;
};
struct PsInputType
{
	float4 position : SV_POSITION;
	float2 coord : COORD;
};
float4 main(PsInputType input) : SV_TARGET
{
	const float2 samplingOffset = samplingStep * (samplingRate * 0.5f - 0.5f);

	float color = 0.0f;
	for (float i = 0.0f; i < samplingRate; i += 1.0f)
	{
		const float funcVal = funcTexture.Sample(ss, input.coord.x - samplingOffset.x + i * samplingStep.x).x;
		for (float j = 0.0f; j < samplingRate; j += 1.0f)
		{
			color += funcVal > (input.coord.y - samplingOffset.y + j * samplingStep.y) ? 1.0f : -1.0f;
		}
	}
	color = abs(color) / (samplingRate * samplingRate);

	if (input.coord.x < 0.0f || input.coord.x > 1.0f)
		color = 1.0f;

	return float4(0.0f, 1.0f - color, 0.0f, 1.0f);
}
)";
}

Graphics::Graphics() : m_viewPort{}
{
}

void Graphics::Init(HWND target)
{
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0
	};
	ThrowIfFailed(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &m_device3D, nullptr, &m_context3D), "Failed to create D3D device");
	ComPtr<IDXGIDevice> dxgiDevice;
	ThrowIfFailed(m_device3D->QueryInterface(&dxgiDevice), "Failed to query DXGI device");
	ComPtr<ID2D1Factory1> d2dFactory;
	ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&d2dFactory)), "Failed to create D2D factory");
	ThrowIfFailed(d2dFactory->CreateDevice(dxgiDevice.Get(), &m_device2D), "Failed to create D2D device");
	ThrowIfFailed(m_device2D->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_context2D), "Failed to create D2D device context");

	RECT rect;
	GetClientRect(target, &rect);

	ComPtr<IDXGIFactory> factory;
	ThrowIfFailed(CreateDXGIFactory(IID_PPV_ARGS(&factory)), "Failed to greate DXGI factory");

	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Width = max(1, rect.right);
	swapChainDesc.BufferDesc.Height = max(1, rect.bottom);
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = target;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;
	ThrowIfFailed(factory->CreateSwapChain(m_device3D.Get(), &swapChainDesc, &m_swapChain), "Failed to create swap chain");

	ComPtr<IDXGISurface> backBufferSurface;
	ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBufferSurface)), "Failed to get back buffer surface");
	ThrowIfFailed(m_context2D->CreateBitmapFromDxgiSurface(backBufferSurface.Get(), D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET |D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)), &m_targetBitmap), "Failed to create render tarteg bitmap");
	m_context2D->SetTarget(m_targetBitmap.Get());

	ComPtr<ID3D11Texture2D> backBufferTexture;
	ThrowIfFailed(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBufferTexture)), "Failed to get back buffer texture");
	ThrowIfFailed(m_device3D->CreateRenderTargetView(backBufferTexture.Get(), nullptr, &m_renderTargetView), "Failed to create render target view");
	m_context3D->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

	m_viewPort.TopLeftX = 0;
	m_viewPort.TopLeftY = 0;
	m_viewPort.Width = static_cast<float>(swapChainDesc.BufferDesc.Width);
	m_viewPort.Height = static_cast<float>(swapChainDesc.BufferDesc.Height);
	m_viewPort.MinDepth = 0;
	m_viewPort.MaxDepth = 1;
	m_context3D->RSSetViewports(1, &m_viewPort);

	m_context3D->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	float vertices[] = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f
	};
	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.ByteWidth = sizeof(vertices);
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	D3D11_SUBRESOURCE_DATA vertexInitData{};
	vertexInitData.pSysMem = vertices;
	ThrowIfFailed(m_device3D->CreateBuffer(&vertexBufferDesc, &vertexInitData, &m_vertexBuffer), "Failed to create vertex buffer");
	UINT stride = sizeof(float) * 2;
	UINT offset = 0;
	m_context3D->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

	ComPtr<ID3DBlob> shaderByteCode, errorText;
	HRESULT hr = D3DCompile(ShaderCode::g_VsCode, std::strlen(ShaderCode::g_VsCode), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &shaderByteCode, &errorText);
	if (FAILED(hr))
	{
		//std::cout << reinterpret_cast<const char*>(errorText->GetBufferPointer()) << std::endl;
		//OutputDebugStringA(reinterpret_cast<const char*>(errorText->GetBufferPointer()));
		throw std::runtime_error("Failed to compile vertex shader");
	}
	ThrowIfFailed(m_device3D->CreateVertexShader(shaderByteCode->GetBufferPointer(), shaderByteCode->GetBufferSize(), nullptr, &m_vertexShader), "Failed to create vertex shader");
	m_context3D->VSSetShader(m_vertexShader.Get(), nullptr, 0);

	D3D11_INPUT_ELEMENT_DESC inputLayoutDesc{};
	inputLayoutDesc.SemanticName = "POSITION";
	inputLayoutDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	inputLayoutDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	ThrowIfFailed(m_device3D->CreateInputLayout(&inputLayoutDesc, 1, shaderByteCode->GetBufferPointer(), shaderByteCode->GetBufferSize(), &m_vertexInputLayout), "Failed to create vertex input layout");
	m_context3D->IASetInputLayout(m_vertexInputLayout.Get());

	hr = D3DCompile(ShaderCode::g_PsCode, std::strlen(ShaderCode::g_PsCode), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &shaderByteCode, &errorText);
	if (FAILED(hr))
	{
		//std::cout << reinterpret_cast<const char*>(errorText->GetBufferPointer()) << std::endl;
		//OutputDebugStringA(reinterpret_cast<const char*>(errorText->GetBufferPointer()));
		throw std::runtime_error("Failed to compile pixel shader");
	}
	ThrowIfFailed(m_device3D->CreatePixelShader(shaderByteCode->GetBufferPointer(), shaderByteCode->GetBufferSize(), nullptr, &m_pixelShader), "Failed to create pixel shader");
	m_context3D->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	D3D11_BUFFER_DESC vsConstBufferDesc{};
	vsConstBufferDesc.ByteWidth = sizeof(ConstBuffer);
	vsConstBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vsConstBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vsConstBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vsConstBufferDesc.MiscFlags = 0;
	vsConstBufferDesc.StructureByteStride = 0;
	ThrowIfFailed(m_device3D->CreateBuffer(&vsConstBufferDesc, nullptr, &m_vsConstBuffer), "Failed to create vertex shader const buffer");
	m_context3D->VSSetConstantBuffers(0, 1, m_vsConstBuffer.GetAddressOf());
	m_context3D->PSSetConstantBuffers(0, 1, m_vsConstBuffer.GetAddressOf());

	D3D11_TEXTURE1D_DESC textureDesc{};
	textureDesc.Format = DXGI_FORMAT_R32_FLOAT;
	textureDesc.Width = FUNCTION_TEXTURE_SIZE;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.Usage = D3D11_USAGE_DYNAMIC;
	ThrowIfFailed(m_device3D->CreateTexture1D(&textureDesc, nullptr, &m_funcTexture), "Failed to create function texture");

	D3D11_SHADER_RESOURCE_VIEW_DESC funcTextureViewDesc{};
	funcTextureViewDesc.Format = textureDesc.Format;
	funcTextureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	funcTextureViewDesc.Texture1D.MipLevels = textureDesc.MipLevels;
	funcTextureViewDesc.Texture1D.MostDetailedMip = 0;
	ThrowIfFailed(m_device3D->CreateShaderResourceView(m_funcTexture.Get(), &funcTextureViewDesc, &m_functionTextureView), "Failed to create function texture view");
	m_context3D->PSSetShaderResources(0, 1, m_functionTextureView.GetAddressOf());
}

void Graphics::Resize(int width, int height)
{
	m_renderTargetView.Reset();
	m_context2D->SetTarget(nullptr);
	m_targetBitmap.Reset();
	ThrowIfFailed(m_swapChain->ResizeBuffers(2, max(1, width), max(1, height), DXGI_FORMAT_UNKNOWN, 0), "Failed to resize swapchain buffer");

	ComPtr<IDXGISurface> backBufferSurface;
	ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBufferSurface)), "Failed to get back buffer surface");
	ThrowIfFailed(m_context2D->CreateBitmapFromDxgiSurface(backBufferSurface.Get(), D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)), &m_targetBitmap), "Failed to create render tarteg bitmap");
	m_context2D->SetTarget(m_targetBitmap.Get());

	ComPtr<ID3D11Texture2D> backBufferTexture;
	ThrowIfFailed(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBufferTexture)), "Failed to get back buffer");
	ThrowIfFailed(m_device3D->CreateRenderTargetView(backBufferTexture.Get(), nullptr, &m_renderTargetView), "Failed to create render target view");
	m_context3D->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

	m_viewPort.Width = static_cast<float>(width);
	m_viewPort.Height = static_cast<float>(height);
	m_context3D->RSSetViewports(1, &m_viewPort);
}

void Graphics::BeginDraw() const
{
	const float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_context3D->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
	m_context2D->BeginDraw();
}

void Graphics::RenderFunc(const ConstBuffer& constBuffer, const float func[], const std::size_t length) const
{
	D3D11_MAPPED_SUBRESOURCE resource;

	ThrowIfFailed(m_context3D->Map(m_vsConstBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource));
	std::memcpy(resource.pData, &constBuffer, sizeof(ConstBuffer));
	m_context3D->Unmap(m_funcTexture.Get(), 0);

	ThrowIfFailed(m_context3D->Map(m_funcTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource));
	std::memcpy(resource.pData, func, length * sizeof(float));
	std::memset(reinterpret_cast<char*>(resource.pData) + length * sizeof(float), 0, (FUNCTION_TEXTURE_SIZE - length) * sizeof(float));
	m_context3D->Unmap(m_funcTexture.Get(), 0);

	m_context3D->Draw(6, 0);
}

void Graphics::EndDraw() const
{
	m_context2D->EndDraw();
	m_swapChain->Present(1, 0);
}