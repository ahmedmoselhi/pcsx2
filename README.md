# PCSX2 (P2IO fork)
This is a fork of PCSX2 that implements a USB Python 2 I/O device to allow for Python 2 arcade games to be played using PCSX2. Please DO NOT report any issues relating to Python 2 to the official PCSX2 upstream repository.

This fork only applies for Python 2 games. This does not and will never support Python 1 (entirely separate from Python 2) or System 246/256 games as those would require much more work than a simple USB device to make work. The MG code being used in this fork also does not support memory card MG encryption stuff properly (the memory card dongles used for Python 1 and System 246/256) so those platforms aren't really any closer to working compared to before.

## Supported games
The following is a list of all known games supported by the Python 2 platform, including undumped games. If a game is not on this list then it most likely is not a Python 2 game!

| Game Title  | Status    | Notes |
|-------------|-----------|-------|
| Dance 86.4 Funky Radio Station | Untested | I/O unimplemented until a dump is found. |
| Dance Dance Revolution SuperNOVA | Supported |       |
| Dance Dance Revolution SuperNOVA 2 | Supported |       |
| Drummania V | Supported |       |
| Drummania V2 | Supported |       |
| Drummania V2 1.01 | Supported |       |
| Drummania V3 | Supported |       |
| Guitar Freaks V | Supported |       |
| Guitar Freaks V2 | Supported |       |
| Guitar Freaks V2 1.01 | Supported |       |
| Guitar Freaks V3 | Supported |       |
| Thrill Drive 3 | Supported | Devices are stubbed. Enable DIPSW 2 to remove calibration error. |
| Toy's March | Supported |       |
| Toy's March 2 | Supported |       |

## Builds
Latest builds can be downloaded thanks to nightly.link:

Windows: https://nightly.link/987123879113/pcsx2/workflows/windows-workflow/master?status=completed

Linux: https://nightly.link/987123879113/pcsx2/workflows/linux-workflow/master?status=completed

The above builds are automatically created through Github Actions whenever code is pushed to the repository, so it should always be the latest version when downloaded through these links.

## Instructions
1. Select a BIOS SCPH-50000 or later.

2. Settings > Network and HDD Settings > enable the "Hard Disk Drive" and select a random file to enable the HDD. You can create a 0 byte blank file named fake_hdd.raw and open it here to avoid PCSX2 trying to make a new 40gb HDD image.

3. Enable Python 2 as the USB device through Config > USB Settings > set `Python 2` as Port 1 and `None` as Port 2.
    - (Windows Only) Select `RawInput` as the Device API to allow for mappable controls.
    - Select `Passthrough` as the Device API to use a real USB Python 2 I/O device.

4. Configure your controls and select the game to load by pressing the `Configure` button under the USB Port 1 Device API.

5. Use System > Boot BIOS to start game.
    - Running dnasload.elf directly is not required and not recommended. If you are required to do so (boot BIOS does not work) then you most likely are using a BIOS that is too old. See note about BIOS versions.

## Important Notes
- For MG support to work, put civ.bin, cks.bin, eks.bin, and kek.bin in the BIOS folder (you must find these on your own). Python 2 games won't work without these files.

- Python 2 BIOS dumped from multiple machines using the PS2 BIOS dumper tool resulted in SCPH-50000_BIOS_V10_JAP_190.BIN (SHA-1: `0ea98a25a32145dda514de2f0d4bfbbd806bd00c`).
    - It's a stock BIOS. The exact BIOS is not required but you must use a similar aged (or later) BIOS or else it won't try booting directly from the HDD. It's not an issue with PCSX2 or the fork.
    - Early BIOS versions WILL NOT WORK and booting using System > boot BIOS will just go to the dashboard. Again, you must use a later revision of the PS2 BIOS.

- Use the Software renderer for Guitar Freaks, Drummania, Toy's March (all GFDM engine-based games) to fix graphical glitches.
    - Dance Dance Revolution, Thrill Drive 3 seem unaffected.

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

## Extended pnach patch files
The .pnach patch format has been extended to include a new command, `patchExtended`, which additionally checks the data before patching it to avoid overwriting unintended data. This is required because games go through multiple bootloaders before you're in-game, and overwriting data at the wrong time could cause things to stop working.

The format is the same as the regular `patch` command with an extra parameter at the end of the command which contains the pre-patched expected data.
```
gametitle=Test Game

// Reroute stubbed detailed debug print messages to logger
patchExtended=1,EE,3dfe00,word,08061574,27bdffb0
patchExtended=1,EE,3dfe04,word,00000000,ffa50018
```
## Backing up Python 2 games
The data on the HDD is DNAS encrypted and is tied directly to the original HDD and the PS2 itself. You can't write an unmodified image to a different HDD and have it boot on real hardware as normal. You can't swap out the PS2 itself with another stock PS2 and have it work as normal. All of the parts must match for the DNAS decryption process to work. This also means that if your real Python 2 hardware (the PS2 itself or the HDD itself) were to ever break you're out of luck.

You must provide your own dumps for use with this fork. The bare minimum required is a full raw HDD image of the HDD, a dump of the HDD ID from the exact HDD that the image was created for, and an ILINK ID dump from the exact PS2 that the HDD is tied to (can be dumped as a separate file or pulled from an NVRAM dump). An NVRAM dump is nice to have if possible but not required. If you somehow don't have an ILINK_ID.bin dump but do have an NVRAM/NVM dump, you can extract the ILINK_ID.bin data from the NVM file by copying 8 bytes from 0x1e0 into a new file named ILINK_ID.bin.

#### HDD imaging
For dumping the HDD image, you can use a tool such as `dd` (most standard Linux distributions include or have an easy way to install `dd`). For Windows I recommend `HDD Raw Copy Tool` (https://hddguru.com/software/HDD-Raw-Copy-Tool/). Anything that is able to make complete raw HDD dumps should work.

#### BIOS/ILINK/NVRAM dumping
The following tools must be run on the PS2 directly using an exploit. You can use a standard FreeMcBoot memory card to boot these, or put the tools on a USB thumb stick and launch them using the FreeMcBoot memory card.
- PS2 BIOS dumper (https://pcsx2.net/download/releases/tools/category/9-tools.html) can be used to make a PS2 BIOS dump + NVRAM dump from the PS2.
- id dumper (https://www.psx-place.com/threads/id-dumper-by-krhacken-dump-ps2-ilink-ids.11380/) can be used to dump the HDD_ID.bin, ILINK_ID.bin, and MC_NVRAM.bin (same as the .NVM files output by PS2 BIOS dumper).
    - Note: Requires the dev9.irx, atad.irx and hdd.irx files for HDD ID to be dumped (not included)

#### HDD ID dumping
There are two methods for dumping the HDD ID required for decrypting the contents of the HDD. The first is to run the id dumper tool with the HDD inserted into a PS2 that is not the original PS2, or optionally by switching out the network adapter using the original arcade PS2. This is required to stop the PS2 from priority booting from the HDD before it can launch the dumper tools. This requires extra hardware that may not be on hand.

The easier and safer method requires either connecting the HDD directly to a PC using a physical IDE connection (without a USB adapter), or an IDE to USB adapter that has a supported chipset that can be used for ATA passthrough to issue raw ATA commands.

The following commands use sg_raw, which is part of sg3_utils. You can download a Win32 build from the developer's website: https://sg.danny.cz/sg/p/sg3_utils-1.42exe.zip

**NOTE: You must run the following sg_scan and sg_raw with Administrator privileges or it won't work!!!**

To find the target hard drive, use `sg_scan`.

```
>sg_scan -b
PD0     [F]     <Sata >  Drive1
PD1     [F]     <Sata >  Drive2
PD2     [F]     <Sata >  Drive3
...
```
The PD0/PD1/PD2/etc here (/dev/sda or similar on Linux) is the target drive name you should use for the following sg_raw commands in place of where it says `/dev/sda`.

For connecting directly to the PC through IDE, you can use the following command:
```
sg_raw -o HDD_ID.bin -b -r 512 /dev/sda 85 09 0d 00 ec 00 00 00 00 00 00 00 00 00 8e 00
```

SAT12 version of the same command (depends on chip in USB adapter):
```
sg_raw -o HDD_ID.bin -b -r 512 /dev/sda a1 09 0d ec 00 00 00 00 00 8e 00 00
```

JMicron-specific version of the same command (requires a JMicron chip in USB adapter):
```
sg_raw -o HDD_ID.bin -b -r 512 /dev/sda df 10 00 02 00 ec 00 00 00 00 00 8e
```
(All sg_raw commands thanks to @dev_console)

I personally have had success using the JMicron command with a very cheap adapter (non-affiliate link): https://www.amazon.com/Warmstor-Adapter-Computer-Connector-Converter/dp/B076WZ1N4K/

The above linked set includes a power brick. The exact same adapter is also sold for cheaper but the USB adapter itself does not provide power, so be sure to have a power solution prepared before attempting dumping the hard drive.

## Credits/Code pulled in from others
- PCSX2 (https://github.com/PCSX2/pcsx2/)
- balika011's MG support PR (https://github.com/PCSX2/pcsx2/pull/4274 and https://github.com/PCSX2/pcsx2/issues/5092#issuecomment-986187643)
- libusb (https://github.com/libusb/libusb)
- libmmmagic
