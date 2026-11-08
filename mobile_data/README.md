# Ktm mobile data

The provider and guarded modem helpers are opt-in through
`AERA_ENABLE_MOBILE_DATA=1`. The implementation is adapted from Infiniti and
Lighthouse; it discovers the modem remoteproc and QRTR service instances at
runtime instead of depending on their device-specific numbering.

## OEM modem feature map

Offsets were derived from `CPH2653_16.0.2.402(EX01)` stock ODM `mdm_feature`
(project 23821, OnePlus 13 family) and **verified on Ace 6** by reading
`oplusreserve1` from a rooted PLQ110 (project 24851) on 2026-11-08:

- Stock executable: `/odm/bin/hw/mdm_feature`
- SHA-256 of the CPH2653 build: `5d80ade63f3c38c825d8f27968e9edb8e8c60e642f8a6b8208983c09e9eaf0b1`
- SHA-256 of the Ace 6 ODM build (`OnePlus/PLQ110/OP6113L1:16/BP2A.250605.015`):
  `ee33e677334c4ed673cca2d8e70b15de1b3d045f59a3feb9d4f4af96e8f96750`
- The modem feature XML record sits at `0x493000` in `oplusreserve1`
  (UFS, `/dev/block/sda` present), matching the UFS read branch;
  `0x46a000` is unused (zeros) on Ace 6.
- Its `read_reserve1_mdmfeature` routine at `0x4b80` reads `0x3000` bytes
  from `oplusreserve1`, using offset `0x493000` for UFS (`/dev/block/sda`)
  or `0x46a000` otherwise. These match the Infiniti/Lighthouse layout.
- Read-only inspection on the connected Ktm running that build confirmed
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

## Static NV

Ace 6 (project `24851`, the only project this tree targets) has no bundled
static-NV image: the helper opens the device's own active-slot
`oplusstanvbk_a/b` partition read-only (verified on the rooted PLQ110:
20 MiB each, `FEMNVBK` header). The project ID is still read from
`/proc/oplusVersion/prjName` and logged. The legacy OnePlus 13 CN image
(project 23821) was removed with this commit; its provenance (the
`fix-signal-oneplus13` v4.1 module, author Fly) remains in the git history
at the removed `firmware/23821/` path.

The selection is private to RMTFS: no global partition symlinks, loop devices,
NV writes, root-module scripts, or files under `/data` are needed. The helper
opens its backing read-only and shadows modem writes in RAM. The original
phone's feature record and hardware metadata are still used unchanged.

Ktm also needs its stock-compatible virtual OEM metadata sector at offset
20 MiB. The helper supplies that sector from a private RAM file rather than
appending anything to either backing image or physical partition.

## Verification

Host tests: `tests/sim_test.cpp`, `tests/modem_features_test.cpp`,
`tests/oem_miscinfo_test.cpp`, and `tests/readonly_guard_test.c`. The file-open guard covers both normal libc
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
