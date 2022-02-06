# PCSX2 (P2IO fork)
This is a fork of PCSX2 that implements a USB Python 2 I/O device to allow for Python 2 arcade games to be played using PCSX2. Please DO NOT report any issues relating to Python 2 to the official repository. 

This fork only applies for Python 2 games. This does not and will never support Python 1 (entirely separate from Python 2) or System 246/256 games as those would require much more work than a simple USB device to make work. The MG code being used in this fork also does not support memory card MG encryption stuff properly (the memory card dongles used for Python 1 and System 246/256) so those platforms aren't really any closer to working compared to before.

## Backing up Python 2 games
The data on the HDD is DNAS encrypted and is tied directly to the original HDD and the PS2 itself. You can't write an unmodified image to a different HDD and have it boot on real hardware as normal. You can't swap out the PS2 itself with another stock PS2 and have it work as normal. All of the parts must match for the DNAS decryption process to work. This also means that if your real Python 2 hardware (the PS2 itself or the HDD itself) were to ever break you're out of luck.

You must provide your own dumps for use with this fork. The bare minimum required is a full raw HDD image of the HDD, a dump of the HDD ID from the exact HDD that the image was created for, and an ILINK ID dump from the exact PS2 that the HDD is tied to (can be dumped as a separate file or pulled from an NVRAM dump).

Required tools for dumping
- dd (or anything that can make full raw HDD images)
- id dumper https://www.psx-place.com/threads/id-dumper-by-krhacken-dump-ps2-ilink-ids.11380/
    - This tool generates HDD_ID.BIN, ILINK_ID.bin, and MC_NVRAM.BIN (same as NVM dumped from BIOS dumper)
    - Requires the dev9.irx, atad.irx and hdd.irx files for HDD ID to be dumped (not included)
- PS2 BIOS dumper https://pcsx2.net/download/releases/tools/category/9-tools.html
    - Dumps the PS2 BIOS and the nvm/etc files

If you somehow don't have an ILINK_ID.bin dump but do have an NVM dump, you can extract the ILINK_ID.bin directly from the NVM file by copying 8 bytes from 0x1e0 into a new file named ILINK_ID.bin.

There is an alternative method for dumping the HDD ID directly from the HDD using a tool to send raw commands to the drive, but it does not play well with USB adapters. It apparently works if you can connect the drive directly to your computer. I was not able to get it to work using multiple USB adapters so your milage may vary with this method.
```
sg_raw -b -r 512 /dev/sda 85 09 0d 00 ec 00 00 00 00 00 00 00 00 00 8e 00
```
Credit: @dev_console https://twitter.com/dev_console/status/1457584811422453761

Some finessing is required to get the HDD_ID.bin to dump directly on the source PS2 machine because HDD boot takes priority. Swapping out the network adapter or PS2 itself so that it's no longer able to decrypt the HDD data on boot, or overwriting the first few sectors of the HDD with garbage data so it's not recognized as a bootable HDD (don't forget to restore the overwritten data from your HDD image backup!) is enough but do it at your own risk.

Please contact me if a safer, more reliable method of dumping the HDD_ID.bin is discovered and I'll add it here.

## Builds
Latest builds can be downloaded thanks to nightly.link:  
Windows: https://nightly.link/987123879113/pcsx2/workflows/windows-workflow/master?status=completed  
Linux: https://nightly.link/987123879113/pcsx2/workflows/linux-workflow/master?status=completed  

The above builds are automatically created through Github Actions whenever code is pushed to the repository, so it should always be the latest version when downloaded through these links.

## Notes
- Python 2 BIOS dumped from multiple machines using the PS2 BIOS dumper tool resulted in SCPH-50000_BIOS_V10_JAP_190.BIN (SHA-1: `0ea98a25a32145dda514de2f0d4bfbbd806bd00c`).
    -  It's a stock BIOS. The exact BIOS is not required but you must use a similar aged (or later) BIOS or else it won't try booting directly from the HDD. It's not an issue with PCSX2 or the fork.
    
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
The .pnach patch format has been extended to include a new command, `patchExtended`, which additionally checks the data before patching it to avoid overwriting unintended data. This is required because games go through multiple bootloaders before you're in-game, and overwriting data at the wrong time could cause things to stop working. 

The format is the same as the regular `patch` command with an extra parameter at the end of the command which contains the pre-patched expected data.
```
gametitle=Test Game

// Reroute stubbed detailed debug print messages to logger
patchExtended=1,EE,3dfe00,word,08061574,27bdffb0
patchExtended=1,EE,3dfe04,word,00000000,ffa50018
```

## Credits/Code pulled in from others
- PCSX2 (https://github.com/PCSX2/pcsx2/)
- balika011's MG support PR (https://github.com/PCSX2/pcsx2/pull/4274 and https://github.com/PCSX2/pcsx2/issues/5092#issuecomment-986187643)
- libusb (https://github.com/libusb/libusb)
- libmmmagic
