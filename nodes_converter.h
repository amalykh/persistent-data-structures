#pragma once
#include "shared.h"
#include "vector_persistent_node.h"
#include <memory>
#include <vector>
#include "tail_node.h"
#include "reference_node.h"
#include "data_node.h"

namespace persistent {
	template<class T, class Allocator = std::allocator<T>>
	class NodesConverter {
		using PersNode = PersistentNode<T, Allocator>;
		using PersNodePtr = shared_ptr<PersNode>;
		using TNode = TailNode<T, Allocator>;
		using TNodePtr = shared_ptr<TNode>;
		using RefNode = ReferenceNode<T, Allocator>;
		using RefNodePtr = shared_ptr<RefNode>;
		using DNode = DataNode<T, Allocator>;
		using DNodePtr = shared_ptr<DNode>;
	public:
		static DNodePtr toDataNode(TNodePtr tail_node, VersionID version) {
			return make_shared<DNode>(tail_node->getData(), version);
		}
		static TNodePtr toTailNode(DNodePtr data_node, VersionID version) {
			auto temp = make_shared<TNode>(version);
			for (int i = 0; i < C_SIZE; i++) {
				(*temp)[i] = (*data_node)[i];
			}
			temp->setLastChanged(C_SIZE - 1);
			return temp;
		}
	};
}