#pragma once
#include "../Shared/shared.h"
#include "vector_persistent_node.h"
#include <memory>
#include <vector>
#include "tail_node.h"
#include "reference_node.h"
#include "data_node.h"

namespace persistent {
	template<class T, class Allocator = std::allocator<T>>
	class AbstractPV {
		using PersNode = PersistentNode<T, Allocator>;
		using PersNodePtr = shared_ptr<PersNode>;
		using TNode = TailNode<T, Allocator>;
		using TNodePtr = shared_ptr<TNode>;
		using RefNode = ReferenceNode<T, Allocator>;
		using RefNodePtr = shared_ptr<RefNode>;
		using DNode = DataNode<T, Allocator>;
		using DNodePtr = shared_ptr<DNode>;
	public:
		//Properties
		bool isFull() { return (count >> C_BITS) > (1 << C_BITS * depth); }
		bool isTailFull() { return count > 0 && count % 32 == 0; }
		int32_t getTailOffset() { return (((count - 1) >> C_BITS) << C_BITS); }
		bool isEmpty() { return count == 0; }
		int32_t getCount() { return count; }

		AbstractPV(PersNodePtr root, int32_t count, int32_t depth, TNodePtr tail) :
			root(root), count(count), depth(depth), tail(tail) {
			version = NO_VERSION;
		}
		// TODO: add build conversion from list

		// Auxilary methods
		PersNodePtr removeTail(int level, PersNodePtr node) {
			int subix = ((count - 2) >> level) & ABITS;
			if (level > C_BITS) {
				RefNodePtr ref_node = static_pointer_cast<RefNode>(node);
				auto new_child = removeTail(level - C_BITS, (*ref_node)[subix]);
				if (new_child == nullptr && subix == 0) return nullptr;
				else return ref_node->change(subix, new_child, version);
			}
			else if (subix == 0) return nullptr;
			else {
				if ((dynamic_pointer_cast<RefNode>(node)) == NULL) return nullptr;
				RefNodePtr ref_node = static_pointer_cast<RefNode>(node);
				return ref_node->pop(version);
			}
		}
		PersNodePtr addTail(RefNodePtr parent, DNodePtr tail, int32_t level) {
			int subidx = ((count - 1) >> level) & ABITS;
			if (level == C_BITS) {
				return parent->change(subidx, tail, version);
			}
			else {
				if (subidx < parent->size()) {
					PersNodePtr sub_ptr = (*parent)[subidx];
					return parent->change(subidx, addTail(static_pointer_cast<RefNode>(sub_ptr), tail, level - C_BITS), version);
				}
				else {
					return parent->change(subidx, newPath(level - C_BITS, tail), version);
				}
			}
		}
		PersNodePtr changeValue(int level, RefNodePtr node, int ind, T& val) {
			int subidx = (ind >> level) & ABITS;
			if (level == C_BITS) {
				auto temp = static_pointer_cast<DNode>((*node)[subidx]);
				return node->change(subidx, temp->change(ind & ABITS, val, version), version);
			}
			auto sub = static_pointer_cast<RefNode>((*node)[subidx]);
			auto changedVal = changeValue(level - C_BITS, sub, ind, val);
			//if (changedVal == sub) return node;
			return node->change(subidx, changedVal, version);
		}
		DNodePtr getNodeAt(int index) {
			assert(!(index < 0 || index >= count));
			auto node = root;
			for (int lvl = depth * 5; lvl > 0; lvl -= C_BITS) {
				node = (*(static_pointer_cast<RefNode>(node)))[(index >> lvl) & ABITS];
			}
			return (static_pointer_cast<DNode>(node));
		}
		PersNodePtr newPath(int level, PersNodePtr node) {
			if (level == 0) return node;
			std::vector<PersNodePtr> r;
			r.push_back(newPath(level - C_BITS, node));
			return RefNode::createWithItems(version, r);
		}

		// Necessary functions
		T& operator[] (int i) {
			return getAt(i);
		}
		T& getAt(int index) {
			assert(index >= 0 && index < count);
			if (index >= getTailOffset()) return (*tail)[index & ABITS];
			auto node = root;
			for (int lvl = depth * C_BITS; lvl > 0; lvl -= C_BITS) {
				node = (*static_pointer_cast<RefNode>(node))[(index >> lvl) & ABITS];
			}
			return (*(static_pointer_cast<DNode>(node)))[index & ABITS];
		}
		int32_t size() { return count; }
		virtual ~AbstractPV() {}
	protected:
		PersNodePtr root;
		int32_t count;
		int32_t depth;
		TNodePtr tail;
		VersionID version;
	};
} // namespace persistent