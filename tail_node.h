#pragma once
#include "shared.h"
#include <memory>

namespace persistent {
	template<class T, class Allocator = allocator<T>>
	class TailNode {
	public:
		TailNode(VersionID version) : version(version) {
			data = allocator.allocate(C_SIZE);
		}
	private:
		T* data;
		VersionID version;
		int32_t last_change;
		Allocator allocator;
	};
} // namespace persistent