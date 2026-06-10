# libnfc - Free/Libre Near Field Communication library

Librería de comunicación NFC para espacio de usuario. Compatible con lectores PN532, ACR122, SCL3711, Feitian bR500/R502 y otros.

**Nota:** Este README conserva la documentación oficial aguas arriba. Las secciones siguientes son del proyecto original.

---

```
*-
* Free/Libre Near Field Communication (NFC) library
*
* Libnfc historical contributors:
* Copyright (C) 2009      Roel Verdult
* Copyright (C) 2009-2015 Romuald Conty
* Copyright (C) 2010-2012 Romain Tartière
* Copyright (C) 2010-2013 Philippe Teuwen
* Copyright (C) 2012-2013 Ludovic Rousseau
* Additional contributors:
* See AUTHORS file
*-
```

## General Information

libnfc is a library which allows userspace application access to NFC devices.

The official web site is gone, browse a copy via the [WayBackMachine](https://web.archive.org/web/20210815064122/http://nfc-tools.org/index.php/Main_Page).

The official forum site is gone, browse a copy via the [WayBackMachine](https://web.archive.org/web/20210225042232/http://forums.nfc-tools.org/).

The official development site is:
  https://github.com/nfc-tools/libnfc

## Requirements

Some NFC drivers depend on third party software:

* pn53x_usb & acr122_usb:
  - libusb-0.1 http://libusb.sf.net
* acr122_pcsc:
  - pcsc-lite https://pcsclite.apdu.fr/
* pcsc:
  - Support build with pcsc driver, which can be using all compatible readers, Feitian R502 and bR500 already passed the test.

The regression test suite depends on the cutter framework:
http://cutter.sf.net

## Building

If working from git clone, first generate files:
```
autoreconf -vis
```

Otherwise use a .tar.bz2 release:
https://github.com/nfc-tools/libnfc/releases/

```
./configure
make
```

To build with specific driver(s), see `--with-drivers=...` in `./configure --help`.

## Installation

```
make install
```

You may need udev rules for your user:
```
sudo cp contrib/udev/93-pn53x.rules /lib/udev/rules.d/
```

## Configuration

```
sudo mkdir /etc/nfc
sudo cp libnfc.conf.sample /etc/nfc/libnfc.conf
```

For multiple devices:
```
sudo mkdir -p /etc/nfc/devices.d
printf 'name = "My device"\nconnstring = "pn532_uart:/dev/ttyACM0"\n' | sudo tee /etc/nfc/devices.d/first.conf
```

## Environment Variables

- `LIBNFC_DEFAULT_DEVICE=<connstring>`: default device
- `LIBNFC_DEVICE=<connstring>`: ignore config, use only this device
- `LIBNFC_AUTO_SCAN=<true|false>`: override allow_autoscan
- `LIBNFC_INTRUSIVE_SCAN=<true|false>`: override allow_intrusive_scan
- `LIBNFC_LOG_LEVEL=<0|1|2|3>`: override log_level

To discover devices: `LIBNFC_AUTO_SCAN=true nfc-scan-device`

## Troubleshooting

See sections in [original README](./README.md) for:
- Touchatag/ACR122
- ACR122 PCSC tweaks
- SCL3711 concurrent usage
- PN533 USB on Linux >= 3.1 (blacklist nfc, pn533, pn533_usb)
- Feitian bR500/R502

## Patches & Bugs

https://github.com/nfc-tools/libnfc/issues
