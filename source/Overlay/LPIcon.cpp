#include "stdafx.h"
#include "Overlay/LPIcon.h"
#include "Overlay/LPImage.h"

LPIcon::LPIcon() {
}

LPIcon::~LPIcon() {
}

void LPIcon::SetImage(const std::shared_ptr<LPImage>& image, bool update) {
  // Trivial return check:
  if(m_image == image)
    return;

  m_image = image;
  if (update) {
    Update();
  }
}
