#pragma once
#include "shared.h"
#include "vector_persistent_node.h"
#include <memory>
#include <vector>

namespace persistent {
	template<class T, class Allocator = std::allocator<T>>
	class ReferenceNode : public PersistentNode<T, Allocator> {
		using RefNodePtr = ReferenceNode<T, Allocator>*;
		using RefNode = ReferenceNode<T, Allocator>;
		ReferenceNode(VersionID version) : version(version) {}
		ReferenceNode(VersionID version, int size) : ReferenceNode(version) {
			references.resize(size);
		}
		RefNodePtr CreateWithItems(VersionID version, std::vector<PersistentNode<T, Allocator>, Allocator> nodes) {
			RefNodePtr res = make_shared<RefNode>(version);
			res->setReferences(nodes);
			return res;
		}
		ReferenceNode<T, Allocator>& operator[] (int x) {
			return references[x];
		}
		RefNodePtr pop(VersionID version) {
			assert(!references.empty());
			auto cp = make_shared<RefNode>(version, references.size() - 1);
			cp->setReferences(references);
			return cp;
		}
		RefNodePtr change(int ind, std::vector<T, Allocator>* value, VersionID version) {
			if (value == nullptr && ind + 1 == references.size()) return pop(version);
			if (version != NO_VERSION && this->version == version && ind < references.size()) {
				references[ind] = *value;
				return make_shared<RefNode>(*this);
			}
			auto size = (ind < references.size()) ? references.size() : ind + 1;
			auto cp = make_shared<RefNode>(version, size);
			cp->setReferences(references);
			(*cp)[ind] = value;
			return cp;
		}
		std::vector<T, Allocator>& getReferences() { return references; }
		void setReferences(std::vector<T, Allocator> refs) { references = refs; }
	private:
		std::vector<PersistentNode<T, Allocator>, Allocator> references;
		VersionID version;
	};
} // namespace persistent