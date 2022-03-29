#include <gtest/gtest.h>
#include <utf/utf.hpp>
#include <utf/version.hpp>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace utf::testing {
	using namespace ::std::literals;
	using ::testing::TestWithParam;
	using ::testing::ValuesIn;

	enum class op {
		none,
#ifdef __cpp_lib_char8_t
		utf8to16,
		utf8to32,
		utf16to8,
		utf32to8,
		str8to8,
		u8to8s,
#endif
		str8to16,
		str8to32,
		utf16to8s,
		utf32to8s,
		utf16to32,
		utf32to16,
	};

	struct string_convert {
		const std::string str8;
#ifdef __cpp_lib_char8_t
		const std::u8string _u8;
#endif
		const std::u16string utf16;
		const std::u32string utf32;
		op oper{op::none};
	};

	template <typename Out>
	struct utf_helper;
#ifdef __cpp_lib_char8_t
	template <>
	struct utf_helper<char8_t> {
		template <typename View>
		static auto conv(View str) {
			return as_u8(str);
		}
	};
#endif
	template <>
	struct utf_helper<char> {
		template <typename View>
		static auto conv(View str) {
			return as_str8(str);
		}
	};
	template <>
	struct utf_helper<char16_t> {
		template <typename View>
		static auto conv(View str) {
			return as_u16(str);
		}
	};
	template <>
	struct utf_helper<char32_t> {
		template <typename View>
		static auto conv(View str) {
			return as_u32(str);
		}
	};

	template <typename Out, typename In>
	static std::basic_string<Out> utf_as(std::basic_string_view<In> in) {
		return utf_helper<Out>::conv(in);
	}

	void print(std::ostream& out, std::string const& s) { out << s; }

#ifdef __cpp_lib_char8_t
	void print(std::ostream& out, std::u8string const& s) {
		out << std::string_view{reinterpret_cast<char const*>(s.data()),
		                        s.size()};
	}
#endif

	void print(std::ostream& out, std::u16string const& s) {
#if 0
		for (auto c : s) {
			if (c < 127) {
				out << static_cast<char>(c);
				continue;
			}
			char buffer[100];
			snprintf(buffer, sizeof buffer, "\\u%04x",
			         static_cast<unsigned>(c));
			out << buffer;
		}
#else
		print(out, as_str8(s));
#endif
	}

	void print(std::ostream& out, std::u32string const& s) {
#if 0
		for (auto c : s) {
			if (c < 127) {
				out << static_cast<char>(c);
				continue;
			}
			char buffer[100];
			snprintf(buffer, sizeof buffer, "\\U%08x",
			         static_cast<unsigned>(c));
			out << buffer;
		}
#else
		print(out, as_str8(s));
#endif
	}

	template <typename String>
	struct printable {
		String val;
		std::string_view tag{};
		bool operator==(printable const& right) const noexcept {
			return val == right.val;
		}

		friend std::ostream& operator<<(std::ostream& out,
		                                printable const& print) {
			utf::testing::print(out, print.val);
			if (!print.tag.empty()) out << " [" << print.tag << ']';
			return out;
		}
	};

	template <typename String>
	printable<String> make_printable(String const& str) {
		return {str};
	}

	template <typename Char>
	std::string_view tag_v;

	template <>
	std::string_view tag_v<char> = "char"sv;

	template <>
	std::string_view tag_v<char16_t> = "u16"sv;

	template <>
	std::string_view tag_v<char32_t> = "u32"sv;

#ifdef __cpp_char8_t
	template <>
	std::string_view tag_v<char8_t> = "u8"sv;
#endif

	struct utf_conv : TestWithParam<string_convert> {
		template <typename Out, typename In>
		void ExpectUtfEq(const std::basic_string<Out>& expected_str,
		                 const std::basic_string<In>& tested) {
			using printable_t = printable<std::basic_string<Out>>;
			auto expected = printable_t{expected_str, tag_v<Out>};
			auto actual = printable_t{utf_as<Out, In>(tested), tag_v<In>};
			EXPECT_EQ(expected, actual);
		}

		static void SetUpTestSuite() {
#ifdef _WIN32
			SetConsoleOutputCP(CP_UTF8);
#endif
		}
	};

	TEST(utf, version) {
		EXPECT_EQ(version, get_version());
		EXPECT_TRUE(version.compatible_with(get_version()));
	}

	struct utf_errors : utf_conv {};

	TEST_P(utf_conv, utf8) {
#ifdef __cpp_lib_char8_t
		auto [s8, u8, u16, u32, ign] = GetParam();
#else
		auto [s8, u16, u32, ign] = GetParam();
#endif
		ExpectUtfEq(s8, u16);
		ExpectUtfEq(s8, u32);

		EXPECT_TRUE(is_valid(s8));

#ifdef __cpp_lib_char8_t
		ExpectUtfEq(s8, u8);
		ExpectUtfEq(u8, u16);
		ExpectUtfEq(u8, u32);
		ExpectUtfEq(u8, s8);

		EXPECT_TRUE(is_valid(u8));
#endif
	}

	TEST_P(utf_conv, utf16) {
#ifdef __cpp_lib_char8_t
		auto [s8, u8, u16, u32, ign] = GetParam();
#else
		auto [s8, u16, u32, ign] = GetParam();
#endif
		ExpectUtfEq(u16, s8);
#ifdef __cpp_lib_char8_t
		ExpectUtfEq(u16, u8);
#endif
		ExpectUtfEq(u16, u32);

		EXPECT_TRUE(is_valid(u16));
	}

	TEST_P(utf_conv, utf32) {
#ifdef __cpp_lib_char8_t
		auto [s8, u8, u16, u32, ign] = GetParam();
#else
		auto [s8, u16, u32, ign] = GetParam();
#endif
		ExpectUtfEq(u32, s8);
#ifdef __cpp_lib_char8_t
		ExpectUtfEq(u32, u8);
#endif
		ExpectUtfEq(u32, u16);

		EXPECT_TRUE(is_valid(u32));
	}

	TEST_P(utf_errors, check) {
#ifdef __cpp_lib_char8_t
		auto [s8, u8, u16, u32, oper] = GetParam();
#else
		auto [s8, u16, u32, oper] = GetParam();
#endif
		switch (oper) {
#ifdef __cpp_lib_char8_t
			case op::utf8to16:
				ExpectUtfEq(u16, u8);
				EXPECT_EQ(!u16.empty(), is_valid(u8)) << make_printable(u8);
				break;
			case op::utf8to32:
				ExpectUtfEq(u32, u8);
				EXPECT_EQ(!u32.empty(), is_valid(u8)) << make_printable(u8);
				break;
			case op::utf16to8:
				ExpectUtfEq(u8, u16);
				EXPECT_EQ(!u8.empty(), is_valid(u16)) << make_printable(u16);
				break;
			case op::utf32to8:
				ExpectUtfEq(u8, u32);
				EXPECT_EQ(!u8.empty(), is_valid(u32)) << make_printable(u32);
				break;
			case op::str8to8:
				ExpectUtfEq(u8, s8);
				EXPECT_EQ(!u8.empty(), is_valid(s8)) << make_printable(s8);
				break;
			case op::u8to8s:
				ExpectUtfEq(s8, u8);
				EXPECT_EQ(!s8.empty(), is_valid(u8)) << make_printable(u8);
				break;
#endif
			case op::str8to16:
				ExpectUtfEq(u16, s8);
				EXPECT_EQ(!u16.empty(), is_valid(s8)) << make_printable(s8);
				break;
			case op::str8to32:
				ExpectUtfEq(u32, s8);
				EXPECT_EQ(!u32.empty(), is_valid(s8)) << make_printable(s8);
				break;
			case op::utf16to8s:
				ExpectUtfEq(s8, u16);
				EXPECT_EQ(!s8.empty(), is_valid(u16)) << make_printable(u16);
				break;
			case op::utf16to32:
				ExpectUtfEq(u32, u16);
				EXPECT_EQ(!u32.empty(), is_valid(u16)) << make_printable(u16);
				break;
			case op::utf32to8s:
				ExpectUtfEq(s8, u32);
				EXPECT_EQ(!s8.empty(), is_valid(u32)) << make_printable(u32);
				break;
			case op::utf32to16:
				ExpectUtfEq(u16, u32);
				EXPECT_EQ(!u16.empty(), is_valid(u32)) << make_printable(u32);
				break;
			default:
				break;
		};
	}

	template <class... Char>
	std::u32string utf32(Char... chars) {
		char32_t data[] = {static_cast<char32_t>(chars)..., 0u};
		return data;
	}

	template <class... Char>
	std::u16string utf16(Char... chars) {
		char16_t data[] = {static_cast<char16_t>(chars)..., 0u};
		return data;
	}

#ifdef __cpp_lib_char8_t
	template <class... Char>
	std::u8string utf8(Char... chars) {
		char8_t data[] = {static_cast<char8_t>(chars)..., 0u};
		return data;
	}
#endif

	template <class Char>
	char u2s(Char c) {
		return static_cast<char>(static_cast<unsigned char>(c));
	}
	template <class... Char>
	std::string utf8s(Char... chars) {
		char data[] = {u2s(chars)..., 0u};
		return data;
	}

#ifdef __cpp_lib_char8_t
#define S8(...) , __VA_ARGS__
#else
#define S8(...)
#endif

#define U8STR(X) X S8(u8##X)

	const string_convert strings[] = {
	    {},
	    {U8STR("ascii"), u"ascii", U"ascii"},
	    {U8STR("\x24"), utf16(0x24), utf32(0x24)},
	    {"\xc2\xa2" S8(u8"\u00a2"), utf16(0xa2), utf32(0xa2)},
	    {"\xe2\x82\xac" S8(u8"\u20ac"), utf16(0x20ac), utf32(0x20ac)},
	    {"\xf0\x90\x8d\x88" S8(u8"\U00010348"), utf16(0xd800, 0xdf48),
	     utf32(0x10348u)},
	    {U8STR("vȧĺũê\0vȧĺũêş"s), u"vȧĺũê\0vȧĺũêş"s, U"vȧĺũê\0vȧĺũêş"s},
	    {U8STR("ŧĥê qũïçķ Ƌȓôŵñ ƒôx ĵũmpş ôvêȓ ȧ ĺȧȥÿ đôğ"),
	     u"ŧĥê qũïçķ Ƌȓôŵñ ƒôx ĵũmpş ôvêȓ ȧ ĺȧȥÿ đôğ",
	     U"ŧĥê qũïçķ Ƌȓôŵñ ƒôx ĵũmpş ôvêȓ ȧ ĺȧȥÿ đôğ"},
	    {U8STR("ȾĦȄ QÙÍÇĶ ßŔÖŴÑ ƑÖX ĴÙMPŞ ÖVȄŔ Ä ȽÄȤÝ ÐÖĠ"),
	     u"ȾĦȄ QÙÍÇĶ ßŔÖŴÑ ƑÖX ĴÙMPŞ ÖVȄŔ Ä ȽÄȤÝ ÐÖĠ",
	     U"ȾĦȄ QÙÍÇĶ ßŔÖŴÑ ƑÖX ĴÙMPŞ ÖVȄŔ Ä ȽÄȤÝ ÐÖĠ"}};

	INSTANTIATE_TEST_SUITE_P(strings, utf_conv, ValuesIn(strings));

	const string_convert bad[] = {
	    {utf8s('a', 'b', 0xe0, 0x9f, 0x9f) S8({}), {}, {}, op::str8to32},
	    {utf8s('a', 'b', 0xed, 0xa0, 0xa0) S8({}), {}, {}, op::str8to32},
	    {utf8s('a', 'b', 0xf0, 0x8f, 0x8f, 0x8f) S8({}), {}, {}, op::str8to32},
	    {utf8s('a', 'b', 0xf4, 0x90, 0x90, 0x90) S8({}), {}, {}, op::str8to32},

	    {utf8s('a', 'b', 0xef, 0xbf, 0xbd, 'c', 'd') S8({}),
	     {},
	     utf32('a', 'b', 0x00110000u, 'c', 'd'),
	     op::utf32to8s},
	    {{} S8({}),
	     utf16('a', 'b', 0xFFFD, 'c', 'd'),
	     utf32('a', 'b', 0x00110000u, 'c', 'd'),
	     op::utf32to16},
	    {utf8s('a', 'b', 0xf4, 0x8f, 0xbf, 0xbf, 'c', 'd') S8({}),
	     {},
	     utf32('a', 'b', 0x0010FFFFu, 'c', 'd'),
	     op::utf32to8s},
	    {{} S8({}),
	     utf16('a', 'b', 0xDBFF, 0xDFFF, 'c', 'd'),
	     utf32('a', 'b', 0x0010FFFFu, 'c', 'd'),
	     op::utf32to16},
	    {utf8s('a', 'b', 0xef, 0xbf, 0xbd, 'c', 'd') S8({}),
	     {},
	     utf32('a', 'b', 0xD811, 'c', 'd'),
	     op::utf32to8s},
	    {{} S8({}),
	     utf16('a', 'b', 0xFFFD, 'c', 'd'),
	     utf32('a', 'b', 0xD811, 'c', 'd'),
	     op::utf32to16},
	    {utf8s('a', 'b', 0xef, 0xbf, 0xbd, 'c', 'd') S8({}),
	     {},
	     utf32('a', 'b', 0x00110000u, 'c', 'd'),
	     op::utf32to8s},

	    // timer clock: U+23F2 E2:8F:B2
	    {utf8s('a', 'b', 0xe2, 0x8f) S8({}), {}, {}, op::str8to32},
	    {utf8s('a', 'b', 0xe2, 0x8f, '-') S8({}), {}, {}, op::str8to32},
	    {utf8s('a', 'b', 0xff, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xfe, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xfd, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xfc, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xfb, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xfa, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xf0, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xf1, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xf2, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xf3, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xf4, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xf3, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xf2, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xf1, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xf0, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xef, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xee, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xed, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xec, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xeb, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xea, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xe0, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xe9, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xe8, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xe7, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xe6, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xe5, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xe4, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xe3, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xe1, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xe0, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xdf, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xde, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xdd, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xdc, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xdb, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xda, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xd0, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xd9, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xd8, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xd7, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xd6, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xd5, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xd4, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xd3, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xd1, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},
	    {utf8s('a', 'b', 0xd0, '-', '-', '-', '-', '-') S8({}),
	     {},
	     {},
	     op::str8to32},

#ifdef __cpp_lib_char8_t
	    {{}, utf8('a', 'b', 0xe0, 0x9f, 0x9f), {}, {}, op::utf8to32},
	    {{}, utf8('a', 'b', 0xed, 0xa0, 0xa0), {}, {}, op::utf8to32},
	    {{}, utf8('a', 'b', 0xf0, 0x8f, 0x8f, 0x8f), {}, {}, op::utf8to32},
	    {{}, utf8('a', 'b', 0xf4, 0x90, 0x90, 0x90), {}, {}, op::utf8to32},

	    {{},
	     utf8('a', 'b', 0xef, 0xbf, 0xbd, 'c', 'd'),
	     {},
	     utf32('a', 'b', 0x00110000u, 'c', 'd'),
	     op::utf32to8},
	    {{},
	     utf8('a', 'b', 0xf4, 0x8f, 0xbf, 0xbf, 'c', 'd'),
	     {},
	     utf32('a', 'b', 0x0010FFFFu, 'c', 'd'),
	     op::utf32to8},
	    {{},
	     utf8('a', 'b', 0xef, 0xbf, 0xbd, 'c', 'd'),
	     {},
	     utf32('a', 'b', 0xD811, 'c', 'd'),
	     op::utf32to8},
	    {{},
	     utf8('a', 'b', 0xef, 0xbf, 0xbd, 'c', 'd'),
	     {},
	     utf32('a', 'b', 0x00110000u, 'c', 'd'),
	     op::utf32to8},

	    {{}, utf8('a', 'b', 0xe2, 0x8f), {}, {}, op::utf8to32},
	    {{}, utf8('a', 'b', 0xe2, 0x8f, '-'), {}, {}, op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xff, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xfe, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xfd, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xfc, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xfb, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xfa, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xf0, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xf1, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xf2, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xf3, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xf4, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xf3, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xf2, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xf1, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xf0, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xef, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xee, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xed, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xec, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xeb, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xea, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xe0, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xe9, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xe8, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xe7, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xe6, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xe5, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xe4, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xe3, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xe1, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xe0, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xdf, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xde, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xdd, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xdc, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xdb, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xda, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xd0, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xd9, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xd8, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xd7, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xd6, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xd5, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xd4, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xd3, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xd1, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
	    {{},
	     utf8('a', 'b', 0xd0, '-', '-', '-', '-', '-'),
	     {},
	     {},
	     op::utf8to32},
#endif  // __cpp_lib_char8_t
	};

	INSTANTIATE_TEST_SUITE_P(bad, utf_errors, ValuesIn(bad));

	const string_convert those_four[] = {
	    {utf8s(0xe0, 0x9f, 0x80) S8({}), {}, {}, op::str8to32},
	    {utf8s(0xe0, 0xa0, 0x80) S8({}), {}, utf32(0x0800u), op::str8to32},
	    {utf8s(0xe0, 0xa1, 0x80) S8({}), {}, utf32(0x0840u), op::str8to32},

	    {utf8s(0xed, 0x9e, 0x80) S8({}), {}, utf32(0xD780u), op::str8to32},
	    {utf8s(0xed, 0x9f, 0x80) S8({}), {}, utf32(0xD7C0u), op::str8to32},
	    {utf8s(0xed, 0xa0, 0x80) S8({}), {}, {}, op::str8to32},

	    {utf8s(0xf0, 0x8f, 0x80, 0x80) S8({}), {}, {}, op::str8to32},
	    {utf8s(0xf0, 0x90, 0x80, 0x80) S8({}),
	     {},
	     utf32(0x10000u),
	     op::str8to32},
	    {utf8s(0xf0, 0x91, 0x80, 0x80) S8({}),
	     {},
	     utf32(0x11000u),
	     op::str8to32},

	    {utf8s(0xf4, 0x8e, 0x80, 0x80) S8({}),
	     {},
	     utf32(0x10E000u),
	     op::str8to32},
	    {utf8s(0xf4, 0x8f, 0x80, 0x80) S8({}),
	     {},
	     utf32(0x10F000u),
	     op::str8to32},
	    {utf8s(0xf4, 0x90, 0x80, 0x80) S8({}), {}, {}, op::str8to32},

	    {utf8s(0x80) S8({}), {}, {}, op::str8to32},

#ifdef __cpp_lib_char8_t
	    {{}, utf8(0xe0, 0x9f, 0x80), {}, {}, op::utf8to32},
	    {{}, utf8(0xe0, 0xa0, 0x80), {}, utf32(0x0800u), op::utf8to32},
	    {{}, utf8(0xe0, 0xa1, 0x80), {}, utf32(0x0840u), op::utf8to32},

	    {{}, utf8(0xed, 0x9e, 0x80), {}, utf32(0xD780u), op::utf8to32},
	    {{}, utf8(0xed, 0x9f, 0x80), {}, utf32(0xD7C0u), op::utf8to32},
	    {{}, utf8(0xed, 0xa0, 0x80), {}, {}, op::utf8to32},

	    {{}, utf8(0xf0, 0x8f, 0x80, 0x80), {}, {}, op::utf8to32},
	    {{}, utf8(0xf0, 0x90, 0x80, 0x80), {}, utf32(0x10000u), op::utf8to32},
	    {{}, utf8(0xf0, 0x91, 0x80, 0x80), {}, utf32(0x11000u), op::utf8to32},

	    {{}, utf8(0xf4, 0x8e, 0x80, 0x80), {}, utf32(0x10E000u), op::utf8to32},
	    {{}, utf8(0xf4, 0x8f, 0x80, 0x80), {}, utf32(0x10F000u), op::utf8to32},
	    {{}, utf8(0xf4, 0x90, 0x80, 0x80), {}, {}, op::utf8to32},

	    {{}, utf8(0x80), {}, {}, op::utf8to32},
#endif
	};
	INSTANTIATE_TEST_SUITE_P(those_four, utf_errors, ValuesIn(those_four));
}  // namespace utf::testing
