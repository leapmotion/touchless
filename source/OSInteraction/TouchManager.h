#if !defined(_TouchManager_h_)
#define _TouchManager_h_
#if _WIN32
#include "GdiPlusInitializer.h"
#endif
#include "Utility/LPVirtualScreen.h"
#include "Touch.h"
#include <set>

using Touchless::Touch;

class TouchManager {
public:
  TouchManager(void);
  virtual ~TouchManager(void);

  /// <summary>
  /// Creates a new platform-specific touch manager implementation
  /// </summary>
  static TouchManager* New(void);

protected:
  std::set<Touch> m_touches;

#if _WIN32
  // GDI+:
  GdiPlusInitializer m_initializer;
#endif

  // Wired in because we need virtual coordinates for the driver:
  LPVirtualScreen m_virtualScreen;

  /// <summary>
  /// Service routine, used to print debug information about touch updates
  /// </summary>
  void DebugTouchInformation(const std::set<Touch>& touches);

  /// <summary>
  /// Adds a new active touch point
  /// </summary>
  virtual void AddTouch(const Touch& touch) = 0;

  /// <summary>
  /// Updates an existing touch point
  /// </summary>
  /// <param name="oldTouch">The prior touch state</param>
  /// <param name="newTouch">The current (updated) touch state</param>
  virtual void UpdateTouch(const Touch& oldTouch, const Touch& newTouch) = 0;

  /// <summary>
  /// Removes a previously valid touch point
  /// </summary>
  /// <param name="oldTouch">The last valid value of that touch point</param>
  virtual void RemoveTouch(const Touch& oldTouch) = 0;

  /// <summary>
  /// Event fired when all touches have just been removed
  /// </summary>
  virtual void OnRemoveAllTouches(void) {}

public:
  // Accessor methods:
  virtual int Version() = 0;
  const std::set<Touch>& getTouches() const { return m_touches; }
  virtual size_t numTouchScreens(void) const;

  // Mutator methods:
  void clearTouches();

  /// <summary>
  /// Updates the collection of touches managed by this touch manager
  /// </summary>
  /// <remarks>
  /// This causes the touch manager to consider the passed collection of touches.  The primary concern is
  /// in detecting new touches in the passed set, existing touches which have been changed, and former
  /// touches which are not currently present.
  ///
  /// Adjustments to this set are emitted to concrete classes in the form of internal virtual members.
  /// </remarks>
  void setTouches(std::set<Touch>&& touches);
};

#endif // _TouchManager_h_
