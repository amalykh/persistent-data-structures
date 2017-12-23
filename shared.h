#pragma once
#include <cstdint>
#include <cassert>

namespace persistent {
	const int32_t C_BITS = 5;
	const int32_t C_SIZE = (1 << C_BITS);
	typedef int64_t VersionID;
} // namespace persistent