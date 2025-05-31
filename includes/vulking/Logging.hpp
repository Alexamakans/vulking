#pragma once

#include <array>
#include <string_view>

#ifndef PROJECT_ROOT
// Set by CMakeLists.txt, prefix to strip from __FILE__
#define PROJECT_ROOT ""
#endif

#ifdef NDEBUG
#define LOG_TRACE(...)
#define LOG_DEBUG(...)
#else
#define LOG_TRACE(x)                                                           \
  do {                                                                         \
    static constexpr auto _prefix = COMPTIME_PREFIX(__FILE__, __LINE__);       \
    std::cout << _prefix.c_str() << "[trce]: " << x << "\n";                   \
  } while (0)

#define LOG_DEBUG(x)                                                           \
  do {                                                                         \
    static constexpr auto _prefix = COMPTIME_PREFIX(__FILE__, __LINE__);       \
    std::cout << _prefix.c_str() << "[dbug]: " << x << "\n";                   \
  } while (0)
#endif

#define LOG_INFO(x)                                                            \
  do {                                                                         \
    static constexpr auto _prefix = COMPTIME_PREFIX(__FILE__, __LINE__);       \
    std::cout << _prefix.c_str() << "[info]: " << x << "\n";                   \
  } while (0)

#define LOG_WARNING(x)                                                         \
  do {                                                                         \
    static constexpr auto _prefix = COMPTIME_PREFIX(__FILE__, __LINE__);       \
    std::cout << _prefix.c_str() << "[warn]: " << x << "\n";                   \
  } while (0)

#define LOG_ERROR(x)                                                           \
  do {                                                                         \
    static constexpr auto _prefix = COMPTIME_PREFIX(__FILE__, __LINE__);       \
    std::cout << _prefix.c_str() << "[erro]: " << x << "\n";                   \
  } while (0)

constexpr std::size_t LOG_PREFIX_WIDTH = 40;
constexpr std::size_t MAX_PREFIX_SIZE = 256;

constexpr std::string_view strip_prefix(std::string_view full,
                                        std::string_view prefix) {
  return full.starts_with(prefix) ? full.substr(prefix.size()) : full;
}

constexpr int num_digits(int n) {
  int digits = 1;
  while (n >= 10) {
    n /= 10;
    digits++;
  }
  return digits;
}

constexpr void int_to_chars(int value, char *buffer_end, int length) {
  int v = value;
  for (int i = length - 1; i >= 0; i--) {
    buffer_end[i] = '0' + (v % 10);
    v /= 10;
  }
}

template <std::size_t N> struct Prefix {
  std::array<char, N> data{};
  std::size_t size = 0;

  constexpr const char *c_str() const { return data.data(); }
};

/// Produces a padded "filename:line" prefix, aligned to LOG_PREFIX_WIDTH with a
/// maximum length of N.
template <std::size_t N>
constexpr Prefix<N> build_prefix(std::string_view file, int line) {
  Prefix<N> prefix{};

  // Strip project root
  std::string_view short_file = strip_prefix(file, PROJECT_ROOT);

  // Copy filename into prefix.data
  std::size_t pos = 0;
  for (; pos < short_file.size() && pos < N - 1; ++pos) {
    prefix.data[pos] = short_file[pos];
  }

  prefix.data[pos++] = ':';

  // Write line number
  int digits = num_digits(line);

  // Safety check
  if (pos + digits >= N) {
    // Truncate if necessary
    digits = N - 1 - pos;
  }

  int_to_chars(line, &prefix.data[pos], digits);
  pos += digits;

  // Pad spaces until LOG_PREFIX_WIDTH
  while (pos < LOG_PREFIX_WIDTH && pos < N) {
    prefix.data[pos++] = ' ';
  }

  // Null terminate if space available
  if (pos < N) {
    prefix.data[pos] = '\0';
  } else {
    prefix.data[N - 1] = '\0';
  }

  prefix.size = pos;
  return prefix;
}

#define COMPTIME_PREFIX(file, line)                                            \
  ([]() constexpr -> auto {                                                    \
    return build_prefix<MAX_PREFIX_SIZE>(file, line);                          \
  }())
