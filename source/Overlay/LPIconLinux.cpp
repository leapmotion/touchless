#include "stdafx.h"
#include "LPIconLinux.h"

LPIconLinux::LPIconLinux(void) :
m_visible(false),
m_position(LPPointMake(0,0))
{
}

LPIconLinux::~LPIconLinux(void)
{
}

bool LPIconLinux::Update() {
  return false; // LPIcon objects are rendeerd using LPOverlay
}

void LPIconLinux::SetPosition(const LPPoint& position) {
  m_position = position;
}

void LPIconLinux::SetVisibility(bool isVisible) {
  m_visible = isVisible;
}

bool LPIconLinux::GetVisibility() const {
  return m_visible;
}

LPPoint LPIconLinux::GetPosition(void) const {
  return m_position;
}

LPIcon* LPIcon::New(void) {
  return new LPIconLinux;
}