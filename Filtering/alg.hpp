#pragma once

template <typename T>
class vec2
{
public:
	T x;
	T y;

public:
	vec2() : x{}, y{} {}
	vec2(T v) : x{ v }, y{ v } {}
	vec2(T x, T y) : x{ x }, y{ y } {}

	vec2 operator+(vec2 rhs) const { return vec2(x + rhs.x, y + rhs.y); }
	vec2 operator-(vec2 rhs) const { return vec2(x - rhs.x, y - rhs.y); }
	vec2 operator*(vec2 rhs) const { return vec2(x * rhs.x, y * rhs.y); }
	vec2 operator/(vec2 rhs) const { return vec2(x / rhs.x, y / rhs.y); }
	vec2 operator+=(vec2 rhs) { x += rhs.x; y += rhs.y; return *this; }
	vec2 operator-=(vec2 rhs) { x -= rhs.x; y -= rhs.y; return *this; }
	vec2 operator*=(vec2 rhs) { x *= rhs.x; y *= rhs.y; return *this; }
	vec2 operator/=(vec2 rhs) { x /= rhs.x; y /= rhs.y; return *this; }

	vec2 operator+(T rhs) const { return vec2(x + rhs, y + rhs); }
	vec2 operator-(T rhs) const { return vec2(x - rhs, y - rhs); }
	vec2 operator*(T rhs) const { return vec2(x * rhs, y * rhs); }
	vec2 operator/(T rhs) const { return vec2(x / rhs, y / rhs); }
	vec2 operator+=(T rhs) { x += rhs; y += rhs; return *this; }
	vec2 operator-=(T rhs) { x -= rhs; y -= rhs; return *this; }
	vec2 operator*=(T rhs) { x *= rhs; y *= rhs; return *this; }
	vec2 operator/=(T rhs) { x /= rhs; y /= rhs; return *this; }
	vec2 operator=(T rhs) { x = rhs; y = rhs; return *this; }

	template <typename T2> vec2<T2> As() const { return vec2<T2>(static_cast<T2>(x), static_cast<T2>(y)); }
};
template <typename T> vec2<T> operator+(T lhs, vec2<T> rhs) { return vec2<T>(lhs + rhs.x, lhs + rhs.y); }
template <typename T> vec2<T> operator-(T lhs, vec2<T> rhs) { return vec2<T>(lhs - rhs.x, lhs - rhs.y); }
template <typename T> vec2<T> operator*(T lhs, vec2<T> rhs) { return vec2<T>(lhs * rhs.x, lhs * rhs.y); }
template <typename T> vec2<T> operator/(T lhs, vec2<T> rhs) { return vec2<T>(lhs / rhs.x, lhs / rhs.y); }

template <typename T>
struct Transform
{
	T offset;
	T scaler;

	Transform() : offset{}, scaler{} {}
	Transform(T offset, T scaler) : offset{ offset }, scaler{ scaler } {}
	Transform(T srcBegin, T srcEnd, T dstBegin, T dstEnd) :
		offset{ (srcEnd * dstBegin - srcBegin * dstEnd) / (srcEnd - srcBegin) },
		scaler{ (dstEnd - dstBegin) / (srcEnd - srcBegin) } {}
	Transform(const Transform& t1, const Transform& t2) :
		offset{ t1.offset * t2.scaler + t2.offset },
		scaler{ t1.scaler * t2.scaler } {}

	T Forward(T v) const { return v * scaler + offset; }
	T Inverse(T v) const { return (v - offset) / scaler; }
};

using int2 = vec2<int>;
using uint2 = vec2<unsigned>;
using float2 = vec2<float>;
using double2 = vec2<double>;