#include "USB/usb-python2/usb-python2.h"
#include "acio.h"

namespace usb_python2
{
	class acio_icca_device : public acio_device_base
	{
	private:
		Python2Input* p2dev;

	protected:
		uint8_t keyLastActiveState = 0;
		uint8_t keyLastActiveEvent[2] = {0, 0};
		bool accept = false;
		bool inserted = false;
		bool isCardInsertPressed = false;
		bool isKeypadSwapped = false;
		bool isKeypadSwapPressed = false;

		bool cardLoaded = false;
		uint8_t cardId[8] = {0};
		std::string cardFilename = "";

		TSTDSTRING keypadIdsByDeviceId[2][12] = {
			{TEXT("KeypadP1_0"),
				TEXT("KeypadP1_1"),
				TEXT("KeypadP1_2"),
				TEXT("KeypadP1_3"),
				TEXT("KeypadP1_4"),
				TEXT("KeypadP1_5"),
				TEXT("KeypadP1_6"),
				TEXT("KeypadP1_7"),
				TEXT("KeypadP1_8"),
				TEXT("KeypadP1_9"),
				TEXT("KeypadP1_00"),
				TEXT("KeypadP1InsertEject")},
			{TEXT("KeypadP2_0"),
				TEXT("KeypadP2_1"),
				TEXT("KeypadP2_2"),
				TEXT("KeypadP2_3"),
				TEXT("KeypadP2_4"),
				TEXT("KeypadP2_5"),
				TEXT("KeypadP2_6"),
				TEXT("KeypadP2_7"),
				TEXT("KeypadP2_8"),
				TEXT("KeypadP2_9"),
				TEXT("KeypadP2_00"),
				TEXT("KeypadP2InsertEject")},
		};

		void write(std::vector<uint8_t>& packet) {}

	public:
		acio_icca_device(Python2Input* device)
		{
			p2dev = device;
		}

		acio_icca_device(Python2Input* device, std::string targetCardFilename)
		{
			p2dev = device;
			cardFilename = targetCardFilename;
		}

		bool device_write(std::vector<uint8_t>& packet, std::vector<uint8_t>& outputResponse);
	};
} // namespace usb_python2
#pragma once
