// Copyright (c) 2020 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <utf/version.hpp>

namespace utf {
#ifndef UTF_NO_VERSION
	semver::project_version get_version() noexcept { return version; }
#endif
}  // namespace utf
