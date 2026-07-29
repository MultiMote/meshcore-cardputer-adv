#include <cctype>

class Helpers {
public:
  static bool containsIgnoreCase(const char *str, const char *search) {
    if (*search == '\0') {
      return true;
    }

    for (; *str != '\0'; ++str) {
      const char *s1 = str;
      const char *s2 = search;

      while (*s1 && *s2 &&
             std::tolower(static_cast<unsigned char>(*s1)) == std::tolower(static_cast<unsigned char>(*s2))) {
        ++s1;
        ++s2;
      }

      if (*s2 == '\0') {
        return true;
      }
    }

    return false;
  }

  // In case if input has utf-8 multibyte characters
  static void removeLastStringChar(String &str) {
    unsigned int len = str.length();
    if (len == 0) {
      return;
    }

    unsigned int bytesToRemove = 1;

    while (len - bytesToRemove > 0 && ((uint8_t)str.charAt(len - bytesToRemove) & 0xC0) == 0x80) {
      bytesToRemove++;
    }

    str.remove(len - bytesToRemove, bytesToRemove);
  }
};
