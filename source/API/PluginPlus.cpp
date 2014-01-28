/*==================================================================================================================

    Copyright (c) 2010 - 2012 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#include "PluginImplementation.h"

namespace Leap {

//
// PluginPlus
//

PluginPlus::PluginPlus(const Plugin& plugin) : Plugin(plugin) {}
PluginPlus::PluginPlus() {}
OutputPeripheral& PluginPlus::outputPeripheral() const { return get<PluginImplementation>()->outputPeripheral(); }

}
