#pragma once
#include "shared.h"
#include "vector_persistent_node.h"
#include <memory>
#include <vector>

namespace persistent {
	template<class T, class Allocator = std::allocator<T>>
	class ReferenceNode : public PersistentNode<T, Allocator> {
		ReferenceNode(VersionID version) : version(version) {}
		ReferenceNode(VersionID version, int size) : ReferenceNode(version) {
			references.resize(size);
		}
		ReferenceNode<T, Allocator> CreateWithItems(VersionID version, std::vector<PersistentNode<T, Allocator>, Allocator> nodes) {
			auto res = ReferenceNode<T, Allocator>(version);
			res.setReferences(nodes);
			return res;
		}
		ReferenceNode<T, Allocator>& operator[] (int x) {
			return references[x];
		}
		ReferenceNode<T, Allocator> pop(VersionID version) {
			assert(!references.empty());
			auto cp = ReferenceNode<T, Allocator>(version, references.size() - 1);
			cp.setReferences(references);
			return cp;
		}
		ReferenceNode<T, Allocator> change(int ind, std::vector<T, Allocator>* value, VersionID version) {
			if (value == nullptr && ind + 1 == references.size()) return pop(version);
			if (version != NO_VERSION && this->version == version && ind < references.size()) {
				references[ind] = *value;
			}
			auto size = (ind < references.size()) ? references.size() : ind + 1;
			auto cp = ReferenceNode<T, Allocator>(version, size);
			cp.setReferences(references);
			cp[ind] = value;
			return cp;
		}
		std::vector<T, Allocator>& getReferences() { return references; }
		void setReferences(std::vector<T, Allocator> refs) { references = refs; }
	private:
		std::vector<PersistentNode<T, Allocator>, Allocator> references;
		VersionID version;
	};
} // namespace persistent