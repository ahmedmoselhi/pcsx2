#include "thrilldrive_handle.h"

namespace usb_python2
{
	bool thrilldrive_handle_device::device_write(std::vector<uint8_t>& packet, std::vector<uint8_t>& outputResponse)
	{
		const auto addr = packet[1];
		const auto code = (packet[2] << 8) | packet[3];
		const auto seqNum = packet[4];
		const auto packetLen = packet[5];

		std::vector<uint8_t> response;
		bool isEmptyResponse = false;
		if (code == 0x0120)
		{
			uint8_t resp[4] = {0};
			response.insert(response.end(), std::begin(resp), std::end(resp));
		}
		else
		{
			// Just return 0 for anything else
			response.push_back(0);
		}

		if (response.size() > 0 || isEmptyResponse)
		{
			outputResponse.insert(outputResponse.end(), response.begin(), response.end());
			return true;
		}

		return false;
	}
} // namespace usb_python2