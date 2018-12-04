#pragma once

// Having the iterator as template parameter enables us to also
// define ContainerRanges with const_iterators.
template <typename T, typename iterator = typename T::iterator>
struct ContainerRange
{
	ContainerRange(iterator begin, iterator end)
		: _begin(begin), _end(end) {};

	iterator begin() const { return _begin; };
	iterator end() const { return _end; };

private:
	iterator const _begin;
	iterator const _end;
};

template <typename T>
class ValueRange
{
public:
	class iterator
	{
	private:
		T current_value;

	public:
		explicit iterator(T value) : current_value(value) {}

		iterator operator++() { ++current_value; return *this; }
		bool operator==(iterator other) const { return current_value == other.current_value; }
		bool operator!=(iterator other) const { return !(*this == other); }
		T operator*() const { return current_value; }
	};

	ValueRange(T begin, T end)
		: _begin(begin), _end(end), _range_size(end-begin) {}

	iterator begin() const { return _begin; }
	iterator end() const { return _end; }
	std::size_t size() const { return _range_size; }

private:
	iterator const _begin;
	iterator const _end;
	std::size_t _range_size;
};
