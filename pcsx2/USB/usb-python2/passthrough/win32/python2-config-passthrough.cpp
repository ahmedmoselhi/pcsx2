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

#include <wx/fileconf.h>

#include "common/IniInterface.h"
#include "gui/AppConfig.h"

#include "USB/Win32/Config_usb.h"

#include <commctrl.h>
#include "USB/usb-python2/passthrough/usb-python2-passthrough.h"
#include "python2-config-passthrough-res.h"

#include <windowsx.h>

namespace usb_python2
{
	namespace passthrough
	{
		INT_PTR CALLBACK ConfigurePython2PassthroughDlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			switch (uMsg)
			{
				case WM_CREATE:
					SetWindowLongPtr(hW, GWLP_USERDATA, lParam);
					break;

				case WM_INITDIALOG:
				{
					SetWindowLongPtr(hW, GWLP_USERDATA, lParam);

					const Python2DlgConfig* cfg = (Python2DlgConfig*)lParam;

					SendDlgItemMessage(hW, IDC_COMBO1, CB_RESETCONTENT, 0, 0);
					std::wstring selectedDevice;
					LoadSetting(Python2Device::TypeName(), cfg->port, "python2", N_DEVICE, selectedDevice);
					for (auto i = 0; i != cfg->devList.size(); i++)
					{
						SendDlgItemMessageW(hW, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)cfg->devList[i].c_str());
						if (selectedDevice == cfg->devListGroups.at(i))
						{
							SendDlgItemMessage(hW, IDC_COMBO1, CB_SETCURSEL, i, i);
						}
					}

					return TRUE;
				}
				case WM_COMMAND:
					if (HIWORD(wParam) == BN_CLICKED)
					{
						switch (LOWORD(wParam))
						{
							case IDOK:
							{
								INT_PTR res = RESULT_OK;

								const Python2DlgConfig* cfg = (Python2DlgConfig*)GetWindowLongPtr(hW, GWLP_USERDATA);

								// Save machine configuration selection
								auto deviceIdx = ComboBox_GetCurSel(GetDlgItem(hW, IDC_COMBO1));
								if (!SaveSetting<std::wstring>(Python2Device::TypeName(), cfg->port, "python2", N_DEVICE, cfg->devListGroups[deviceIdx]))
									res = RESULT_FAILED;

								EndDialog(hW, res);
								return TRUE;
							}

							case IDCANCEL:
								EndDialog(hW, RESULT_CANCELED);
								return TRUE;
						}
					}
					break;
			}
			return FALSE;
		}
	} // namespace passthrough
} // namespace usb_python2
