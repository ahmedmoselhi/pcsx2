#include "PrecompiledHeader.h"
#include "python2proxy.h"
#include "inputs/raw/usb-python2-raw.h"
#include "inputs/passthrough/usb-python2-passthrough.h"
#include "inputs/btools/usb-python2-btools.h"

void usb_python2::RegisterUsbPython2::Register()
{
	auto& inst = RegisterUsbPython2::instance();

#ifdef PCSX2_CORE
	inst.Add(usb_python2::native::APINAME, new UsbPython2Proxy<usb_python2::native::NativeInput>());
#else
	inst.Add(usb_python2::APINAME, new UsbPython2Proxy<usb_python2::raw::RawInputPad>());
#endif

	inst.Add(usb_python2::passthrough::APINAME, new UsbPython2Proxy<usb_python2::passthrough::PassthroughInput>());

#ifdef INCLUDE_BTOOLS
	inst.Add(usb_python2::btools::APINAME, new UsbPython2Proxy<usb_python2::btools::BToolsInput>());
#endif
}
