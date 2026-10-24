# KTM mobile data

The provider and guarded modem helpers are opt-in through
`AERA_ENABLE_MOBILE_DATA=1`. The implementation is adapted from Infiniti and
Lighthouse; it discovers the modem remoteproc and QRTR service instances at
runtime instead of depending on their device-specific numbering.

## OEM modem feature map

Checked against OnePlus 13 stock OTA `CPH2653_16.0.2.402(EX01)`:

- Stock executable: `/odm/bin/hw/mdm_feature`
- SHA-256: `5d80ade63f3c38c825d8f27968e9edb8e8c60e642f8a6b8208983c09e9eaf0b1`
- Its `read_reserve1_mdmfeature` routine at `0x4b80` reads `0x3000` bytes
  from `oplusreserve1`, using offset `0x493000` for UFS (`/dev/block/sda`)
  or `0x46a000` otherwise. These match the Infiniti/Lighthouse layout.
- Read-only inspection on the connected device running that build confirmed
  a valid 241-byte original XML record at `0x493000`, the `/dev/mdmfeature`
  character device, UIO `rmtfs`, and a valid 15-digit DMS IMEI. Identity values
  were not logged. The installed stock executable matches the hash above.
- The provider opens that partition read-only, validates the original XML,
  and copies it unchanged into shared RAM through `/dev/mdmfeature` before
  starting MSS. It does not generate or alter an IMEI, write NV partitions,
  or execute the stock utility's write/update routines.

RMTFS backing partitions remain read-only and writable TFTP/RFS paths are
RAM-only. Kernel data modules are loaded from the installed ROM, not bundled
into the recovery ramdisk. Do not stop the helpers before stopping MSS.

## Project-specific static NV

Project `23821` uses the CN static-NV image bundled at
`/system/etc/aera/mobile/23821/oplusstanvbk.img`. The provider reads the project
ID from `/proc/oplusVersion/prjName` and checks the image's exact size and
SHA-256 before starting MSS. A missing or modified image fails the connection
instead of silently selecting an incompatible image. All other project IDs
use their original active-slot `oplusstanvbk_a/b` partition.

The selection is private to RMTFS: no global partition symlinks, loop devices,
NV writes, root-module scripts, or files under `/data` are needed. The helper
opens its backing read-only and shadows modem writes in RAM. The original
phone's feature record and hardware metadata are still used unchanged.

KTM also needs its stock-compatible virtual OEM metadata sector at offset
20 MiB. The helper supplies that sector from a private RAM file rather than
appending anything to either backing image or physical partition.

The bundled blob is proprietary and was supplied by the user's installed
`fix-signal-oneplus13` v4.1 module (author Fly). See `firmware/23821/NOTICE`.
The image remains proprietary; the license of the surrounding provider source
does not apply to it. Its provenance and original notice are retained separately.

## Verification

Host tests: `tests/sim_test.cpp`, `tests/modem_features_test.cpp`,
`tests/oem_miscinfo_test.cpp`, `tests/nv_override_test.cpp`, and
`tests/readonly_guard_test.c`. The file-open guard covers both normal libc
calls and Bionic's fortified `__open_2` / `__openat_2` entry points; the latter
must not bypass read-only enforcement or the private NV-path selection.
Live testing of the RAM-only CN overlay restored a valid DMS IMEI, SIM-ready
state, automatic selection of SIM 1, and Telia 4G data. The user confirmed it
worked. The integrated selector also connected successfully using the bundled
image, with valid original identity and two successful cellular-interface
ping replies. A newly built and flashed recovery then reconnected to Telia 4G
after a fresh boot using the packaged helper, without temporary NV aliases or
helper bind mounts.
Use the init-managed `aera-cellular` service, not an ADB-background daemon.
