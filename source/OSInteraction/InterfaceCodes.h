#pragma once
enum eHidStatus;
//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//
#define HID_STATUS_WARNING            0x2
#define HID_STATUS_SUCCESS            0x0
#define HID_STATUS_INFORMATION        0x1
#define HID_STATUS_ERROR              0x3


//
// MessageId: eHidIntrSuccess
//
// MessageText:
//
// The operation completed successfully.
//
#define eHidIntrSuccess                  ((eHidStatus)0x20020000L)

//
// MessageId: eHidIntrGestureOverflow
//
// MessageText:
//
// The gesture had to be aborted because one of the input contact points passed beyond the boundaries of the screen
//
#define eHidIntrGestureOverflow          ((eHidStatus)0xA0020000L)

//
// MessageId: eHidIntrIoctlFailed
//
// MessageText:
//
// Communication failed with the Hid emulator
//
#define eHidIntrIoctlFailed              ((eHidStatus)0xE0020000L)

//
// MessageId: eHidIntrNotOcuHid
//
// MessageText:
//
// This HID instance is a HID device, but not a Leap Motion HidEmulator
//
#define eHidIntrNotOcuHid                ((eHidStatus)0xE0020001L)

//
// MessageId: eHidIntrHidAttributeQueryFail
//
// MessageText:
//
// Failed to query the HID instance attributes on this device
//
#define eHidIntrHidAttributeQueryFail    ((eHidStatus)0xE0020002L)

//
// MessageId: eHidIntrCompletionPortCreateFail
//
// MessageText:
//
// Failed to create a completion port for asynchronous IO
//
#define eHidIntrCompletionPortCreateFail ((eHidStatus)0xE0020003L)

//
// MessageId: eHidIntrCompletionThreadNotCreated
//
// MessageText:
//
// Failed to create a thread to wait on the completion port
//
#define eHidIntrCompletionThreadNotCreated ((eHidStatus)0xE0020004L)

//
// MessageId: eHidIntrNotRespondingProperly
//
// MessageText:
//
// The device appears to be a Leap Motion HidEmulator, but isn't responding to write reports correctly
//
#define eHidIntrNotRespondingProperly    ((eHidStatus)0xE0020005L)

//
// MessageId: eHidIntrHbitmapCreateFail
//
// MessageText:
//
// Failed to create an HBITMAP
//
#define eHidIntrHbitmapCreateFail        ((eHidStatus)0xE0020006L)

//
// MessageId: eHidIntrUpdateLayeredWindowFailed
//
// MessageText:
//
// Call to UpdateLayeredWindow failed for an overlay icon
//
#define eHidIntrUpdateLayeredWindowFailed ((eHidStatus)0xE0020007L)

//
// MessageId: eHidIntrIndexOutOfBounds
//
// MessageText:
//
// The specified display index is out of bounds
//
#define eHidIntrIndexOutOfBounds         ((eHidStatus)0xE0020008L)

//
// MessageId: eHidIntrNoFocusWindow
//
// MessageText:
//
// Could not identify the window that currently has focus
//
#define eHidIntrNoFocusWindow            ((eHidStatus)0xE0020009L)

//
// MessageId: eHidIntrPixelFormatNotRecognized
//
// MessageText:
//
// The passed pixel format was not recognized
//
#define eHidIntrPixelFormatNotRecognized ((eHidStatus)0xE002000AL)

//
// MessageId: eHidIntrCreateDibSectionFailed
//
// MessageText:
//
// Failed to create a DIB section to hold the passed image data
//
#define eHidIntrCreateDibSectionFailed   ((eHidStatus)0xE002000BL)

//
// MessageId: eHidIntrFailedToLoad
//
// MessageText:
//
// Failed to load the specified image file.  This may be because the image doesn't exist, resource shortages
// on the system, or the file itself could be corrupted or of an unsupported format.
//
#define eHidIntrFailedToLoad             ((eHidStatus)0xE002000CL)

//
// MessageId: eHidIntrFailedToLock
//
// MessageText:
//
// Failed to lock the bits of the source image.  The image file could be corrupted, or there may not be enough
// system resources to lock bits.
//
#define eHidIntrFailedToLock             ((eHidStatus)0xE002000DL)

