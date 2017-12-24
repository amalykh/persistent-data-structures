#pragma once

namespace persistent {
	template<class T, class Allocator = std::allocator<T>>
	class PersistentNode {
	public:
		virtual ~PersistentNode(){}
	};
} // namespace persistent