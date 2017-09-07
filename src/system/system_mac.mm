#include "common/system/system.h"

#import <Cocoa/Cocoa.h>
#import <CoreServices/CoreServices.h>

namespace {
std::string StatusToString(OSStatus status) {
  char str[20] = {0};
  *(UInt32*)(str + 1) = CFSwapInt32HostToBig(status);
  if (isprint(str[1]) && isprint(str[2]) && isprint(str[3]) && isprint(str[4])) {
    str[0] = str[5] = '\'';
    str[6] = '\0';
  } else {
    sprintf(str, "%d", (int)status);
  }

  return str;
}
}

namespace common {
namespace system {

ErrnoError Shutdown(shutdown_t type) {
  /*
   *    kAERestart        will cause system to restart
   *    kAEShutDown       will cause system to shutdown
   *    kAEReallyLogout   will cause system to logout
   *    kAESleep          will cause system to sleep
   */

  AEEventID eventToSendID;
  if (type == SHUTDOWN) {
    eventToSendID = kAEShutDown;
  } else if (type == LOGOUT) {
    eventToSendID = kAEReallyLogOut;

  } else if (type == REBOOT) {
    eventToSendID = kAERestart;
  } else {
    return make_error_perror("systemShutdown", EINVAL, ERROR_TYPE);
  }

  AEAddressDesc targetDesc;
  static const ProcessSerialNumber kPSNOfSystemProcess = {0, kSystemProcess};
  AppleEvent eventReply = {typeNull, NULL};
  AppleEvent eventToSend = {typeNull, NULL};

  OSStatus status =
      AECreateDesc(typeProcessSerialNumber, &kPSNOfSystemProcess, sizeof(kPSNOfSystemProcess), &targetDesc);

  if (status != noErr) {
    return make_errno_error(StatusToString(status), status, ERROR_TYPE);
  }

  status = AECreateAppleEvent(kCoreEventClass, eventToSendID, &targetDesc, kAutoGenerateReturnID, kAnyTransactionID,
                              &eventToSend);

  AEDisposeDesc(&targetDesc);

  if (status != noErr) {
    return make_errno_error(StatusToString(status), status, ERROR_TYPE);
  }

  status = AESendMessage(&eventToSend, &eventReply, kAENormalPriority, kAEDefaultTimeout);

  AEDisposeDesc(&eventToSend);
  if (status != noErr) {
    return make_errno_error(StatusToString(status), status, ERROR_TYPE);
  }

  AEDisposeDesc(&eventReply);
  return ErrnoError();
}
}
}
