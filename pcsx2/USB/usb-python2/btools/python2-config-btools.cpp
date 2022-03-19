/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2020  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

// Used OBS as example

#include "PrecompiledHeader.h"

#include <stdio.h>

#include <vector>
#include <ghc/filesystem.h>

#include <wx/wx.h>
#include <wx/collpane.h>
#include <wx/filepicker.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <wx/gbsizer.h>

#include "gui/AppCoreThread.h"

#include "gui/AppConfig.h"

#include "usb-python2-passthrough.h"

namespace usb_python2
{
	namespace btools
	{

		void ConfigurePython2Passthrough(Python2DlgConfig &config)
		{
			ScopedCoreThreadPause paused_core;

			TSTDSTRING selectedDevice;
			LoadSetting(Python2Device::TypeName(), config.port, "python2", N_DEVICE, selectedDevice);

			Python2PassthroughConfigDialog dialog(config.devList);
			dialog.Load(config, selectedDevice);

			if (dialog.ShowModal() == wxID_OK)
			{
				auto selectedIdx = dialog.GetSelectedGame();

				#ifdef _WIN32
				TSTDSTRING selectedGameEntry = config.devListGroups[selectedIdx].ToStdWstring();
				#else
				TSTDSTRING selectedGameEntry = config.devListGroups[selectedIdx].ToStdString();
				#endif
				SaveSetting<TSTDSTRING>(Python2Device::TypeName(), config.port, "python2", N_DEVICE, selectedGameEntry);
			}

			paused_core.AllowResume();
		}
	} // namespace passthrough
} // namespace usb_python2
