#pragma once
#include<type_traits>
#include<Windows.h>
#include<string>
#include<comdef.h>

namespace WindowsDemo {

	namespace Utils {

		inline std::string makeString(std::wstring wideString) {
			return std::string{ wideString.begin(), wideString.end() };
		}

		inline std::wstring makeWideString(std::string string) {
			return std::wstring{ string.begin(), string.end() };
		}

		namespace TypeTraits {

			template<typename Base, typename Derived>
			struct IsBaseOf : std::false_type {};

			template<typename Base, typename Derived>
			constexpr bool isBaseOf() {
				return IsBaseOf<Base, Derived>::value;
			}
		}
	}
}

