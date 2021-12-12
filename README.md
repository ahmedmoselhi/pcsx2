# PCSX2 (P2IO fork)
This is a fork of PCSX2 that implements a USB Python 2 I/O device to allow for Python 2 arcade games to be played using PCSX2.
This is NOT the official PCSX2 repository. See https://github.com/PCSX2/pcsx2/ for the official repository.

## Builds
Latest builds can be downloaded thanks to nightly.link:  
Windows: https://nightly.link/987123879113/pcsx2/workflows/windows-workflow/master?status=completed  
Linux: https://nightly.link/987123879113/pcsx2/workflows/linux-workflow/master?status=completed  

The above builds are automatically created through Github Actions whenever code is pushed to the repository, so it should always be the latest version when downloaded through these links.

## Notes
- Enable game fixes to fix a hang during boot: Settings > General Settings > check `Enable manual game fixes` and `OPH Flag hack`.
- Use the Software renderer for Guitar Freaks, Drummania, Toy's March (all GFDM engine-based games) to fix graphical glitches
    - Dance Dance Revolution, Thrill Drive 3 seem unaffected

- Settings > Network and HDD Settings > enable the "Hard Disk Drive" and select a random file (you can create a blank file named fake_hdd.raw and load it here to avoid PCSX2 trying to make a new HDD image sometimes)

- For MG support to work, put civ.bin, cks.bin, eks.bin, and kek.bin in the BIOS folder (you must find these on your own)

- You can change games by pressing the `Configure` button under the Port 1 Device API.
- You must create a file named `Python2.ini` in the `inis` folder for games to be detected by the Python 2 configuration menu.
```
[CardReader]
; Card files are text files with the 16 character card ID.
; Optional. You'll know if you have a need for this.
Player1Card=card1.txt
Player2Card=card2.txt

; All game entries will start with GameEntry, and must be followed by a unique internal name
[GameEntry TestGameEntry1]
; Friendly name to display in the Python 2 configuration menu
Name=Test Game Entry 1

; Path to HDD image file.
; Note: For Windows you must use \\ instead of just \ for file paths or it WILL NOT WORK.
HddImagePath=C:\\Python2\\game1.raw

; HDD ID corresponding to the HDD image (required for unpatched drives)
HddIdPath=C:\\Python2\\game1_HDD_ID.BIN

; ILINK ID corresponding to the HDD image (required for unpatched drives)
IlinkIdPath=C:\\Python2\\game1_ILINK_ID.BIN

; Black and white dongle files (required for unpatched games)
; Format of binary dongle file is:
; 8 bytes - serial ID
; 32 bytes - encrypted dongle payload
DongleBlackPath=C:\\Python2\\game1_dongle_black.bin
DongleWhitePath=C:\\Python2\\game1_dongle_white.bin

; Input types
; 0 = Drummania
; 1 = Guitar Freaks
; 2 = Dance Dance Revolution
; 3 = Toy's March
; 4 = Thrill Drive 3
; 5 = Dance 86.4 Funky Radio Station
InputType=0

; DIP Switches 1234
; Change from 0 to 1 to enable selected dipswitch
DipSwitch=0000

; Force 31 kHz mode
; You shouldn't need to enable this but it exists.
; Will cause the top of the screen to not refresh in Guitar Freaks, Drummania, Toy's March (all GFDM engine-based games) which also occurs on real hardware.
Force31kHz=0

; Optional, extended pnach patch file (see README.md for more details)
PatchFile=C:\\Python2\\game1.pnach

; ...Repeat until all games are added...
[GameEntry TestGameEntry2]
Name=Test Game Entry 2
HddImagePath=C:\\Python2\\game2.raw
HddIdPath=C:\\Python2\\game2_HDD_ID.BIN
IlinkIdPath=C:\\Python2\\game2_ILINK_ID.BIN
DongleBlackPath=C:\\Python2\\game2_dongle_black.bin
DongleWhitePath=C:\\Python2\\game2_dongle_white.bin
InputType=2
DipSwitch=0000
Force31kHz=0
PatchFile=C:\\Python2\\game2.pnach
```

- You must enable Python 2 as the USB device through Config > USB Settings > set `Python 2` as Port 1 and `None` as Port 2.
    - (Windows Only) Select `RawInput` as the Device API to allow for mappable controls.
    - Select `Passthrough` as the Device API to use a real USB Python 2 I/O device.


## Extended pnach patch files
The .pnach patch format has been extended to include a new command, `patchExtended`, which additionally checks the data before patching it to avoid overwriting unintended data.
The format is the same as the regular `patch` command with an extra parameter at the end of the command which contains the pre-patched expected data.
```
gametitle=Test Game

// Reroute stubbed detailed debug print messages to logger
patchExtended=1,EE,3dfe00,word,08061574,27bdffb0
patchExtended=1,EE,3dfe04,word,00000000,ffa50018
```

## Credits/Code pulled in from others
- balika011's MG support PR (https://github.com/PCSX2/pcsx2/pull/4274 and https://github.com/PCSX2/pcsx2/issues/5092#issuecomment-986187643)
- libusb (https://github.com/libusb/libusb)
- libmmmagic
