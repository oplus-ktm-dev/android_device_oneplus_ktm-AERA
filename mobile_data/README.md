# KTM mobile data

The provider and guarded modem helpers are opt-in through
`AERA_ENABLE_MOBILE_DATA=1`. The implementation is adapted from Infiniti and
Lighthouse; it discovers the modem remoteproc and QRTR service instances at
runtime instead of depending on their device-specific numbering.

## OEM modem feature map

Checked against OnePlus 13 stock OTA `CPH2653_16.0.2.402(EX01)`. The current
`16.0.10.x` builds of CPH2653 and PLQ110 ship the same `mdm_feature` binary
(SHA-256 `5e95d0856243c8915200bf0c88db3ec5a9ab5752b4f716226d380b3cac8650bf`),
and the UFS record was confirmed at `0x493000` on PLQ110 hardware:

- Stock executable: `/odm/bin/hw/mdm_feature`
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

## Static NV partition

KTM (PLQ110, project `24851`) uses the device's own active-slot
`oplusstanvbk_<slot>` partition (20 MiB `OEMNVBK` image; both slots confirmed
in sync on hardware). The provider reads the project ID from
`/proc/oplusVersion/prjName` and the slot from `ro.boot.slot_suffix`, then maps
the stock `oem_stanvbk` alias to the read-only `oplusstanvbk_<slot>` partition.
The `oem_dycnvbk` alias maps to the read-only, unslotted `oplusdycnvbk`
partition. No NV image is bundled: an earlier OnePlus 13
(project `23821`) workaround shipped a proprietary CN static-NV image, retired
in this tree.

The selection is private to RMTFS: no global partition symlinks, loop devices,
NV writes, root-module scripts, or files under `/data` are needed. The helper
opens its backing read-only and shadows modem writes in RAM. The original
phone's feature record and hardware metadata are still used unchanged.

KTM also needs its stock-compatible virtual OEM metadata sector at offset
20 MiB. The helper supplies that sector from a private RAM file rather than
appending anything to either backing image or physical partition.

## Verification

Host tests: `tests/sim_test.cpp`, `tests/modem_features_test.cpp`,
`tests/oem_miscinfo_test.cpp`, and `tests/readonly_guard_test.c`. The file-open guard covers both normal libc
calls and Bionic's fortified `__open_2` / `__openat_2` entry points; the latter
must not bypass read-only enforcement or the private NV-path selection.
On the original OnePlus 13 target, live testing of the RAM-only CN overlay
restored a valid DMS IMEI, SIM-ready state, automatic selection of SIM 1, and
Telia 4G data. A newly built and flashed recovery reconnected to Telia 4G
after a fresh boot using the packaged helper, without temporary NV aliases or
helper bind mounts. On KTM hardware the active-slot `oplusstanvbk` partitions
and the UFS modem-feature record were verified read-only; no NV writes were
performed.
Use the init-managed `aera-cellular` service, not an ADB-background daemon.
