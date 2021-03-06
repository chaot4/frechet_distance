#pragma once

#include <cstdint>
#include <functional>
#include <limits>

// Typesafe ID class such that there are compiler errors if different IDs are
// mixed. The template parameter T is just there to assure this behavior.
// Additionally, we have a member function which can check for validity.
template <typename T>
struct ID
{
public:
	using IDType = uint32_t;
	static constexpr IDType invalid_value = std::numeric_limits<IDType>::max();

	ID(IDType id = invalid_value) : id(id) {}
	ID(ID<T> const& other) : id(other.id) {}

	operator IDType() const { return id; }
	IDType operator+(ID<T> other) const { return id + other.id; }
	IDType operator+(int offset) const { return id + offset; }
	IDType operator+(size_t offset) const { return id + offset; }
	IDType operator-(ID<T> other) const { return id - other.id; }
	IDType operator-(int offset) const { return id - offset; }
	IDType operator/(int div) const { return id/div; }
	IDType operator+=(ID<T> other) { return id += other.id; }
	IDType operator-=(ID<T> other) { return id -= other.id; }
	IDType operator=(ID<T> other) { return id = other.id; }
	IDType operator++() { return ++id; }
	IDType operator--() { return --id; }
	// FIXME:
	// bool operator==(ID<T> other) const { return id == other.id; }
	// bool operator==(IDType other) const { return id == other; }
	bool operator!=(ID<T> other) const { return id != other.id; }

	bool valid() const { return id != invalid_value; }
	void invalidate() { id = invalid_value; }

private:
	IDType id;
};

// define custom hash function to be able to use IDs with maps/sets
namespace std
{

template <typename T>
struct hash<ID<T>>
{
	using IDType = typename ID<T>::IDType;
	std::size_t operator()(ID<T> const& id) const noexcept
	{
		return std::hash<IDType>()(id);
	}
};

} // std
