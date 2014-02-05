#pragma once
#include "Overlay/LPImage.h"

class LPImageWin:
  public LPImage
{
public:
  LPImageWin(void);
  ~LPImageWin(void);

private:
  // The DC representing this image
  HDC m_hDC;

public:
  // Accessor methods:
  HDC GetDC() const { return m_hDC; }

  /// <summary>
  /// Constructs a DC from the passed Gdi+ bitmap
  /// </summary>
  /// <param name="bitmap">The Gdi+ bitmap containing the desired image data</param>
  /// <param name="pHotspot">The hot spot for the image.  If left null, it will default to the geometric center.</param>
  bool SetImageFromBitmap(Gdiplus::Bitmap& src, const POINT* pHotspot = 0, bool write = false);

  /// <summary>
  /// Sets the DC used as a source for this image
  /// </summary>
  /// <param name="pHotspot">The hot spot for the image.  If left null, it will default to the geometric center.</param>
  /// <remarks>
  /// The passed DC must have been created with CreateCompatibleDC.  Once assigned, this class
  /// takes responsibility for the cleanup of the HDC and the HBITMAP attached to it
  /// </remarks>
  void SetDC(HDC hDc, const POINT* pHotspot = 0);

  // Base overrides:
  bool SetImage(const std::wstring& filename, const POINT* pHotspot) override;
  bool SetImage(const SIZE& size, const POINT* pHotspot) override;
  bool InitImage(long width, long height, const POINT* pHotspot) override;
};

