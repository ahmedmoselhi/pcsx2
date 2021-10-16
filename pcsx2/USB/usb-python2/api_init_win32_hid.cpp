#include "PrecompiledHeader.h"
#include "python2proxy.h"
#include "usb-python2-raw.h"

void usb_python2::RegisterUsbPython2::Register()
{
	auto& inst = RegisterUsbPython2::instance();
	inst.Add(usb_python2::APINAME, new UsbPython2Proxy<usb_python2::raw::RawInputPad>());
}
