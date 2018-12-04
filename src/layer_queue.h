#pragma once

#include <array>
#include <vector>

// TODO: implement emplace
template <typename T>
class LayerQueue
{
	using Layer = std::vector<T>;

	std::array<Layer, 2> layers;

	std::size_t current_layer = 0;
	std::size_t current_index = 0;

	Layer& currentLayerVec() { return layers[current_layer%2]; }
	Layer& otherLayerVec() { return layers[(current_layer+1)%2]; }
	Layer const& currentLayerVec() const { return layers[current_layer%2]; }
	Layer const& otherLayerVec() const { return layers[(current_layer+1)%2]; }

public:
	LayerQueue() = default;

	void clear() {
		current_layer = 0;
		current_index = 0;
		layers[0].clear(); layers[1].clear();
	}

	void init(T const& first_element) {
		clear();
		layers[0].push_back(first_element);
	}

	void push(T const& element) {
		otherLayerVec().push_back(element);
	}

	T const& current() const {
		return currentLayerVec()[current_index];
	}

	// invalidates the reference returned by current()
	void step() {
		++current_index;
		if (current_index < currentLayerVec().size()) { return; }
		else { // next layer
			currentLayerVec().clear();
			current_index = 0;
			++current_layer;
		}
	}

	bool empty() const { return layers[0].empty() && layers[1].empty(); }
	// invalidates the reference returned by current()
	std::size_t currentLayer() const { return current_layer; }

	using const_iterator = typename std::vector<T>::const_iterator;
	const_iterator begin_current() const { return currentLayerVec().cbegin() + current_index; }
	const_iterator end_current() const { return currentLayerVec().cend(); }
	const_iterator begin_other() const { return otherLayerVec().cbegin(); }
	const_iterator end_other() const { return otherLayerVec().cend(); }
};
