/*==================================================================================================================

    Copyright (c) 2010 - 2012 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

/// <summary>
/// Class for handling dynamic types
/// </summary>
/// <remarks>
/// This is a class for holding objects of any type. One can also easily convert between different "basic" types. By
/// using the Array and Hash, this class may be used to create a "property list" that holds keys with values
/// of different types in a tree-like structure. It also contains methods for converting to-and-from JSON.
///
/// Maintainers: Jonathan
/// </remarks>

#if !defined(__Value_h__)
#define __Value_h__

#include <boost/any.hpp>

#include <map>
#include <string>
#include <vector>
#include <istream>
#include <ostream>
#include <sstream>
#include <climits>
#include EXCEPTION_PTR_HEADER

class Value {
  public:
    typedef std::vector<Value> Array;
    typedef std::map<std::string, Value> Hash;

    Value() : m_value(static_cast<void*>(0)) {} // Null
    Value(char* value) : m_value(std::string(value != 0 ? value : "")) {}
    Value(const char* value) : m_value(std::string(value != 0 ? value : "")) {}
    Value(const std::wstring& value) : m_value(convertWideStringToUTF8String(value)) {}
    Value(int value) : m_value(value) {}
    Value(unsigned int value) : m_value(value) {}
    Value(long long value) : m_value(value) {}
    Value(unsigned long long value) : m_value(value) {}
    Value(double value) : m_value(value) {}
    Value(long double value) : m_value(value) {}

    Value(float value) : m_value(static_cast<double>(value)) {}
    Value(short value) : m_value(static_cast<int>(value)) {}
    Value(unsigned short value) : m_value(static_cast<unsigned int>(value)) {}
#if (INT_MAX == LONG_MAX)
    Value(long value) : m_value(static_cast<int>(value)) {}
    Value(unsigned long value) : m_value(static_cast<unsigned int>(value)) {}
#else
    Value(long value) : m_value(static_cast<long long>(value)) {}
    Value(unsigned long value) : m_value(static_cast<unsigned long long>(value)) {}
#endif
    template<typename T> Value(const T& value) : m_value(value) {}

    template<typename T> inline bool Is() const { return (m_value.type() == typeid(T)); }
    inline bool IsNull() const { return (Is<void*>() && Cast<void*>(0) == 0); }
    inline bool IsHash() const { return Is<Hash>(); }
    inline bool IsArray() const { return Is<Array>(); }
    inline bool IsString() const { return Is<std::string>(); }
    inline bool IsBool() const { return Is<bool>(); }
    inline bool IsNumeric() const {
       return (Is<int>() || Is<double>() || Is<long long>() ||
               Is<unsigned int>() || Is<unsigned long long>() || Is<long double>());
    }
    inline bool IsBasic() const { return (IsNumeric() || IsString() || IsBool()); }
    template<typename T> inline T& Cast() {
      T* casted = boost::any_cast<T>(&m_value);
      if (!casted) {
        throw_rethrowable std::exception();
      }
      return *casted;
    }
    template<typename T> inline const T& Cast(const T& defaultValue = T()) const {
      const T* casted = boost::any_cast<T>(&m_value);
      if (casted) { return *casted; }
      return defaultValue;
    }
    template<typename T> inline const T& ConstCast(const T& defaultValue = T()) const {
      return Cast<T>(defaultValue);
    }
    template<typename T> inline T To() const { return Cast<T>(); }
    std::string ToJSON(bool escapedSlashes = true, bool prettify = false) const;
    std::string ToMessagePack() const;

    template<typename T> inline operator T () const { return Cast<T>(); }
    inline operator short () const { return ToBasic<short>(); }
    inline operator unsigned short () const { return ToBasic<unsigned short>(); }
    inline operator int () const { return ToBasic<int>(); }
    inline operator unsigned int () const { return ToBasic<unsigned int>(); }
    inline operator long () const { return ToBasic<long>(); }
    inline operator unsigned long () const { return ToBasic<unsigned long>(); }
    inline operator long long () const { return ToBasic<long long>(); }
    inline operator unsigned long long () const { return ToBasic<unsigned long long>(); }
    inline operator float () const { return ToBasic<float>(); }
    inline operator double () const { return ToBasic<double>(); }
    inline operator long double () const { return ToBasic<long double>(); }
    inline operator bool () const;
    inline operator std::string () const;
    inline operator std::wstring () const;

    inline bool operator==(const Value& rhs) const { return (ToJSON() == rhs.ToJSON()); }
    inline bool operator!=(const Value& rhs) const { return (ToJSON() != rhs.ToJSON()); }
    inline Value operator[](const std::string& key) const { return HashGet(key); }
    inline Value operator[](const char* key) const { return HashGet(key); }

    static Value FromJSON(const std::string& json) {
      return fromString<Value>(json);
    }

    bool HashHas(const std::string& key) const {
      const Hash* hash = boost::any_cast<Hash>(&m_value);
      return (hash && hash->find(key) != hash->end());
    }

    template<typename T>
    bool HashHasType(const std::string& key) const {
      const Hash* hash = boost::any_cast<Hash>(&m_value);
      if ( hash ) {
        Hash::const_iterator i = hash->find(key);
        return i != hash->end() && i->second.Is<T>();
      }

      return false;
    }

    Value HashGet(const std::string& key) const {
      const Hash* hash = boost::any_cast<Hash>(&m_value);
      if (hash) {
        Hash::const_iterator iter = hash->find(key);
        if (iter != hash->end()) {
          return iter->second;
        }
      }
      return Value();
    }
    bool HashSet(const std::string& key, const Value& value) {
      Hash* hash = boost::any_cast<Hash>(&m_value);
      if (hash) {
        (*hash)[key] = value;
        return true;
      }
      return false;
    }

    friend std::ostream& operator<< (std::ostream& out, const Value& value);
    friend std::istream& operator>> (std::istream& in, Value& value);

    static std::string convertWideStringToUTF8String(const std::wstring& wide);
    static std::wstring convertUTF8StringToWideString(const std::string& utf8);

  private:
    template<typename T> T ToBasic() const {
      if (Is<int>()) { return static_cast<T>(Cast<int>()); }
      if (Is<double>()) { return static_cast<T>(Cast<double>()); }
      if (Is<std::string>()) { return fromString<T>(Cast<std::string>()); }
      if (Is<bool>()) { return Cast<bool>(); }
      if (Is<long long>()) { return static_cast<T>(Cast<long long>()); }
      if (Is<unsigned int>()) { return static_cast<T>(Cast<unsigned int>()); }
      if (Is<unsigned long long>()) { return static_cast<T>(Cast<unsigned long long>()); }
      if (Is<long double>()) { return static_cast<T>(Cast<long double>()); }
      return T();
    }
    bool toBinaryStream(std::ostream& stream) const;
    bool toStream(std::ostream& stream, bool asJSON = false, bool escapeSlashes = true, int indent = -1) const;
    static Value parseValue(std::istream& stream);
    static Value parseObject(std::istream& stream);
    static Value parseArray(std::istream& stream);
    static Value parseString(std::istream& stream);
    static Value parseNumber(std::istream& stream);
    static Value parseTrue(std::istream& stream);
    static Value parseFalse(std::istream& stream);
    static Value parseNull(std::istream& stream);
    static void skipWhitespace(std::istream& stream);
    static inline char getChar(std::istream& stream) {
      char c;
      if (!stream.get(c).good()) {
        throw_rethrowable std::exception();
      }
      return c;
    }
    static inline char peekChar(std::istream& stream, bool optional = false) {
      try {
        int c = stream.peek();
        if (stream.good()) { return static_cast<char>(c); }
      } catch (...) {}
      if (!optional) {
        throw_rethrowable std::exception();
      }
      return '\0';
    }
    static inline void requireChar(std::istream& stream, char required) {
      if (peekChar(stream) == required) {
        stream.get();
      } else {
        throw_rethrowable std::exception();
      }
    }
    static inline char getDigit(std::istream& stream) {
      char c = peekChar(stream);
      if (c < '0' || c > '9') {
        throw_rethrowable std::exception();
      }
      getChar(stream);
      return c;
    }
    static inline bool getIfDigit(std::istream& stream, char& digit) {
      char c = peekChar(stream, true);
      if (c >= '0' && c <= '9') {
        getChar(stream);
        digit = c;
        return true;
      }
      return false;
    }
    static inline bool getIfHexDigitAsNibble(std::istream& stream, unsigned int& nibble) {
      char c = peekChar(stream, true);
      if (c >= '0' && c <= '9') {
        getChar(stream);
        nibble = c - '0';
      } else if (c >= 'A' && c <= 'F') {
        getChar(stream);
        nibble = c - 'A' + 10;
      } else if (c >= 'a' && c <= 'f') {
        getChar(stream);
        nibble = c - 'a' + 10;
      } else {
        return false;
      }
      return true;
    }
    static inline unsigned int getUnicode(std::istream& stream) {
      unsigned int u0 = 0, u1 = 0, u2 = 0, u3 = 0;

      if (!getIfHexDigitAsNibble(stream, u0) ||
          !getIfHexDigitAsNibble(stream, u1) ||
          !getIfHexDigitAsNibble(stream, u2) ||
          !getIfHexDigitAsNibble(stream, u3)) {
        throw_rethrowable std::exception();
      }
      return (u0 << 12) | (u1 << 8) | (u2 << 4) | u3;
    }

    template<typename T> static inline T fromString(const std::string& value) {
      std::istringstream iss(value);
      T t = T();
      iss >> t;
      return t;
    }

    boost::any m_value;
};

template<> inline Value Value::To() const { return *this; }
template<> inline short Value::To() const { return ToBasic<short>(); }
template<> inline unsigned short Value::To() const { return ToBasic<unsigned short>(); }
template<> inline int Value::To() const { return ToBasic<int>(); }
template<> inline unsigned int Value::To() const { return ToBasic<unsigned int>(); }
template<> inline long Value::To() const { return ToBasic<long>(); }
template<> inline unsigned long Value::To() const { return ToBasic<unsigned long>(); }
template<> inline long long Value::To() const { return ToBasic<long long>(); }
template<> inline unsigned long long Value::To() const { return ToBasic<unsigned long long>(); }
template<> inline float Value::To() const { return ToBasic<float>(); }
template<> inline double Value::To() const { return ToBasic<double>(); }
template<> inline long double Value::To() const { return ToBasic<long double>(); }
template<> inline bool Value::To() const {
  if (Is<bool>()) { return Cast<bool>(); }
  if (Is<int>()) { return (Cast<int>() != 0); }
  if (Is<double>()) { return (Cast<double>() != 0.0); }
  if (Is<std::string>()) {
    const std::string& value = Cast<std::string>();
    return (!value.empty() && value != "0" && value != "false");
  }
  if (Is<long long>()) { return (Cast<long long>() != 0); }
  if (Is<unsigned int>()) { return (Cast<unsigned int>() != 0); }
  if (Is<unsigned long long>()) { return (Cast<unsigned long long>() != 0); }
  if (Is<long double>()) { return (Cast<long double>() != 0); }
  if (IsNull()) { return false; }
  return true;
}
template<> inline std::string Value::To() const {
  if (Is<std::string>()) {
    return Cast<std::string>();
  } else {
    std::ostringstream oss;
    if (toStream(oss, false)) { return oss.str(); }
  }
  return std::string();
}
template<> inline std::wstring Value::To() const {
  std::string utf8(To<std::string>());
  return convertUTF8StringToWideString(utf8);
}

inline Value::operator bool () const { return To<bool>(); }
inline Value::operator std::string () const { return To<std::string>(); }
inline Value::operator std::wstring () const { return To<std::wstring>(); }

#endif // __Value_h__
