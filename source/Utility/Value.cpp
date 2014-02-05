/*==================================================================================================================

    Copyright (c) 2010 - 2012 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/
#include "stdafx.h"

#if !defined(__STDC_LIMIT_MACROS)
  #define __STDC_LIMIT_MACROS
#endif
#include "Value.h"
#include <stdint.h>
#include <cmath>

#if _WIN32
#define isnan _isnan
#elseif __APPLE__
using std::isnan;
#endif

std::string Value::ToJSON(bool escapeSlashes, bool prettify) const
{
  std::ostringstream oss;

  toStream(oss, true, escapeSlashes, prettify ? 0 : 1);
  return oss.str();
}

// Indent the output if streaming JSON data and indent is set an even number.
// Pack the output otherwise.
bool Value::toStream(std::ostream& stream, bool asJSON, bool escapeSlashes, int indent) const
{
  if (asJSON && !(indent & 1)) {
    if (indent > 0) {
      stream << std::endl << std::string(indent, ' ');
    } else {
      indent = -indent;
    }
  }
  if (Is<std::string>()) {
    const std::string& str = Cast<std::string>();

    if (asJSON) {
      const char* ptr = str.c_str();

      stream << '"';
      while (*ptr != '\0') {
        switch (*ptr) {
          case '"':
            stream << "\\\"";
            break;
          case '\\':
            stream << "\\\\";
            break;
          case '/':
            if (escapeSlashes) {
              stream << "\\/";
            } else {
              stream << *ptr;
            }
            break;
          case '\b':
            stream << "\\b";
            break;
          case '\f':
            stream << "\\f";
            break;
          case '\n':
            stream << "\\n";
            break;
          case '\r':
            stream << "\\r";
            break;
          case '\t':
            stream << "\\t";
            break;
          default:
            stream << *ptr;
            break;
        }
        ptr++;
      }
      stream << '"';
    } else {
      stream << str;
    }
  } else if (Is<int>()) {
    stream << Cast<int>();
  } else if (Is<long long>()) {
    stream << Cast<long long>();
  } else if (Is<double>()) {
    const double value = Cast<double>();
    if (!isnan(value)) {
      stream << value;
    } else {
      stream << "null"; // NaN is not valid JSON, use null instead
    }
  } else if (Is<bool>()) {
    stream << (Cast<bool>() ? "true" : "false");
  } else if (IsNull()) {
    stream << "null";
  } else if (Is<unsigned int>()) {
    stream << Cast<unsigned int>();
  } else if (Is<unsigned long long>()) {
    stream << Cast<unsigned long long>();
  } else if (Is<long double>()) {
    stream << Cast<long double>();
  } else if (Is<Array>()) {
    const Array* array = boost::any_cast<Array>(&m_value);

    stream << '[';
    if (array) {
      const size_t n = array->size();

      for (size_t i = 0; i < n; i++) {
        if (i != 0) {
          stream << ',';
        }
        array->at(i).toStream(stream, asJSON, escapeSlashes, indent + 2);
      }
    }
    if (asJSON && !(indent & 1)) {
      stream << std::endl << std::string(indent, ' ');
    }
    stream << ']';
  } else if (Is<Hash>()) {
    const Hash* hash = boost::any_cast<Hash>(&m_value);

    stream << '{';
    if (hash) {
      for (Hash::const_iterator iter = hash->begin(); iter != hash->end(); ++iter) {
        if (iter != hash->begin()) {
          stream << ',';
        }
        if (asJSON) {
          Value(iter->first).toStream(stream, asJSON, escapeSlashes, indent + 2);
        } else {
          stream << iter->first;
        }
        stream << ':';
        if (asJSON && !(indent & 1)) {
          stream << ' ';
        }
        iter->second.toStream(stream, asJSON, escapeSlashes, -(indent + 2));
      }
    }
    if (asJSON && !(indent & 1) && indent >= 0) {
      stream << std::endl << std::string(indent, ' ');
    }
    stream << '}';
  } else {
    return false;
  }
  return true;
}

Value Value::parseValue(std::istream& stream)
{
  skipWhitespace(stream);

  switch (peekChar(stream)) {
    case '{':
      return parseObject(stream);
    case '[':
      return parseArray(stream);
    case 't':
      return parseTrue(stream);
    case 'f':
      return parseFalse(stream);
    case 'n':
      return parseNull(stream);
    case '"':
      return parseString(stream);
    default:
      return parseNumber(stream);
  }
}

Value Value::parseObject(std::istream& stream)
{
  Value::Hash hash;

  requireChar(stream, '{');
  for (;;) {
    skipWhitespace(stream);
    char c = peekChar(stream);
    if (c == ',') {
      if (hash.empty()) {
        throw_rethrowable std::exception();
      }
      getChar(stream);
    } else if (c == '}') {
      getChar(stream);
      return hash;
    } else {
      std::string key = parseString(stream).To<std::string>();
      skipWhitespace(stream);
      requireChar(stream, ':');
      hash[key] = parseValue(stream);
    }
  }
}

Value Value::parseArray(std::istream& stream)
{
  Value::Array array;

  requireChar(stream, '[');
  for (;;) {
    skipWhitespace(stream);
    char c = peekChar(stream);
    if (c == ',') {
      if (array.empty()) {
        throw_rethrowable std::exception();
      }
      getChar(stream);
    } else if (c == ']') {
      getChar(stream);
      return array;
    } else {
      array.push_back(parseValue(stream));
    }
  }
}

Value Value::parseString(std::istream& stream)
{
  std::string value;

  requireChar(stream, '"');
  for (;;) {
    char c = getChar(stream);
    if (c == '"') {
      return value;
    } else if (c == '\\') {
      c = getChar(stream);
      switch (c) {
        case '"':
        case '\\':
        case '/': break;
        case 'b': c = '\b'; break;
        case 'f': c = '\f'; break;
        case 'n': c = '\n'; break;
        case 'r': c = '\r'; break;
        case 't': c = '\t'; break;
        case 'u':
          {
            unsigned int unicode = getUnicode(stream);
            if (unicode >= 0xD800 && unicode <= 0xDBFF) {
              // Surrogate pairs
              requireChar(stream, '\\');
              requireChar(stream, 'u');
              unsigned int pair = getUnicode(stream);
              if (pair < 0xDC00 || pair > 0xDFFF) {
                throw_rethrowable std::exception();
              }
              // Combine the surrogate pairs
              unicode = 0x10000 + ((unicode - 0xD800) << 10) + (pair - 0xDC00);
            }
            // Convert Unicode to UTF-8
            if (unicode <= 0x7F) {
              c      = static_cast<char>(unicode);
            } else if (unicode <= 0x7FF) {
              value += static_cast<char>(0xC0 | (unicode >> 6));
              c      = static_cast<char>(0x80 | (unicode & 0x3F));
            } else if (unicode <= 0xFFFF) {
              value += static_cast<char>(0xE0 |  (unicode >> 12));
              value += static_cast<char>(0x80 | ((unicode >>  6) & 0x3F));
              c      = static_cast<char>(0x80 |  (unicode        & 0x3F));
            } else if (unicode <= 0x10FFFF) {
              value += static_cast<char>(0xF0 |  (unicode >> 18));
              value += static_cast<char>(0x80 | ((unicode >> 12) & 0x3F));
              value += static_cast<char>(0x80 | ((unicode >>  6) & 0x3F));
              c      = static_cast<char>(0x80 |  (unicode        & 0x3F));
            } else {
              throw_rethrowable std::exception();
            }
          }
          break;
        default:
          throw_rethrowable std::exception();
      }
    }
    value += c;
  }
}

Value Value::parseNumber(std::istream& stream)
{
  std::string number;
  char c = peekChar(stream);
  bool real = false;
  bool positive = true;

  if (c == '-') {
    positive = false;
    number += c;
    getChar(stream);
  }
  c = getDigit(stream);
  number += c;
  if (c != '0') {
    while (getIfDigit(stream, c)) {
      number += c;
    }
  }
  c = peekChar(stream, true);
  if (c == '.') {
    real = true;
    number += c;
    getChar(stream);
    number += getDigit(stream);
    while (getIfDigit(stream, c)) {
      number += c;
    }
    c = peekChar(stream, true);
  }
  if (c == 'E' || c == 'e') {
    real = true;
    number += c;
    getChar(stream);
    c = peekChar(stream);
    if (c == '+' || c == '-') {
      number += c;
      getChar(stream);
    }
    number += getDigit(stream);
    while (getIfDigit(stream, c)) {
      number += c;
    }
  }
  std::istringstream iss(number);
  if (real) {
    double value;
    iss >> value;
    return value;
  } else if (positive) {
    unsigned long long value;
    iss >> value;
    if (value <= UINT_MAX) {
      if (value <= INT_MAX) {
        return static_cast<int>(value);
      }
      return static_cast<unsigned int>(value);
    }
    return value;
  } else {
    long long value;
    iss >> value;
    if (value >= INT_MIN && value <= INT_MAX) {
      return static_cast<int>(value);
    }
    return value;
  }
}

Value Value::parseTrue(std::istream& stream)
{
  requireChar(stream, 't');
  requireChar(stream, 'r');
  requireChar(stream, 'u');
  requireChar(stream, 'e');
  return true;
}

Value Value::parseFalse(std::istream& stream)
{
  requireChar(stream, 'f');
  requireChar(stream, 'a');
  requireChar(stream, 'l');
  requireChar(stream, 's');
  requireChar(stream, 'e');
  return false;
}

Value Value::parseNull(std::istream& stream)
{
  requireChar(stream, 'n');
  requireChar(stream, 'u');
  requireChar(stream, 'l');
  requireChar(stream, 'l');
  return Value();
}

void Value::skipWhitespace(std::istream& stream)
{
  for (;;) {
    char c = peekChar(stream);
    if (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\f') {
      getChar(stream);
    } else {
      return;
    }
  }
}

std::string Value::convertWideStringToUTF8String(const std::wstring& wide)
{
  std::string utf8;
  size_t wide_size = wide.size();
  const wchar_t* wide_cstr = wide.c_str();

  try {
    for (size_t i = 0; i < wide_size; i++) {
      unsigned int unicode = static_cast<unsigned int>(*wide_cstr++);

      if (unicode <=  0x7F) {
        utf8 += static_cast<char>(unicode);
      } else if (unicode <= 0x7FF) {
        utf8 += static_cast<char>(0xC0 | (unicode >> 6));
        utf8 += static_cast<char>(0x80 | (unicode & 0x3F));
      } else if (unicode >= 0xD800 && unicode <= 0xDBFF) {
        if (++i >= wide_size) { // Surrogate pairs come in pairs, it must be there
          throw_rethrowable std::exception();
        }
        unsigned int pair = static_cast<unsigned int>(*wide_cstr++);
        if (pair < 0xDC00 || pair > 0xDFFF) {
          throw_rethrowable std::exception(); // And the pair must fall within the surrogate pair range
        }
        // Combine the surrogate pairs
        unicode = 0x10000 + ((unicode - 0xD800) << 10) + (pair - 0xDC00);
        utf8 += static_cast<char>(0xF0 |  (unicode >> 18));
        utf8 += static_cast<char>(0x80 | ((unicode >> 12) & 0x3F));
        utf8 += static_cast<char>(0x80 | ((unicode >>  6) & 0x3F));
        utf8 += static_cast<char>(0x80 |  (unicode        & 0x3F));
      } else if (unicode <= 0xFFFF) {
        utf8 += static_cast<char>(0xE0 |  (unicode >> 12));
        utf8 += static_cast<char>(0x80 | ((unicode >>  6) & 0x3F));
        utf8 += static_cast<char>(0x80 |  (unicode        & 0x3F));
      } else if (unicode <= 0x10FFFF) {
        utf8 += static_cast<char>(0xF0 |  (unicode >> 18));
        utf8 += static_cast<char>(0x80 | ((unicode >> 12) & 0x3F));
        utf8 += static_cast<char>(0x80 | ((unicode >>  6) & 0x3F));
        utf8 += static_cast<char>(0x80 |  (unicode        & 0x3F));
      } else {
        throw_rethrowable std::exception();
      }
    }
  } catch (...) {}
  return utf8;
}

std::wstring Value::convertUTF8StringToWideString(const std::string& utf8)
{
  std::wstring wide;
  size_t utf8_size = utf8.size();
  const unsigned char* utf8_cstr = reinterpret_cast<const unsigned char*>(utf8.c_str());
  unsigned int unicode = 0;
  int remainingBytes = 0;

  try {
    for (size_t i = 0; i < utf8_size; i++) {
      unsigned int c = static_cast<unsigned int>(*utf8_cstr++);

      if (c <= 0x7F) {        // 01111111
        if (remainingBytes) {
          throw_rethrowable std::exception(); // Invalid UTF-8 String
        }
        unicode = c;
      } else if (c <= 0xBF) { // 10111111
        if (remainingBytes) {
          unicode = (unicode << 6) | (c & 0x3F);
          remainingBytes--;
        } else {
          throw_rethrowable std::exception(); // Invalid UTF-8 String
        }
      } else if (c <= 0xDF) { // 11011111
        unicode = c & 0x1F;
        remainingBytes = 1;
      } else if (c <= 0xEF) { // 11101111
        unicode = c & 0x0F;
        remainingBytes = 2;
      } else if (c <= 0xF7) { // 11110111
        unicode = c & 0x07;
        remainingBytes = 3;
      } else {
        throw_rethrowable std::exception(); // Invalid UTF-8 String
      }
      if (!remainingBytes) {
        if (sizeof(wchar_t) == 2 && unicode > 0xFFFF) {
          wchar_t high = 0xD800 | (((unicode - 0x10000) >> 10) & 0x3FF);
          wide += high;
          unicode = 0xDC00 | (unicode & 0x3FF);
        } else if (unicode > 0x10FFFF) {
          throw_rethrowable std::exception();
        }
        wide += static_cast<wchar_t>(unicode);
      }
    }
  } catch (...) {}
  return wide;
}

std::ostream& operator<< (std::ostream& out, const Value& value)
{
  value.toStream(out, true, true, -1);
  return out;
}

std::istream& operator>> (std::istream& in, Value& value)
{
  try {
    value = Value::parseValue(in);
  } catch (...) {}
  return in;
}

std::string Value::ToMessagePack() const
{
  std::ostringstream oss;

  toBinaryStream(oss);
  return oss.str();
}

template<typename T>
union PackedData {
  private:
    T m_data;
    uint8_t m_bytes[sizeof(T)];

    static bool isLittleEndian() {
      const int endianess = 1;
      return (*reinterpret_cast<const char*>(&endianess) == 1);
    }

  public:
    PackedData(const T& data) : m_data(data) {
      static bool s_isLittleEndian = isLittleEndian();

      if (s_isLittleEndian) {
        const size_t half = sizeof(T)/2;
        for (size_t i = 0; i < half; i++) {
          uint8_t tmp = m_bytes[i];
          m_bytes[i] = m_bytes[sizeof(T) - 1 - i];
          m_bytes[sizeof(T) - 1 - i] = tmp;
        }
      }
    }
    inline int size() const { return static_cast<int>(sizeof(T)); }
    inline uint8_t operator[](int index) const { return m_bytes[index]; }
    inline const char* c_str() const { return reinterpret_cast<const char*>(m_bytes); }
};

bool Value::toBinaryStream(std::ostream& stream) const
{
  if (Is<std::string>()) {
    const std::string& str = Cast<std::string>();
    const size_t length = str.size();

    if (length <= 31) {
      // fix raw
      stream.put(static_cast<uint8_t>(0xA0) | static_cast<uint8_t>(length));
    } else if (length <= UINT16_MAX) {
      // raw 16
      PackedData<uint16_t> packedLength(static_cast<uint16_t>(length));
      stream.put(static_cast<uint8_t>(0xDA)).write(packedLength.c_str(), packedLength.size());
    } else if (length <= UINT32_MAX) {
      // raw 32
      PackedData<uint32_t> packedLength(static_cast<uint32_t>(length));
      stream.put(static_cast<uint8_t>(0xDB)).write(packedLength.c_str(), packedLength.size());
    } else {
      throw_rethrowable std::exception();
    }
    stream.write(str.c_str(), str.size());
  } else if (Is<int>()) {
    const int value = Cast<int>();
    if ((value >= 0 && value <= 127) || (value >= -32 && value <= -1)) {
      // positive fixnum | negative fixnum
      stream.put(static_cast<uint8_t>(value));
    } else if (value >= INT8_MIN && value <= INT8_MAX) {
      // int 8
      stream.put(static_cast<uint8_t>(0xD0)).put(value);
    } else if (value >= INT16_MIN && value <= INT16_MAX) {
      // int 16
      PackedData<int16_t> data(static_cast<int16_t>(value));
      stream.put(static_cast<uint8_t>(0xD1)).write(data.c_str(), data.size());
    } else if (value >= INT32_MIN && value <= INT32_MAX) {
      // int 32
      PackedData<int32_t> data(static_cast<int32_t>(value));
      stream.put(static_cast<uint8_t>(0xD2)).write(data.c_str(), data.size());
    } else {
      throw_rethrowable std::exception();
    }
  } else if (Is<long long>()) {
    const long long value = Cast<long long>();
    if ((value >= 0 && value <= 127) || (value >= -32 && value <= -1)) {
      // positive fixnum | negative fixnum
      stream.put(static_cast<uint8_t>(value));
    } else if (value >= INT8_MIN && value <= INT8_MAX) { // Will only match for -128 to -33
      // int 8
      stream.put(static_cast<uint8_t>(0xD0)).put(static_cast<uint8_t>(value));
    } else if (value >= INT16_MIN && value <= INT16_MAX) {
      // int 16
      PackedData<int16_t> data(static_cast<int16_t>(value));
      stream.put(static_cast<uint8_t>(0xD1)).write(data.c_str(), data.size());
    } else if (value >= INT32_MIN && value <= INT32_MAX) {
      // int 32
      PackedData<int32_t> data(static_cast<int32_t>(value));
      stream.put(static_cast<uint8_t>(0xD2)).write(data.c_str(), data.size());
    } else {
      // int 64
      PackedData<int64_t> data(static_cast<int64_t>(value));
      stream.put(static_cast<uint8_t>(0xD3)).write(data.c_str(), data.size());
    }
  } else if (Is<double>()) {
    // double
    PackedData<double> data(Cast<double>());
    stream.put(static_cast<uint8_t>(0xCB)).write(data.c_str(), data.size());
  } else if (Is<bool>()) {
    // true | false
    stream.put((Cast<bool>() ? static_cast<uint8_t>(0xC3) : static_cast<uint8_t>(0xC2)));
  } else if (IsNull()) {
    // nil
    stream.put(static_cast<uint8_t>(0xC0));
  } else if (Is<unsigned int>()) {
    const unsigned int value = Cast<unsigned int>();
    if (value <= UINT8_MAX) {
      // uint 8
      stream.put(static_cast<uint8_t>(0xCC)).put(static_cast<uint8_t>(value));
    } else if (value <= UINT16_MAX) {
      // uint 16
      PackedData<uint16_t> data(static_cast<uint16_t>(value));
      stream.put(static_cast<uint8_t>(0xCD)).write(data.c_str(), data.size());
    } else if (value <= UINT32_MAX) {
      // uint 32
      PackedData<uint32_t> data(static_cast<uint32_t>(value));
      stream.put(static_cast<uint8_t>(0xCE)).write(data.c_str(), data.size());
    } else {
      throw_rethrowable std::exception();
    }
  } else if (Is<unsigned long long>()) {
    const unsigned long long value = Cast<unsigned long long>();
    if (value <= UINT8_MAX) {
      // uint 8
      stream.put(static_cast<uint8_t>(0xCC)).put(static_cast<uint8_t>(value));
    } else if (value <= UINT16_MAX) {
      // uint 16
      PackedData<uint16_t> data(static_cast<uint16_t>(value));
      stream.put(static_cast<uint8_t>(0xCD)).write(data.c_str(), data.size());
    } else if (value <= UINT32_MAX) {
      // uint 32
      PackedData<uint32_t> data(static_cast<uint32_t>(value));
      stream.put(static_cast<uint8_t>(0xCE)).write(data.c_str(), data.size());
    } else {
      // uint 64
      PackedData<uint64_t> data(static_cast<uint64_t>(value));
      stream.put(static_cast<uint8_t>(0xCF)).write(data.c_str(), data.size());
    }
  } else if (Is<long double>()) {
    // double
    PackedData<double> data(static_cast<double>(Cast<long double>()));
    stream.put(static_cast<uint8_t>(0xCB)).write(data.c_str(), data.size());
  } else if (Is<Array>()) {
    const Array* array = boost::any_cast<Array>(&m_value);
    const size_t length = array ? array->size() : 0;

    if (length <= 15) {
      // fix array
      stream.put(static_cast<uint8_t>(0x90) | static_cast<uint8_t>(length));
    } else if (length <= UINT16_MAX) {
      // array 16
      PackedData<uint16_t> packedLength(static_cast<uint16_t>(length));
      stream.put(static_cast<uint8_t>(0xDC)).write(packedLength.c_str(), packedLength.size());
    } else if (length <= UINT32_MAX) {
      // array 32
      PackedData<uint32_t> packedLength(static_cast<uint32_t>(length));
      stream.put(static_cast<uint8_t>(0xDD)).write(packedLength.c_str(), packedLength.size());
    } else {
      throw_rethrowable std::exception();
    }
    for (size_t i = 0; i < length; i++) {
      array->at(i).toBinaryStream(stream);
    }
  } else if (Is<Hash>()) {
    const Hash* hash = boost::any_cast<Hash>(&m_value);

    if (hash) {
      const size_t length = hash->size();

      if (length <= 15) {
        // fix map
        stream.put(static_cast<uint8_t>(0x80) | static_cast<uint8_t>(length));
      } else if (length <= UINT16_MAX) {
        // map 16
        PackedData<uint16_t> packedLength(static_cast<uint16_t>(length));
        stream.put(static_cast<uint8_t>(0xDE)).write(packedLength.c_str(), packedLength.size());
      } else if (length <= UINT32_MAX) {
        // map 32
        PackedData<uint32_t> packedLength(static_cast<uint32_t>(length));
        stream.put(static_cast<uint8_t>(0xDF)).write(packedLength.c_str(), packedLength.size());
      } else {
        throw_rethrowable std::exception();
      }
      for (Hash::const_iterator iter = hash->begin(); iter != hash->end(); ++iter) {
        Value(iter->first).toBinaryStream(stream);
        iter->second.toBinaryStream(stream);
      }
    } else {
      stream.put(static_cast<uint8_t>(0x80)); // Zero-length map
    }
  } else {
    return false;
  }
  return true;
}
