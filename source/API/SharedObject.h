/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#if !defined(__SharedObject_h__)
#define __SharedObject_h__

#include "Leap.h"
#include <boost/thread.hpp>
#include SHARED_PTR_HEADER

namespace Leap {

#define CAST_SHARED_PTR_AS_POINTER(T) ((void*)(&(const std::shared_ptr<Interface::Implementation>&)(T)))
#define CREATE_SHARED_PTR_POINTER(T) (CAST_SHARED_PTR_AS_POINTER(std::shared_ptr<Interface::Implementation>(T)))

class SharedObject {
public:
  SharedObject(const std::shared_ptr<Interface::Implementation>& owner):
    m_reference(owner.get()),
    m_owner(owner)
  {}

  SharedObject(Interface::Implementation* reference, const std::shared_ptr<Interface::Implementation>& owner):
    m_reference(reference),
    m_owner(owner)
  {}

  ~SharedObject() {}

  std::shared_ptr<Interface::Implementation> owner() const { return m_owner; }

private:
  std::shared_ptr<Interface::Implementation> m_owner;
  Interface::Implementation* m_reference;

  friend class Interface;
};

}

#endif // __SharedObject_h__
