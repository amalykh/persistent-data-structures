#pragma once
#include "shared.h"
#include "vector_persistent_node.h"
#include <memory>
#include <vector>
#include "tail_node.h"
#include "reference_node.h"
#include "abstract_pv.h"
#include "data_node.h"
#include "nodes_converter.h"

namespace persistent {
	template<class T, class Allocator = std::allocator<T>>
	class PersistentVector : public AbstractPV<T, Allocator> {
		using PersNode = PersistentNode<T, Allocator>;
		using PersNodePtr = shared_ptr<PersNode>;
		using TNode = TailNode<T, Allocator>;
		using TNodePtr = shared_ptr<TNode>;
		using RefNode = ReferenceNode<T, Allocator>;
		using RefNodePtr = shared_ptr<RefNode>;
		using DNode = DataNode<T, Allocator>;
		using DNodePtr = shared_ptr<DNode>;
		using PV = PersistentVector<T, Allocator>;
	public:

		PersistentVector(PersNodePtr root, int32_t count, int32_t depth, TNodePtr tail) : 
			AbstractPV<T, Allocator>(root, count, depth, tail) {}

		static PersistentVector getEmpty() {
			return PV(nullptr, 0, 0, nullptr);
		}

		PersistentVector(size_t sz) : PV(nullptr, 0, 0, nullptr) {
			// TODO: this must be done using transient arrays!
			for (int i = 0; i < sz; i++) (*this) = this->push_back(T());
		}

		PV push_back(T value) {
			// initializing PV with one element
			if (count == 0) return PV(nullptr, 1, 0, make_shared<TNode>(value, NO_VERSION));
			
			// adding element to tail if it is possible
			if (!isTailFull()) {
				return PV(root, count + 1, depth, tail->push(value, count & ABITS, version));
			}

			// if all levels with this depth are filled, need to add next depth level
			if (isFull()) {
				auto new_tail = make_shared<TNode>(value, version);
				auto old_tail = NodesConverter<T, Allocator>::toDataNode(tail, version);
				std::vector<PersNodePtr> r;
				if (depth == 0) {
					r.push_back(root);
					r.push_back(old_tail);
				}
				else {
					r.push_back(root);
					r.push_back(newPath(depth * C_BITS, old_tail));
				}
				return PV(RefNode::createWithItems(version, r), count + 1, depth + 1, new_tail);
			}
			// we have space in tree, but tail is full - copy info from tail
			PersNodePtr new_root = (root == nullptr) ? (NodesConverter<T, Allocator>::toDataNode(tail, version)) :
				(addTail(static_pointer_cast<RefNode>(root), NodesConverter<T, Allocator>::toDataNode(tail, version), depth * C_BITS));
			return PV(new_root, count + 1, depth, make_shared<TNode>(value, version));
		}
		PV pop_back() {
			assert(count);
			if (count == 1) return getEmpty();
			if (count % 32 != 1) return PV(root, count - 1, depth, tail);
			auto new_tail = getNodeAt(count - 2);
			auto new_root = removeTail(depth * C_BITS, root);
			auto new_depth = depth;
			if (depth > 1 && static_pointer_cast<RefNode>(new_root)->size() == 1) {
				new_root = (*static_pointer_cast<RefNode>(new_root))[0];
				--new_depth;
			}
			return PV(new_root, count - 1, new_depth, NodesConverter<T, Allocator>::toTailNode(new_tail, version));
		}
		PV setAt(int index, T val) {
			assert(index >= 0 && index < count);
			if (index >= getTailOffset()) return PV(root, count, depth, tail->change(index & ABITS, val, version));
			if (dynamic_pointer_cast<DNode>(root)) {
				return PV((static_pointer_cast<DNode>(root))->change(index & ABITS, val), count,
					depth, tail);
			}
			return PV(changeValue(depth * C_BITS, static_pointer_cast<RefNode>(root), index, val), count,
				depth, tail);
		}
		/*~PersistentVector() {
			cout << "awa";
		}*/
	private:
	};

}