# UTF converter

[![Travis (.org)][Travis badge]][Travis]
[![Coveralls github][Coveralls badge]][Coveralls]
[![Cpp Standard][17-badge]][17]
[![Cpp Standard][20-badge]][20]

Open-source library providing conversion between `string`, `u16string`,
`u32string` and `u8string`. It is platform-independent and uses the Unicode
UTF code as its basis.

This library distinguishes `std::string` and `std::u8string` under C++20,
but still assumes the `std::string` objects contain UTF-8 values.

## Synopsis

```cpp
#include <utf/utf.hpp>
```

Apart from `as_u8(string_view)` and `as_str8(u8string_view)`, all the
functions decode each Unicode code point of the input (using `uint32_t` as
the _interlingua_) and encode it in the output. If the decoding fails, an
empty string is returned.

Conversion between `string[_view]` and `u8string[_view]` is done by simple
re-interpretation of the contents.

Versions marked with "_C++20_" comment are only available, if the standard
library defines `__cpp_lib_char8_t`.

### utf::is_valid

```cpp
bool utf::is_valid(std::u8string_view src);         // C++20
bool utf::is_valid(std::string_view src);
bool utf::is_valid(std::u16string_view src);
```

Tries to decode the string one character at a time and returns `false` as
soon as decoding fails; otherwise, returns `true`. If the `utf::is_valid`
returns `false` for any argument, then any `is_xxx` function will return an
empty string for the same argument.

```cpp
bool utf::is_valid(std::u32string_view src);
```

Returns `true`.

### utf::as_u8

```cpp
std::u8string utf::as_u8(std::u16string_view src);  // C++20
std::u8string utf::as_u8(std::u32string_view src);  // C++20
std::u8string utf::as_u8(std::string_view src);     // C++20
```

_(C++20)_ Converts other UTF strings to `std::u8string`. The behavior is
that of `utf::as_str8`, except for the type of the character used.

### utf::as_str8

```cpp
std::string utf::as_str8(std::u8string_view src);   // C++20
std::string utf::as_str8(std::u16string_view src);
std::string utf::as_str8(std::u32string_view src);
```

Converts other UTF strings to `std::string` encoded as UTF-8. If compiled as
C++20, the behavior is that of `utf::as_u8`, except for the type of the
character used.

### utf::as_str8

```cpp
std::u16string utf::as_u16(std::u8string_view src); // C++20
std::u16string utf::as_u16(std::string_view src);
std::u16string utf::as_u16(std::u32string_view src);
```

Converts other UTF strings to `std::u16string`.

### utf::as_u32

```cpp
std::u32string utf::as_u32(std::u8string_view src); // C++20
std::u32string utf::as_u32(std::string_view src);
std::u32string utf::as_u32(std::u16string_view src);
```

Converts other UTF strings to `std::u32string`.

```cpp
#include <utf/version.hpp>
```

### utf::version

```cpp
constexpr semver::project_version utf::version;
```

Current version of the library to link against.

### utf::get_version

```cpp
semver::project_version utf::get_version();
```

Current version of loaded library (if used in dynamic linking) or the same
value as `utf::version` (if used in static linking).

### UTFCONV_NAME macro

```cpp
#define UTFCONV_NAME "utfconv"
```

Name of the library

### UTFCONV_VERSION macros

```cpp
#define UTFCONV_VERSION_MAJOR
#define UTFCONV_VERSION_MINOR
#define UTFCONV_VERSION_PATCH
#define UTFCONV_VERSION_STABILITY
```

C macros representing the same information, as `utf::version` variable, that
is `UTFCONV_VERSION_MAJOR` / `MINOR` / `PATCH` have the same values, as
`utf::version.get_major()` / `get_minor()` / `get_patch()`.
`UTFCONV_VERSION_STABILITY` contains the same string, that would be returned
by `utf::version.get_prerelease().to_string()`, that is, either an empty
string, or string starting with a hyphen for easy version strings
concatenation.

[Travis badge]: https://img.shields.io/travis/mbits-libs/utfconv?style=flat-square
[Travis]: https://travis-ci.org/mbits-libs/utfconv "Travis-CI"
[Coveralls badge]: https://img.shields.io/coveralls/github/mbits-libs/utfconv?style=flat-square
[Coveralls]: https://coveralls.io/github/mbits-libs/utfconv "Coveralls"
[17-badge]: https://img.shields.io/badge/C%2B%2B-17-informational?style=flat-square
[17]: https://en.wikipedia.org/wiki/C%2B%2B17 "Wikipedia C++17"
[20-badge]: https://img.shields.io/badge/C%2B%2B-20-informational?style=flat-square
[20]: https://en.wikipedia.org/wiki/C%2B%2B20 "Wikipedia C++20"
