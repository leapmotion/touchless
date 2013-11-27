/*==================================================================================================================

    Copyright (c) 2010 - 2012 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/


#ifndef __ocu_Type_h__
#define __ocu_Type_h__

#include "ocuConfig.h"

#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif

  #pragma warning(disable : 4482)  // enum used in namespace

  // Using deprecated POSIX names like stricmp, but that's OK
  #define _CRT_NONSTDC_NO_DEPRECATE

#endif // _WIN32

const double OCU_EPSILON           =  1E-10;
const double OCU_PI                =  3.1415926535898;
const double RADIANS_TO_DEGREES    =  180.0 / OCU_PI;
const double DEGREES_TO_RADIANS    =  OCU_PI / 180.0;
const double MICROSEC_TO_SEC       =  0.000001;

const int NUM_CAMERAS              = 2;

const char FILE_EXTENSION_ASCII[] = ".txt";
const char FILE_EXTENSION_BINARY[] = ".bin";

enum { FILE_MODE_BINARY, FILE_MODE_ASCII, FILE_MODE_BOTH };

#if defined(_MSC_VER) && (_MSC_VER < 1600)
// Visual Studio 2008
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int32 uint32_t;
#else
#include <stdint.h>
#endif

typedef uint32_t            FileOffsetType;
typedef int64_t             FrameCounterType;
typedef int64_t             TimeStampType;

const int                   TIME_STAMP_TICKS_PER_SEC  = 1000000;
const double                TIME_STAMP_SECS_TO_TICKS  = static_cast<double>(TIME_STAMP_TICKS_PER_SEC);
const double                TIME_STAMP_TICKS_TO_SECS  = 1.0/TIME_STAMP_SECS_TO_TICKS;

const int CONTOUR_MAX_PEAKS                 = 200;   //WARNING: Must be multiple of 4 and less than 256
const int CONTOUR_MAX_EDGES                 = 20000; //WARNING: Must be multiple of 4
const int IMAGE_MAX_TIPS                    = 120;   //WARNING: Must be multiple of 4
const int ELLIPSES_MAX_PER_FRAME            = 8000;
const int IMAGE_MAX_PEAK_POINTS_2D          = 4000;
const int IMAGE_MAX_PEAK_POINTS_3D          = 10000;

const int MAX_DEVICE_FRAMES_PER_SEC         = 300;
const int TRACKING_HISTORY_SIZE             = MAX_DEVICE_FRAMES_PER_SEC;

const int NUM_DIGIT_IN_FILE_SEQUENCE_NAME = 5;
const int VIDEO_INPUT_MANUAL_DELAY_MS     = 16; // usually ignored, replaced by camera_framerate attrib
const int MAX_NUM_DUMP_FRAMES             = 100000;
const int MAX_NUM_CAMERA_PARAMETERS       = 16;

// installation constants
#if _WIN32 || __APPLE__
const char APPLICATION_DIRECTORY[] = "Leap Motion"; // use this directory located under [user]/AppData/Roaming/
#else
const char APPLICATION_DIRECTORY[] = ".Leap Motion"; // use this directory located under [user]/AppData/Roaming/
#endif
const char LICENSE_KEY_FILENAME[] = "leapid"; // use this filename to store the license key

// support constants
// LOCALIZATION TODO: When we have multi-language websites, we should make these translated.
const char MAIN_WEBSITE_URL[] = "www.leapmotion.com";
const char SUPPORT_WEBSITE_URL[] = "support.leapmotion.com";
const char DEVELOPER_WEBSITE_URL[] = "developer.leapmotion.com";

// Obfuscation
#if IS_INTERNAL_BUILD != 1
#define ContourProfile _DeepNeuralNet
#define ContourMatcher _TrifocalTensor
#define MeanCut        _DepthMap
#endif

#endif
