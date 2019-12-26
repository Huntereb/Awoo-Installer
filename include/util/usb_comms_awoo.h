/**
 * @file usb_comms.h
 * @brief USB comms.
 * @author yellows8
 * @author plutoo
 * @copyright libnx Authors
 */
#ifdef __cplusplus
extern "C" {
#endif

#pragma once
#include "switch/types.h"

typedef struct {
    u8 bInterfaceClass;
    u8 bInterfaceSubClass;
    u8 bInterfaceProtocol;
} awoo_UsbCommsInterfaceInfo;

/// Initializes usbComms with the default number of interfaces (1)
Result awoo_usbCommsInitialize(void);

/// Initializes usbComms with a specific number of interfaces.
Result awoo_usbCommsInitializeEx(u32 num_interfaces, const awoo_UsbCommsInterfaceInfo *infos);

/// Exits usbComms.
void awoo_usbCommsExit(void);

/// Sets whether to throw a fatal error in usbComms{Read/Write}* on failure, or just return the transferred size. By default (false) the latter is used.
void awoo_usbCommsSetErrorHandling(bool flag);

/// Read data with the default interface.
size_t awoo_usbCommsRead(void* buffer, size_t size, u64 timeout);

/// Write data with the default interface.
size_t awoo_usbCommsWrite(const void* buffer, size_t size, u64 timeout);

/// Same as usbCommsRead except with the specified interface.
size_t awoo_usbCommsReadEx(void* buffer, size_t size, u32 interface, u64 timeout);

/// Same as usbCommsWrite except with the specified interface.
size_t awoo_usbCommsWriteEx(const void* buffer, size_t size, u32 interface, u64 timeout);

#ifdef __cplusplus
}
#endif