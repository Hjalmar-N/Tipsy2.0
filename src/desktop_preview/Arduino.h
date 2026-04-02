#pragma once

#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>

class String {
 public:
  String() = default;
  String(const char* value) : value_(value == nullptr ? "" : value) {}
  String(const std::string& value) : value_(value) {}
  String(std::string&& value) : value_(std::move(value)) {}
  String(char value) : value_(1, value) {}

  template <typename T,
            typename = std::enable_if_t<std::is_arithmetic_v<T> &&
                                        !std::is_same_v<std::decay_t<T>, char> &&
                                        !std::is_same_v<std::decay_t<T>, bool>>>
  String(T value) : value_(std::to_string(value)) {}

  String& operator=(const char* value) {
    value_ = value == nullptr ? "" : value;
    return *this;
  }

  bool isEmpty() const {
    return value_.empty();
  }

  const char* c_str() const {
    return value_.c_str();
  }

  std::size_t length() const {
    return value_.length();
  }

  void reserve(std::size_t count) {
    value_.reserve(count);
  }

  String& operator+=(const String& rhs) {
    value_ += rhs.value_;
    return *this;
  }

  String& operator+=(const char* rhs) {
    value_ += rhs == nullptr ? "" : rhs;
    return *this;
  }

  String& operator+=(char rhs) {
    value_ += rhs;
    return *this;
  }

  template <typename T,
            typename = std::enable_if_t<std::is_arithmetic_v<T> &&
                                        !std::is_same_v<std::decay_t<T>, char> &&
                                        !std::is_same_v<std::decay_t<T>, bool>>>
  String& operator+=(T rhs) {
    value_ += std::to_string(rhs);
    return *this;
  }

  friend bool operator==(const String& lhs, const String& rhs) {
    return lhs.value_ == rhs.value_;
  }

  friend bool operator==(const String& lhs, const char* rhs) {
    return lhs.value_ == (rhs == nullptr ? "" : rhs);
  }

  friend bool operator==(const char* lhs, const String& rhs) {
    return rhs == lhs;
  }

  friend bool operator!=(const String& lhs, const String& rhs) {
    return !(lhs == rhs);
  }

  friend bool operator!=(const String& lhs, const char* rhs) {
    return !(lhs == rhs);
  }

  friend bool operator!=(const char* lhs, const String& rhs) {
    return !(lhs == rhs);
  }

  friend String operator+(String lhs, const String& rhs) {
    lhs += rhs;
    return lhs;
  }

  friend String operator+(String lhs, const char* rhs) {
    lhs += rhs;
    return lhs;
  }

  friend String operator+(const char* lhs, const String& rhs) {
    String value(lhs);
    value += rhs;
    return value;
  }

  friend String operator+(String lhs, char rhs) {
    lhs += rhs;
    return lhs;
  }

  template <typename T,
            typename = std::enable_if_t<std::is_arithmetic_v<T> &&
                                        !std::is_same_v<std::decay_t<T>, char> &&
                                        !std::is_same_v<std::decay_t<T>, bool>>>
  friend String operator+(String lhs, T rhs) {
    lhs += rhs;
    return lhs;
  }

  std::string std() const {
    return value_;
  }

 private:
  std::string value_;
};
