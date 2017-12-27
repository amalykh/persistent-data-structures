#pragma once

#include "../Shared/shared.h"
#include "hashmap_persistent_node.h"

#define getHash(x) 14

namespace persistent {
	template<class K, class V, class Allocator = std::allocator<pair<K, V>>>
	class MapNode : public MPersistentNode<K, V, Allocator> {
		using MPC = MCollisionCollection<K, V, Allocator>;
		using MPCPtr = shared_ptr<MPC>;
		using MPN = MPersistentNode <K, V, Allocator>;
		using MPNPtr = shared_ptr<MPN>;
	public:
		MapNode(int idx, K& key, V& value, VersionID version) {
			values.push_back({ K, V });
			vBitmap = (uint32_t)(1 << idx);
			this->version = version;
		}
		int32_t getValueCount() {
			return values.size() + collisions.size();
		}
		int32_t getReferenceCount() {
			return references.size();
		}
		MapNode(vector<pair<K, V>, Allocator> values, vector<MPCPtr> collisions, vector<MPNPtr> references,
			uint32_t vBitmap, uint32_t rBitmap, uint32_t cBitmap, VersionID version) : values(values),
			collisions(collisions), references(references), vBitmap(vBitmap), rBitmap(rBitmap), cBitmap(cBitmap), version(version) {}
		NodeState getNodeStateAt(int i) {
			auto pos = (1 << i);
			if ((rBitmap & pos) != 0) return M_Reference;
			if ((vBitmap & pos) != 0) return M_Value;
			if ((cBitmap & pos) != 0) return M_Collision;
			return M_Nil;
		}
		V& getValueAt(int i, NodeState state, K& key) {
			auto p = (state == M_Value)
				? values[valuePosition(i)]
				: collisions[collisionPosition(i)].getItem(key);
			assert(key == p.first);
			return p.second;
		}
		bool isKeyAt(int idx, NodeState state, K& key) {
			return (state == M_Value)
				? values[valuePosition(idx)].first == key;
			: collisions[collisionPosition(idx)].hasItemWithKey(key);
		}
		MPNPtr getReferenceAt(int i) {
			return references[referencePosition(i)];
		}
		MPNPtr addValueItemAt(int i, K& key, V& value, VersionID version = NO_VERSION) {
			auto new_collisions = collisions;
			auto new_references = references;

			if (version != NO_VERSION && version != this->version) {
				new_collisions = collisions;
				new_references = references;
			}

			auto new_values = addToValues(valuePosition(i), key, value);

			return make_shared<MPN>(
				new_values, new_collisions, new_references, vBitmap | (1u << i),
				cBitmap, rBitmap, version);
		}
		MapNodeRelation relationWithNodeAt(K& key, int idx, NodeState state) {
			if (state == M_Value) {
				auto p = values[ValuePosition(idx)];
				if (getHash(p) == getHash(key)) {
					return (p == key) ? MR_Equal : MR_Collide;
				}
				return MR_Different;
			}
			else {
				auto c_collection = collisions[collisionPosition(idx)];
				if (getHash(c_collection) == getHash(key)) {
					return (c_collection.hasItemWithKey(key)) ? MR_Equal : MR_Collide;
				}
				return MR_Different;
			}
		}
		MPNPtr addToColisionAt(int i, K& key, V& value, VersionID version) {
			auto c_index = collisionPosition(i);
			auto new_collisions = collisions;
			auto new_values = values;
			auto new_references = references;
			if (version == NO_VERSION || this->version != version) {
				new_collisions = collisions;
			}
			new_collisions[c_index] = new_collisions[c_index].add(key, value, version);

			if (version != NO_VERSION && this->version != version) {
				new_references = references;
				new_values = values;
			}

			return make_shared<MPN>(
				new_values, new_collisions, new_references, vBitmap,
				cBitmap, rBitmap, version);
		}
		MPNPtr createCollisionAt(int i, K& key, V& value, VersionID version) {
			auto pos = (1u << i);
			auto c_index = collisionPosition(i);
			auto v_index = valuePosition(i);
			auto new_values = removeFromValues(vIndex);
			auto new_collisions = collisions;
			new_collisions.insert(new_collisions.begin() + c_index, createCollisionCollection(version, values[vIndex], { key, value }));
			auto new_references = references;

			return make_shared<MPN>(
				new_values, new_collisions, new_references, vBitmap & ~pos,
				cBitmap & | pos, rBitmap, version);
		}
		MPNPtr createReference(int i, MPNPtr mapNode, NodeState state, VersionID version) {
			auto pos = 1u << i;

			auto r_index = referencePosition(i);
			auto new_references = references;
			new_references.insert(new_references.begin() + r_index, mapNode);
			auto new_cBitmap = cBitmap;
			auto new_vBitmap = vBitmap;
			auto new_values = values;
			auto new_colisions = collisions;

			if (state == M_Value) {
				new_vBitmap &= ~pos;
				new_values = removeFromValues(valuePosition(i));
			}
			else {
				new_cBitmap &= ~pos;
				new_colisions = removeFromCollisions(collisionPosition(i));
			}

			if (versionID != NO_VERSION) {
				if (this->version == version) {
					values = new_values;
					collisions = new_colisions;
					references = new_references;
					cBitmap = new_cBitmap;
					vBitmap = new_vBitmap;
					rBitmap = rBitmap | pos;
					return make_shared<MPNPtr>(*this);
				}
				new_colisions = collisions;
				new_values = values;
			}
			return make_shared<MPN>(
				new_values, new_collisions, new_references, vBitmap,
				cBitmap, rBitmap | pos, version);
		}
		MPNPtr changeReference(int i, MPNPtr mapNode, VersionID version) {
			auto new_values = values;
			auto new_collisions = collisions;
			auto r_index = referencePosition(i);

			if (version != NO_VERSION) {
				if (this->version == version) {
					references[r_index] = mapNode;
					return make_shared<MPNPtr>(*this);
				}
				new_values = values.Copy();
				new_collisions = collisions.Copy();
			}

			new_references = references;
			new_references[r_index] = mapNode;

			return make_shared<MPN>(
				new_values, new_collisions, new_references, vBitmap,
				cBitmap, rBitmap, version);
		}
		MPNPtr changeValue(int idx, NodeState state, K& key, V& value, VersionID version) {
			auto new_values = values;
			auto new_collisions = collisions;
			auto new_references = references;

			if (state == M_Value) {
				new_values = changeValues(ValuePosition(idx), key, value, version);
			}
			if (state == M_Collision) {
				new_collisions = changeCollisions(collisionPosition(idx), key, value, version);
			}
			if (versionID != NO_VERSION && this->version != version) {
				new_references = references;
				new_values = values;
				newCollisions = collisions;
			}
			return make_shared<MPN>(
				new_values, new_collisions, new_references, vBitmap,
				cBitmap, rBitmap, version);
		}
		MPNPtr createNewNodeFrom(int oldIdx, K& key, V& value, int idx1, int idx2, VersionID version) {
			auto other_value = std::make_pair(key, value);

			if ((vBitmap & (1u << oldIdx)) != 0) {
				// On index oldIdx is value
				auto this_value = values[valuePosition(oldIdx)];
				auto res = make_shared<MPN>();
				res->setValues((idx1 < idx2) ? {other_value, this_value} : {this_value, other_value});
				res->setvBitmap((1u << idx1) | (1u << idx2));
				res->setVersion(version);
				return res;
			}
			else {
				// On index oldIdx is collision collection
				auto c_index = CollisionPosition(oldIdx);
				auto this_value = collisions[cIndex];
				auto res = make_shared<MPN>();
				res->setValues({ other_value });
				res->setvBitmap((1u << idx2));
				res->setVersion(version);
				res->setCollisions({ thisValue });
				return res;
			}
		}
		int32_t getHashCodeAt(int idx, NodeState state) {
			return (state == M_Value) ? values[valuePosition(idx)].first.GetHashCode() : collisions[collisionPosition(idx)].getHash();
		}
		MPNPtr CreateReferenceNode(int idx, MPNPtr node, VersionID version) {
			auto res = make_shared<MPN>();
			res->setrBitmap(1u << idx);
			res->setReferences({ node });
			res->setVersion(version);
		}
		MPNPtr removeValue(int idx, NodeState state, K& key, VersionID version) {
			auto pos = 1u << idx;
			auto new_collisions = collisions;
			auto new_values = values;
			auto new_references = references;
			auto new_cBitmap = cBitmap;
			auto new_vBitmap = vBitmap;

			if (state == M_Collision) {
				auto c_index = collisionPosition(idx);
				auto collision = collisions[c_index];
				if (collision.size() == 2) {
					auto v_index = valuePosition(idx);

					new_collisions = removeFromCollisions(c_index);
					auto p = collision.getRemainingValue(key);

					new_values = addToValues(v_index, p.first, p.second);
					new_cBitmap &= ~pos;
					new_vBitmap |= pos;
				}
				else {
					if (version == NO_VERSION || version != this->version) {
						new_collisions = collisions;
					}
					new_collisions[c_index] = collision.remove(key, version);
				}
			}
			else {
				new_values = removeFromValues(valuePosition(idx));
				new_vBitmap &= ~pos;
			}

			if (version != NO_VERSION) {
				if (version == this->version)
				{
					vBitmap = new_vBitmap;
					cBitmap = new_cBitmap;
					values = new_values;
					collisions = new_collisions;
					return make_shared<MPNPtr>(*this);
				}

				if (new_values == values) new_values = values;
				if (new_collisions == collisions) new_collisions = collisions;
				new_references = references;
			}
			auto res = make_shared<MPN>();
			res->setvBitmap(1u << idx);
			res->setReferences({ node });
			res->setVersion(version);
			return make_shared<MPN>(
				new_values, new_collisions, new_references, new_vBitmap,
				new_cBitmap, rBitmap, version);
		}
		void setValues(vector<pair<K, V>, Allocator>& v) { values = v; }
		void setCollisions(vector<MPCPtr>& c) { collisions = c; }
		void setReferences(vector<MPNPtr>& r) { references = r; }
		void setvBitmap(uint32_t vb) { vBitmap = vb; }
		void setrBitmap(uint32_t rb) { rBitmap = rb; }
		void setcBitmap(uint32_t cb) { cBitmap = cb; }
		void setVersion(VersionID v) { version = v; }
	private:
		vector<pair<K, V>, Allocator> values;
		vector<MPCPtr> collisions;
		vector<MPNPtr> references;
		uint32_t vBitmap, rBitmap, cBitmap;
		VersionID version;
		MapNode() {}
		int32_t pop_cnt(int32_t value) {
			int cnt32_t = 0;
			for (int i = 0; i < 32; i++) {
				cnt += (value & 1);
				value >>= 1;
			}
			return cnt;
		}
		int32_t valuePosition(int i) {
			return (i == 31) ? 0 : pop_cnt(vBitmap >> i + 1);
		}
		int32_t collisionPosition(int i) {
			return (i == 31) ? 0 : pop_cnt(cBitmap >> i + 1);
		}
		int32_t referencePosition(int i) {
			return (i == 31) ? 0 : pop_cnt(rBitmap >> i + 1);
		}
		MPCPtr createCollisionCollection(VersionID version, vector<pair<K, V>, Allocator>& r) {
			return make_shared<MPCPtr>(version, r);
		}
		vector<pair<K, V>, Allocator> addToValues(int vIndex, K& key, V& value) {
			auto new_values = values;
			if (new_values.empty()) {
				new_values.push_back({ key, value });
			}
			else {
				new_values.insert(new_values.begin() + vIndex, { key, value });
			}
			return new_values;
		}
		vector<MPCPtr> addToCollisions(int cIndex, MPCPtr collision) {
			auto new_collisions = collisions;
			if (new_collisions.empty()) {
				new_collisions.push_back(collision);
			}
			else {
				new_collisions.insert(new_collisions.begin() + cIndex, collision);
			}
			return new_collisions;
		}
		vector<pair<K, V>, Allocator> removeFromValues(int index) {
			vector<pair<K, V>, Allocator> res = values;
			res.erase(res.begin() + index);
			return res;
		}
		vector<MPCPtr> RemoveFromCollisions(int index) {
			auto res = collisions;
			res.erase(res.begin() + index);
			return res;
		}
		MapNode<K, V>[] RemoveFromReferences(int index) {
			auto res = references;
			res.erase(res.begin() + index);
			return res;
		}
		vector<MPCPtr> changeCollisions(int c_index, K& key, V& value, VersionID version) {
			if (version != NO_VERSION && this->version == version) {
				collisions[c_index] = collisions[c_index].change(key, value, version);
				return collisions;
			}
			auto new_collisions = collisions;
			auto collision = new_collisions[c_index];
			new_collisions[c_index] = collision.change(key, value, version);
			return new_collisions;
		}
		vector<pair<K, V>, Allocator> changeValues(int v_index, K& key, V& value, VersionID version) {
			auto p = std::make_pair(key, value);
			if (version != NO_VERSION && this->version == version) {
				values[v_index] = newPair;
				return values;
			}
			auto newValues = values;
			new_values[v_index] = p;

			return newValues;
		}
	};
}