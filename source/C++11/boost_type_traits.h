// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_abstract.hpp>
#include <boost/type_traits/is_polymorphic.hpp>
#include <boost/type_traits/has_trivial_constructor.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/decay.hpp>

namespace std {
  using boost::is_base_of;
  using boost::is_same;
  using boost::is_abstract;
  using boost::has_trivial_constructor;
  using boost::is_polymorphic;
  using boost::remove_reference;
  using boost::decay;
}
