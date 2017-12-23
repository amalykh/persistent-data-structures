#pragma once
#include "shared.h"
#include "vector_persistent_node.h"
#include <memory>
#include <vector>

namespace persistent {
	template<class T, class Allocator = std::allocator<T>>
	class ReferenceNode : public PersistentNode {
		ReferenceNode(std::vector<T, Allocator>& source_data, VersionID version) : version(version) {
			data = source_data;
		}
		ReferenceNode(std::vector<T, Allocator>& source_data, int ind, T& value, VersionID version) :
			DataNode(source_data, version) {
			data[idx] = T;
		}
		ReferenceNode<T> change(int ind, std::vector<T, Allocator>* value, VersionID version) {
			if (value == nullptr && ind + 1 == references.size()) return pop(version);
			if (version != NO_VERSION && this->version == version && ind < references.size()) {
				references[ind] = *value;
			}
		}

	private:
		std::vector<T, Allocator> references;
		VersionID version;
	};
} // namespace persistent