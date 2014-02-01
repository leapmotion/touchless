/******************************************************************************\
* Copyright (C) 2012-2013 Leap Motion, Inc. All rights reserved.               *
* Leap Motion proprietary and confidential. Not for distribution.              *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement         *
* between Leap Motion and you, your company or other organization.             *
\******************************************************************************/

#if !defined(__LeapPlugin_h__)
#define __LeapPlugin_h__

#include "Leap.h"

namespace Leap {

//
// Public Interface
//

/**
 * Subclass the Plugin class to add Leap support to existing application.
 *
 * Create a shared object library (e.g., DLL) that contains an "init"
 * C-linkable named function, and returns a pointer to an instance of your
 * inherited Leap::Plugin class. If memory is allocated using "init", a
 * corresponding "destroy" C-linkable named function must be included to
 * release the memory allocated in the "init" function.
 *
 * Here is an example of how to dynamically allocate the memory for the
 * instance. In this case, the memory should be freed as part of the "destroy"
 * function.
 *
 * extern "C" LEAP_EXPORT_PLUGIN Leap::Plugin* init()
 * {
 *   return new MyApplicationPlugin;
 * }
 *
 * extern "C" LEAP_EXPORT_PLUGIN void destroy(Leap::Plugin* plugin)
 * {
 *   delete plugin;
 * }
 *
 * An alternate approach is to return a static instance of the inherited class.
 * In this case, there is no need to include a "destroy" function. However, you
 * should ONLY use this approach if you do NOT have per-plugin configuration
 * information. Failure to follow this rule will result in all plugins that use
 * the module sharing the same configuration information.
 *
 * extern "C" LEAP_EXPORT_PLUGIN Leap::Plugin* init()
 * {
 *   static MyApplicationPlugin s_myApplicationPlugin;
 *
 *   return &s_myApplicationPlugin;
 * }
 *
 * Do not attempt to access the Config information in your sub-classed Plugin
 * constructor. References to that class will not be available until the
 * onInit method.
 * @since 1.0
     */
class LEAP_EXPORT_CLASS Plugin : public Interface {
  public:
    /**
     * Constructs a Plugin object.
     * @since 1.0
     */
    LEAP_EXPORT Plugin();

    /**
     * Destroys this Plugin object.
     * @since 1.0
     */
    LEAP_EXPORT virtual ~Plugin();

    /**
     * Called when the Plugin is initialized, but before the plugin's
     * Controller object is connected to the Leap.
     *
     * @param leap The Controller object for this Plugin.
     * @since 1.0
     */
    LEAP_EXPORT virtual void onInit(const Controller& leap);

    /**
     * Called when a frame of tracking data is available.
     * @since 1.0
     */
    LEAP_EXPORT virtual void onFrame(const Controller& leap);

    /**
     * Called when the Plugin controller is connected to the Leap.
     *
     * @param leap The Controller object for this Plugin.
     * @since 1.0
     */
    LEAP_EXPORT virtual void onConnect(const Controller& leap);

    /**
     * Called when the Plugin controller disconnects from the Leap.
     *
     * @param leap The Controller object for this Plugin.
     * @since 1.0
     */
    LEAP_EXPORT virtual void onDisconnect(const Controller& leap);
    LEAP_EXPORT virtual void onExit(const Controller& leap);

    /**
     * Called when an application associated with this Plugin gains the
     * operating system focus.
     *
     * If two applications are associated with this plugin and the
     * focus changes from one to the other, first onFocusLost() is called and then
     * onFocusGained().
     *
     * @param leap The Controller object for this Plugin.
     * @since 1.0
     */
    LEAP_EXPORT virtual void onFocusGained(const Controller& leap);

    /**
     * Called when an application associated with this Plugin loses the
     * operating system focus.
     *
     * If two applications are associated with this plugin and the
     * focus changes from one to the other, first onFocusLost() is called and then
     * onFocusGained().
     *
     * @param leap The Controller object for this Plugin.
     * @since 1.0
     */
    LEAP_EXPORT virtual void onFocusLost(const Controller& leap);

    /**
     * The Config object for this Plugin.
     *
     * Changes to the configuration are saved permanently.
     * @since 1.0
     */
    LEAP_EXPORT Config& config() const;
};

}

#endif // __LeapPlugin_h__
