#pragma once

#include "../Shared/shared.h"
#include <utility>
#include <vector>

using std::pair;
using std::vector;

namespace persistent {

	enum NodeState {
		M_Nil,
		M_Value,
		M_Collision,
		M_Reference
	};

	enum MapNodeRelation {
		MR_Equal,
		MR_Collide,
		MR_Different
	};

	template<class K, class V, class Allocator = std::allocator<pair<K, V>>>
	class MPersistentNode {
		using MPN = MPersistentNode <K, V, Allocator>;
		using MPNPtr = shared_ptr<MPN>;
	public:
		virtual NodeState getNodeStateAt(int i) = 0;
		virtual V& getValueAt(int i, NodeState state, K& key) = 0;
		virtual MPNPtr addValueItemAt(int i, K& key, V& value, VersionID version) = 0;
		virtual MPNPtr createCollisionAt(int i, K& key, V& value, VersionID version) = 0;
		virtual MPNPtr addToColisionAt(int i, K& key, V& value, VersionID version) = 0;
		virtual MapNodeRelation relationWithNodeAt(K& key, int idx, NodeState state) = 0;
		virtual MPNPtr ñhangeReference(int idx, MPNPtr mapNode, VersionID version) = 0;
		virtual MPNPtr ñreateNewNodeFrom(int oldIdx, K& key, V& value, int idx1, int idx2, VersionID version) = 0;
		virtual MPNPtr ñreateReference(int index, MPNPtr mapNode, NodeState state, VersionID version) = 0;
		virtual MPNPtr createReferenceNode(int idx, MPNPtr node, VersionID version) = 0;
		virtual MPNPtr changeValue(int idx, NodeState state, K& key, V& value, VersionID version) = 0;
		virtual bool isKeyAt(int idx, NodeState state, K key) = 0;
		virtual int valueCount() = 0;
		virtual int referenceCount() = 0;
		virtual MPNPtr removeValue(int idx, NodeState state, K& key, VersionID version) = 0;
		virtual MPNPtr merge(MPNPtr newNode, int index, VersionID version) = 0;
		virtual MPNPtr makeRoot(VersionID version) = 0;
	};

	template<class K, class V, class Allocator = std::allocator<pair<K, V>>>
	class MCollisionCollection {
		using MPC = MPersistentNode <K, V, Allocator>;
		using MPCPtr = shared_ptr<MPC>;
	public:
		virtual int32_t getHash() = 0;
		virtual MPCPtr add(K& key, V& item, VersionID version) = 0;
		virtual MPCPtr remove(K& key, VersionID version) = 0;
		virtual MPCPtr change(K& key, V& value, VersionID version) = 0;
		virtual MPCPtr getItem(K& key) = 0;
		virtual bool hasItemWithKey(K& key) = 0;
		virtual int32_t size() = 0;
		virtual pair<K, V> getRemainingValue(K& removedKey) = 0;
	};
}