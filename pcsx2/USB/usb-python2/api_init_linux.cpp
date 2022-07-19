#include "python2proxy.h"
#include "noop.h"

#ifndef PCSX2_CORE
#include "passthrough/usb-python2-passthrough.h"
#endif

void usb_python2::RegisterUsbPython2::Register()
{
	auto& inst = RegisterUsbPython2::instance();

#ifndef PCSX2_CORE
	inst.Add(usb_python2::noop::APINAME, new UsbPython2Proxy<usb_python2::noop::NOOP>());
	inst.Add(usb_python2::passthrough::APINAME, new UsbPython2Proxy<usb_python2::passthrough::PassthroughInput>());
#else
	inst.Add(usb_python2::noop::APINAME, new UsbPython2Proxy<usb_python2::noop::NOOP>());
#endif
}