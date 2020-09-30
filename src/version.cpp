// Copyright (c) 2020 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <utf/version.hpp>

namespace utf {
	semver::project_version get_version() noexcept { return version; }
}  // namespace utf
