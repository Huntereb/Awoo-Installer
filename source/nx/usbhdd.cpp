/*
Microsoft Public License (Ms-PL)

This license governs use of the accompanying software. If you use the
software, you accept this license. If you do not accept the license,
do not use the software.

 1. Definitions

	The terms "reproduce," "reproduction," "derivative works," and
	"distribution" have the same meaning here as under U.S. copyright
	law.

	A "contribution" is the original software, or any additions or
	changes to the software.

	A "contributor" is any person that distributes its contribution
	under this license.

	"Licensed patents" are a contributor's patent claims that read
	directly on its contribution.

 2. Grant of Rights

	(A) Copyright Grant- Subject to the terms of this license,
	including the license conditions and limitations in section 3,
	each contributor grants you a non-exclusive, worldwide,
	royalty-free copyright license to reproduce its contribution,
	prepare derivative works of its contribution, and distribute its
	contribution or any derivative works that you create.

	(B) Patent Grant- Subject to the terms of this license, including
	the license conditions and limitations in section 3, each
	contributor grants you a non-exclusive, worldwide, royalty-free
	license under its licensed patents to make, have made, use, sell,
	offer for sale, import, and/or otherwise dispose of its
	contribution in the software or derivative works of the
	contribution in the software.

 3. Conditions and Limitations

	(A) No Trademark License- This license does not grant you rights
	to use any contributors' name, logo, or trademarks.

	(B) If you bring a patent claim against any contributor over
	patents that you claim are infringed by the software, your patent
	license from such contributor to the software ends automatically.

	(C) If you distribute any portion of the software, you must retain
	all copyright, patent, trademark, and attribution notices that are
	present in the software.

	(D) If you distribute any portion of the software in source code
	form, you may do so only under this license by including a
	complete copy of this license with your distribution. If you
	distribute any portion of the software in compiled or object code
	form, you may only do so under a license that complies with this
	license.

	(E) You may not distribute, copy, use, or link any portion of this
	code to any other code that requires distribution of source code.

	(F) The software is licensed "as-is." You bear the risk of using
	it. The contributors give no express warranties, guarantees, or
	conditions. You may have additional consumer rights under your
	local laws which this license cannot change. To the extent
	permitted under your local laws, the contributors exclude the
	implied warranties of merchantability, fitness for a particular
	purpose and non-infringement.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <dirent.h>
#include <threads.h>
#include <usbhsfs.h>
#include <mutex>

namespace nx::hdd
{
	static const u32 MAX_DEVICES = 32;
	static UEvent *g_statusChangeEvent = NULL, g_exitEvent = { 0 };

	static u32 g_usbDeviceCount = 0;
	UsbHsFsDevice g_usbDevices[MAX_DEVICES];

	static thrd_t g_thread = { 0 };
	static std::mutex g_mutex;

	static int entry(void *arg)
	{
		(void)arg;

		Result rc = 0;
		int idx = 0;
		u32 listed_device_count = 0;

		Waiter status_change_event_waiter = waiterForUEvent(g_statusChangeEvent);
		Waiter exit_event_waiter = waiterForUEvent(&g_exitEvent);

		while(true)
		{
			rc = waitMulti(&idx, -1, status_change_event_waiter, exit_event_waiter);
			if(R_FAILED(rc)) continue;
			
			std::scoped_lock lock(g_mutex);

			if(idx == 1)
			{
				break;
			}

			g_usbDeviceCount = usbHsFsGetMountedDeviceCount();

			if(!g_usbDeviceCount) continue;

			if(!(listed_device_count = usbHsFsListMountedDevices(g_usbDevices, std::min(g_usbDeviceCount, MAX_DEVICES))))
			{
				continue;
			}
		}
		
		g_usbDeviceCount = 0;

		return 0;
	}

	u32 count()
	{
		return std::min(g_usbDeviceCount, MAX_DEVICES);
	}

	const char* rootPath(u32 index)
	{
		if(index >= MAX_DEVICES)
		{
			return nullptr;
		}

		std::scoped_lock lock(g_mutex);
		
		if(index < usbHsFsGetMountedDeviceCount())
		{
			return g_usbDevices[index].name;
		}

		return nullptr;
	}

	bool init()
	{
		if(g_statusChangeEvent)
		{
			return true;
		}

		if(usbHsFsInitialize(0))
		{
			return false;
		}

		g_statusChangeEvent = usbHsFsGetStatusChangeUserEvent();

		ueventCreate(&g_exitEvent, true);

		thrd_create(&g_thread, entry, NULL);

		return true;
	}

	bool exit()
	{
		if(!g_statusChangeEvent)
		{
			return false;
		}

		ueventSignal(&g_exitEvent);

		thrd_join(g_thread, NULL);

		g_statusChangeEvent = NULL;
		
		usbHsFsExit();

		return true;
	}
}
