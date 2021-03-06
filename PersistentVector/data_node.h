#pragma once
#include "../Shared/shared.h"
#include "vector_persistent_node.h"
#include <memory>

namespace persistent {
	template<class T, class Allocator = std::allocator<T>>
	class DataNode : public PersistentNode<T, Allocator> {
		using DNode = DataNode<T, Allocator>;
		using DNodePtr = shared_ptr<DNode>;
	public:
		T& operator[] (int x) {
			return data[x];
		}
		DataNode(std::vector<T, Allocator>& source_data, VersionID version) : version(version) {
			data = source_data;
		}
		DataNode(std::vector<T, Allocator>& source_data, int ind, T& value, VersionID version) :
			DataNode(source_data, version) {
			data[ind] = value;
		}
		DNodePtr change(int ind, T& value, VersionID version = NO_VERSION) {
			if (version != NO_VERSION && this->version == version) {
				data[ind] = value;
				return make_shared<DNode>(*this);
			}
			return make_shared<DNode>(data, ind, value, version);
		}
		std::vector<T, Allocator>& getData() { return data; }
		virtual ~DataNode() {}
	private:
		std::vector<T, Allocator> data;
		VersionID version;
	};
} // namespace persistent