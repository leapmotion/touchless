#pragma once

/// <summary>
/// Simple RAII-style class which initializes GDI+ on construction and deinitializes on teardown
/// </summary>
class GdiPlusInitializer
{
public:
  GdiPlusInitializer(void);
  ~GdiPlusInitializer(void);

private:
  ULONG_PTR m_gdiplusToken;
};

