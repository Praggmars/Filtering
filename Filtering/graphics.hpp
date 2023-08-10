#pragma once

#include "common.hpp"
#include "alg.hpp"

struct ConstBuffer
{
	Transform<float2> rectTransform;
	Transform<float2> signalTransform;
	float2 samplingStep;
	float samplingRate;
	unsigned padding;
};

class Graphics
{
	ComPtr<ID3D11Device> m_device3D;
	ComPtr<ID3D11DeviceContext> m_context3D;
	ComPtr<ID2D1Device> m_device2D;
	ComPtr<ID2D1DeviceContext> m_context2D;
	ComPtr<IDXGISwapChain> m_swapChain;
	ComPtr<ID2D1Bitmap1> m_targetBitmap;
	ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	ComPtr<ID3D11RasterizerState> m_rasterizerState;
	D3D11_VIEWPORT m_viewPort;
	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11InputLayout> m_vertexInputLayout;
	ComPtr<ID3D11PixelShader> m_pixelShader;
	ComPtr<ID3D11Buffer> m_vsConstBuffer;
	ComPtr<ID3D11Texture1D> m_funcTexture;
	ComPtr<ID3D11ShaderResourceView> m_functionTextureView;

public:
	Graphics();

	void Init(HWND target);
	void Resize(int width, int height);
	void BeginDraw() const;
	void RenderFunc(const ConstBuffer& constBuffer, const float func[], const std::size_t length) const;
	void EndDraw() const;

	inline ID3D11Device* Device3D() const { return m_device3D.Get(); }
	inline ID3D11DeviceContext* Context3D() const { return m_context3D.Get(); }
	inline ID2D1Device* Device2D() const { return m_device2D.Get(); }
	inline ID2D1DeviceContext* Context2D() const { return m_context2D.Get(); }
};