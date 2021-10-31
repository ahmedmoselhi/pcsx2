#include "toysmarch_drumpad.h"

#include <algorithm>
#include <iterator>
#include <numeric>
#include <deque>

namespace usb_python2
{
	uint8_t stateIdx = 0;

	std::vector<uint8_t> toysmarch_drumpad_device::generate_state_response()
	{
		std::vector<uint8_t> response;

		uint8_t state[9] = {0};
		state[0] = stateIdx;
		state[1] = p2dev->GetKeyState(L"ToysMarchP1Cymbal") * 128;
		state[2] = p2dev->GetKeyState(L"ToysMarchP1DrumL") * 128;
		state[3] = p2dev->GetKeyState(L"ToysMarchP1DrumR") * 128;
		state[5] = p2dev->GetKeyState(L"ToysMarchP2Cymbal") * 128;
		state[6] = p2dev->GetKeyState(L"ToysMarchP2DrumL") * 128;
		state[7] = p2dev->GetKeyState(L"ToysMarchP2DrumR") * 128;
		stateIdx = (stateIdx + 1) % 8;

		response.push_back(0xaa);
		response.insert(response.end(), std::begin(state), std::end(state));
		response.push_back(std::accumulate(response.begin() + 1, response.end(), 0));

		return response;
	}

	int toysmarch_drumpad_device::read(std::vector<uint8_t>& buf, const size_t requestedLen)
	{
		// The game always expects to be able to read the state from the device, regardless of what was written
		auto response = generate_state_response();

		buf.insert(buf.end(), response.begin(), response.end());

		return response.size();
	}

	void toysmarch_drumpad_device::write(std::vector<uint8_t>& packet)
	{
		// Keep alive packets
		// aa 11 11 11 33
		// aa 22 22 22 66
		add_packet(generate_state_response());
	}
} // namespace usb_python2