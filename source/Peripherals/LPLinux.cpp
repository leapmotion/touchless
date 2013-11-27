/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#include "LPLinux.h"

//
// CFocusAppInfo
//

CFocusAppInfo::CFocusAppInfo()
{
  m_rcClient.left = m_rcClient.right = m_rcClient.top = m_rcClient.bottom = 0;
}

void CFocusAppInfo::Update()
{
}
