#pragma once
#include "shared.h"
#include "vector_persistent_node.h"
#include <memory>
#include <vector>

namespace persistent {
	template<class T, class Allocator = std::allocator<T>>
	class ReferenceNode : public PersistentNode<T, Allocator> {
		using PersNode = PersistentNode<T, Allocator>;
		using PersNodePtr = shared_ptr<PersNode>;
		using RefNode = ReferenceNode<T, Allocator>;
		using RefNodePtr = shared_ptr<RefNode>;
	public:
		ReferenceNode(VersionID version) : version(version) {}
		ReferenceNode(VersionID version, int size) : ReferenceNode(version) {
			references.resize(size);
		}
		static RefNodePtr createWithItems(VersionID version, std::vector<PersNodePtr>& nodes) {
			RefNodePtr res = make_shared<RefNode>(version);
			res->setReferences(nodes);
			return res;
		}
		PersNodePtr& operator[] (int x) {
			return references[x];
		}
		RefNodePtr pop(VersionID version) {
			assert(!references.empty());
			auto cp = make_shared<RefNode>(version, size() - 1);
			cp->setReferences(references);
			return cp;
		}
		RefNodePtr change(int ind, PersNodePtr value, VersionID version) {
			if (value == nullptr && ind + 1 == references.size()) return pop(version);
			if (version != NO_VERSION && this->version == version && ind < size()) {
				references[ind] = value; // as long as version is same, can just change data
				return RefNodePtr(this);
			}
			auto sz = std::max(size(), ind + 1);
			auto cp = make_shared<RefNode>(version, sz);
			for (int i = 0; i < size(); i++) (*cp)[i] = references[i];
			(*cp)[ind] = value;
			return cp;
		}
		std::vector<PersNodePtr>& getReferences() { return references; }
		void setReferences(std::vector<PersNodePtr> refs) { references = refs; }
		int32_t size() { return references.size(); }
		~ReferenceNode() {}
	private:
		std::vector<PersNodePtr> references;
		VersionID version;
	};
} // namespace persistent