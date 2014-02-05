#include "stdafx.h"
#include "LPIconMac.h"

LPIconMac::LPIconMac(void) :
m_visible(false),
m_position(LPPointMake(0,0))
{
}

LPIconMac::~LPIconMac(void)
{
}

bool LPIconMac::Update() {
  return false; // LPIcon objects are rendeerd using LPOverlay
}

void LPIconMac::SetPosition(const LPPoint& position) {
  m_position = position;
}

void LPIconMac::SetVisibility(bool isVisible) {
  m_visible = isVisible;
}

bool LPIconMac::GetVisibility() const {
  return m_visible;
}

LPPoint LPIconMac::GetPosition(void) const {
  return m_position;
}

LPIcon* LPIcon::New(void) {
  return new LPIconMac;
}