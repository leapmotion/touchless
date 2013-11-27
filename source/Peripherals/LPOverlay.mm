/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/
#include "stdafx.h"
#include "LPOverlay.h"

#include <CIContext.h>
#include <OpenGL/gl.h>

#include <Cocoa/Cocoa.h>
#include <NSColor.h>
#include <NSView.h>
#include <NSWindow.h>

#include "LPImage.h"

//
// LPOpenGLView
//

@interface LPOpenGLView : NSOpenGLView {
  std::shared_ptr<LPOverlayCache> m_overlayCache;
  boost::mutex m_mutex;
  bool m_isInitialized;
  bool m_isDirty;
  bool m_isShown;
}
-(id) initWithFrame:(NSRect)frame pixelFormat:(NSOpenGLPixelFormat*)format;
-(void) dealloc;
-(void) receiveSleepNotification:(NSNotification*)notification;
-(void) receiveWakeNotification:(NSNotification*)notification;
-(void) drawRect:(NSRect)bounds;
-(void) setOverlayCache:(std::shared_ptr<LPOverlayCache>)overlayCache;
-(BOOL) isOpaque;
@end

@implementation LPOpenGLView
-(id) initWithFrame:(NSRect)frame pixelFormat:(NSOpenGLPixelFormat*)format;
{
  self = [super initWithFrame:frame pixelFormat:format];
  if (self) {
    @autoreleasepool {
      NSWorkspace* workspace = [NSWorkspace sharedWorkspace];

      [[workspace notificationCenter] addObserver:self
                                         selector:@selector(receiveSleepNotification:)
                                             name:NSWorkspaceWillSleepNotification
                                           object:nil];
      [[workspace notificationCenter] addObserver:self
                                         selector:@selector(receiveWakeNotification:)
                                             name:NSWorkspaceDidWakeNotification
                                           object:nil];
    }
    GLint opacity = 0, interval = 0;
    [[self openGLContext] setValues:&opacity forParameter:NSOpenGLCPSurfaceOpacity];
    [[self openGLContext] setValues:&interval forParameter:NSOpenGLCPSwapInterval];
    [self prepareOpenGL];
    m_isInitialized = false;
    m_isDirty = false;
    m_isShown = false;
  }
  return self;
}

-(void) dealloc
{
  @autoreleasepool {
    [[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:self];
  }
  [super dealloc];
}

-(void) receiveSleepNotification:(NSNotification*)notification
{
  m_isDirty = true;
  [self setOverlayCache:std::shared_ptr<LPOverlayCache>()]; // Clear overlays
  [self drawRect:NSZeroRect];
}

-(void) receiveWakeNotification:(NSNotification*)notification
{
  m_isDirty = true;
  [self drawRect:NSZeroRect];
}

-(void) drawRect:(NSRect)bounds
{
  boost::unique_lock<boost::mutex> lock(m_mutex);

  if (!m_overlayCache) {
    if (m_isDirty) {
      NSOpenGLContext* context = [self openGLContext];
      [context makeCurrentContext];

      glDrawBuffer(GL_BACK);
      glClear(GL_COLOR_BUFFER_BIT);
      CGLFlushDrawable(static_cast<CGLContextObj>([context CGLContextObj]));
      glClear(GL_COLOR_BUFFER_BIT);

      m_isDirty = false;
    }
    if (m_isShown) {
      @autoreleasepool {
        [[self window] orderOut:self]; // Hide window
      }
      m_isShown = false;
    }
    return;
  }
  bool render = m_isDirty; // We must re-render if the display is dirty

  m_isDirty = false; // Assume not dirty until proven otherwise
  bounds = [[self window] frame]; // Use the bounds of the window instead of what was passed to this function
  CGRect rect = NSRectToCGRect(bounds);
  // Adjust from lower-left display coordinates to screen coordinates
  rect.origin.y = m_overlayCache->mainDisplayHeight - (rect.origin.y + rect.size.height);

  if (!render) { // See if there is anything to draw this time
    for (const auto& iconRect : m_overlayCache->overlayImageBoundingBox) {
      if (!CGRectIsNull(CGRectIntersection(rect, iconRect))) {
        render = true;
        break;
      }
    }
  }
  if (render) {
    NSOpenGLContext* context = [self openGLContext];
    [context makeCurrentContext];

    const GLsizei numOverlays = static_cast<GLsizei>(m_overlayCache->overlayImageBoundingBox.size());
    if (m_isShown && numOverlays == 0) {
      @autoreleasepool {
        [[self window] orderOut:self]; // Hide window
      }
      m_isShown = false;
    } else if (!m_isShown && numOverlays > 0) {
      @autoreleasepool {
        [[self window] orderFrontRegardless]; // Show window
      }
      m_isShown = true;
    }

    if (!m_isInitialized) {
      m_isInitialized = true;

      const GLsizei width = static_cast<GLsizei>(bounds.size.width);
      const GLsizei height = static_cast<GLsizei>(bounds.size.height);

      glViewport(0, 0, width, height);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0, width, 0, height, -1, 1);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_BLEND);
      glDisable(GL_DEPTH_TEST);
      glDisable(GL_LIGHTING);
      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);

      glClearColor(0, 0, 0, 0);
      glDrawBuffer(GL_FRONT);
      glClear(GL_COLOR_BUFFER_BIT);
      glDrawBuffer(GL_BACK);
    }
    glClear(GL_COLOR_BUFFER_BIT);

    if (numOverlays > 0) {
      GLuint textures[numOverlays];
      GLuint index = 0;
      uint32_t* address = m_overlayCache->data;

      glGenTextures(numOverlays, textures);
      for (const auto& iconRect : m_overlayCache->overlayImageBoundingBox) {
        if (!CGRectIsNull(CGRectIntersection(rect, iconRect))) {
          const GLfloat x = iconRect.origin.x - rect.origin.x;
          const GLfloat y = rect.size.height - iconRect.size.height - iconRect.origin.y + rect.origin.y;
          const GLsizei width = static_cast<GLsizei>(iconRect.size.width);
          const GLsizei height = static_cast<GLsizei>(iconRect.size.height);

          glBindTexture(GL_TEXTURE_2D, textures[index]);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, address);
          address += width*height;
          glEnable(GL_TEXTURE_2D);
          glBegin(GL_QUADS);
          glTexCoord2f(0, 0); glVertex2f(x, y);
          glTexCoord2f(1, 0); glVertex2f(x + width, y);
          glTexCoord2f(1, 1); glVertex2f(x + width, y + height);
          glTexCoord2f(0, 1); glVertex2f(x, y + height);
          glEnd();

          m_isDirty = true;
        }
        index++;
      }
      glDeleteTextures(numOverlays, textures);
    }
    CGLFlushDrawable(static_cast<CGLContextObj>([context CGLContextObj]));
  }
}

-(void) setOverlayCache:(std::shared_ptr<LPOverlayCache>)overlayCache
{
  m_overlayCache = overlayCache;
}

-(BOOL) isOpaque
{
  return NO;
}
@end

//
// LPOverlay
//

LPOverlay::LPOverlay() : m_mainDisplayHeight(0), m_modifiedCache(false), m_keepThreadRunning(true)
{
  CGDisplayRegisterReconfigurationCallback(ConfigurationChangeCallback, this);
  m_renderThread = boost::thread(boost::bind(&LPOverlay::Render, this));
  Update();
}

LPOverlay::~LPOverlay()
{
  CGDisplayRemoveReconfigurationCallback(ConfigurationChangeCallback, this);

  while (!m_displays.empty()) {
    [static_cast<NSWindow*>(m_displays.back()) release];
    m_displays.pop_back();
  }
  boost::unique_lock<boost::mutex> lock(m_renderMutex);
  m_keepThreadRunning = false;
  m_modifiedCache = true;
  m_renderCondition.notify_all();
  lock.unlock();
  m_renderThread.join();
}

// Called when the the display configuration changes
void LPOverlay::ConfigurationChangeCallback(CGDirectDisplayID display,
                                            CGDisplayChangeSummaryFlags flags,
                                            void *that)
{
  if (that) {
    static_cast<LPOverlay*>(that)->Update();
  }
}

void LPOverlay::Update()
{
  uint32_t numDisplays = 32; // Support up to 32 displays
  CGDirectDisplayID screenIDs[numDisplays];

  const NSOpenGLPixelFormatAttribute attrs[] = {
    NSOpenGLPFAAccelerated,
    NSOpenGLPFANoRecovery,
    NSOpenGLPFADoubleBuffer,
    0
  };

  // Need to worry about multiple threads accessing the displays -- FIXME
  while (!m_displays.empty()) {
    [static_cast<NSWindow*>(m_displays.back()) release];
    m_displays.pop_back();
  }

  m_mainDisplayHeight = 0;
  if (CGGetActiveDisplayList(numDisplays, screenIDs, &numDisplays) == kCGErrorSuccess && numDisplays > 0) {
    for (int i = 0; i < numDisplays; i++) {
      NSRect bounds = NSRectFromCGRect(CGDisplayBounds(screenIDs[i]));
      const NSRect viewBounds = NSMakeRect(0, 0, bounds.size.width, bounds.size.height);

      if (i == 0) {
        m_mainDisplayHeight = bounds.size.height;
      } else {
        // Adjust from screen coordinates to lower-left display coordinates
        bounds.origin.y = m_mainDisplayHeight - (bounds.origin.y + bounds.size.height);
      }

      NSWindow* window = [[NSWindow alloc] init];
      [window setOpaque:NO];
      [window setHasShadow:NO];
      [window setHidesOnDeactivate:NO];
      [window setStyleMask:NSBorderlessWindowMask];
      [window setAcceptsMouseMovedEvents:NO];
      [window setIgnoresMouseEvents:YES];
      [window setBackgroundColor:[NSColor clearColor]];
      [window setBackingType:NSBackingStoreBuffered];
      [window setSharingType:NSWindowSharingNone];
      [window setLevel:CGShieldingWindowLevel()];
      [window setCollectionBehavior:(NSWindowCollectionBehaviorCanJoinAllSpaces |
                                     NSWindowCollectionBehaviorStationary |
                                     NSWindowCollectionBehaviorFullScreenAuxiliary |
                                     NSWindowCollectionBehaviorIgnoresCycle)];
      NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
      LPOpenGLView *openGLView = [[LPOpenGLView alloc] initWithFrame:viewBounds pixelFormat:pixelFormat];

      [window setFrame:bounds display:NO];
      [window setContentView:openGLView];
      [window orderOut:window]; // Hide window

      [openGLView release];
      [pixelFormat release];

      m_displays.push_back(window);
    }
  }
  Flush();
}

void LPOverlay::Render()
{
  boost::unique_lock<boost::mutex> lock(m_renderMutex);

  while (m_keepThreadRunning) {
    if (!m_modifiedCache) {
      m_renderCondition.wait(lock);
    }
    if (m_modifiedCache && m_keepThreadRunning) {
      std::shared_ptr<LPOverlayCache> overlayCache = m_overlayCache;
      m_overlayCache.reset();
      m_modifiedCache = false;
      lock.unlock();

      // Perform the actual rendering
      for (size_t i = 0; i < m_displays.size(); i++) {
        LPOpenGLView* view = static_cast<LPOpenGLView*>([static_cast<NSWindow*>(m_displays[i]) contentView]);
        [view setOverlayCache:overlayCache];
        [view drawRect:NSZeroRect];
      }

      lock.lock();
    }
  }
}

void LPOverlay::AddIcon(const std::shared_ptr<LPIcon>& icon)
{
  if (icon) {
    m_icons.insert(icon);
  }
}

void LPOverlay::RemoveIcon(const std::shared_ptr<LPIcon>& icon)
{
  if (icon) {
    m_icons.erase(icon);
  }
}

void LPOverlay::Flush()
{
  std::shared_ptr<LPOverlayCache> overlayCache;

  if (!m_icons.empty()) {
    overlayCache.reset(new LPOverlayCache);

    if (overlayCache) {
      overlayCache->mainDisplayHeight = m_mainDisplayHeight;
      size_t size = 0;

      overlayCache->overlayImageBoundingBox.reserve(m_icons.size());

      // Determine the bounding boxes and pixel size of all rasters
      for (const auto& icon : m_icons) {
        if (icon->GetVisibility()) {
          const std::shared_ptr<LPImage>& image = icon->GetImage();

          if (image && image->GetInternalImage()) {
            const LPPoint& position = icon->GetPosition();
            const POINT& hotspot = image->GetHotspot();
            const CGRect boundingBox = CGRectMake(std::floor(position.x) - hotspot.x,
                                                  std::floor(position.y) - hotspot.y,
                                                  image->GetWidth(), image->GetHeight());
            overlayCache->overlayImageBoundingBox.push_back(boundingBox);
            size += image->GetWidth() * image->GetHeight();
          }
        }
      }
      if (size > 0) {
        uint32_t* address = new uint32_t[size]; // Use single buffer to hold all of the images

        if (address) {
          overlayCache->data = address; // Transferring ownership of allocated memory to the overlay cache
          size_t offset = 0;

          for (const auto& icon : m_icons) {
            if (icon->GetVisibility()) {
              const std::shared_ptr<LPImage>& image = icon->GetImage();

              if (image) {
                const uint32_t* imageSource = image->GetInternalImage();

                // Store a copy of each image raster so we have it when we are ready to render to screen
                if (imageSource) {
                  const size_t imageSize = image->GetWidth() * image->GetHeight();
                  memcpy(address + offset, imageSource, imageSize*sizeof(uint32_t));
                  offset += imageSize;
                }
              }
            }
          }
        } else {
          overlayCache->overlayImageBoundingBox.clear();
        }
      }
    }
  }
  boost::unique_lock<boost::mutex> lock(m_renderMutex);
  m_overlayCache = overlayCache;
  m_modifiedCache = true;
  m_renderCondition.notify_all();
}
