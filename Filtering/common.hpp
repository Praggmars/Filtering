#pragma once

#include <Windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <d2d1_1.h>
#include <d3dcompiler.h>
#include <vector>

template <typename T>
class ComPtr
{
	T* m_ptr;

public:
	ComPtr() : m_ptr{} {}
	~ComPtr() { Reset(); }

	void Reset()
	{
		if (m_ptr) 
		{
			m_ptr->Release();
			m_ptr = nullptr; 
		}
	}
	T* Get() const { return m_ptr; }
	T** GetAddressOf() { return &m_ptr; }
	T** operator&() { Reset(); return &m_ptr; }
	T* operator->() const { return m_ptr; }
};

void ThrowIfFailed(HRESULT result, const char* message = "Unknown error");