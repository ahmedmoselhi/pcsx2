#include "input_device.h"

#include <map>
#include <vector>

namespace usb_python2
{
	std::vector<uint8_t> acio_unescape_packet(std::vector<uint8_t>& buffer);
	std::vector<uint8_t> acio_escape_packet(std::vector<uint8_t>& buffer);

	class acio_device_base : public input_device
	{
	public:
		virtual bool device_write(std::vector<uint8_t>& packet, std::vector<uint8_t>& response) = 0;

	protected:
		uint8_t calculate_checksum(std::vector<uint8_t>& buffer);
	};

	class acio_device : public acio_device_base
	{
		std::map<int, std::unique_ptr<acio_device_base>> devices;

		bool device_write(std::vector<uint8_t>& packet, std::vector<uint8_t>& response) { return false; }

		void write(std::vector<uint8_t>& packet);

	public:
		void add_acio_device(int index, std::unique_ptr<acio_device_base> device) noexcept;
	};
} // namespace usb_python2
#pragma once