#ifndef _CUSTOM_TYPE_INDEX_H
#define _CUSTOM_TYPE_INDEX_H
#include TYPE_TRAITS_HEADER

namespace std {

class type_index {
public:
  type_index(const type_info& info):
    info(info)
  {
  }

  const type_info& info;

  bool operator==(type_index rhs) const {
    return info == rhs.info;
  }

  bool operator<(type_index rhs) const {
    return info.before(rhs.info);
  }
};

template<>
struct hash<type_index>:
  public unary_function<type_index, size_t>
{
  size_t operator()(type_index val) const {
    return (size_t)&val.info;
  }
};

};

#endif