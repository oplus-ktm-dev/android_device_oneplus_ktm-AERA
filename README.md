# AERA Recovery Project device tree for OnePlus Ace 6

Device codename: `ktm`

Platform: Qualcomm SM8750
Recovery partition limit: 100 MiB

This tree keeps the history of the original OnePlus device tree (OnePlus Ace 6 stock)
while carrying the OnePlus Ace 6 integration for AERA Recovery Project R1.0.

## Hardware support

- Display and touch (even in fastbootd)
- Decryption, flashing, backup & restore
- Wi-Fi
- Haptics and flashlight
- Adreno 830 recovery rendering with matching Gen8 firmware
- Qualcomm AGM/PAL audio using the installed OnePlus Ace 6 stock partitions
- Mobile data (CN NV backing) via `aera-cellular-provider`

The proprietary graphics, audio, and cellular NV files in this repository
target the SM8750 platform used by `ktm`. They must be validated against
the matching OnePlus Ace 6 stock firmware before release.

## Build

```sh
cd ~/android/AERA_16.0
source build/envsetup.sh
lunch twrp_ktm-eng
mka adbd recoveryimage
```

The output is written to `out/target/product/ktm/`.
