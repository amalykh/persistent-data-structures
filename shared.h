#pragma once
#include <cstdint>
#include <cassert>
#include <memory>

using std::shared_ptr;
using std::make_shared;
using std::static_pointer_cast;
using std::dynamic_pointer_cast;

namespace persistent {
	const int32_t C_BITS = 5;
	const int32_t C_SIZE = (1 << C_BITS);
	const int32_t ABITS = 0x01f;
	const int64_t NO_VERSION = -1;
	typedef int64_t VersionID;
} // namespace persistent