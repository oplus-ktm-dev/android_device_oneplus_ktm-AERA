# OnePlus Ace 6 ktm Android device tree

## Working

- [X] Display
- [X] Touch (Even in FastbootD)
- [X] Decryption
- [X] Flashing
- [X] Backup & Restore
- [X] MTP/OTG Storage
- [X] ADB/FastbootD
- [X] Factory Reset
- [X] Vibrator
- [X] Display & Vibration Settings

## Not working
- [ ] ????????

# How To Build

### Clone & Sync Source
```
mkdir -p ~/android/OrangeFox_14
cd ~/android/OrangeFox_14
git clone https://gitlab.com/OrangeFox/sync.git
cd sync
./orangefox_sync.sh --branch 14.1 --path ~/android/fox_14.1
```
### Clone Device-tree
```
cd ~/android/fox_14.1/device
mkdir -p oneplus
cd oneplus
git clone --branch ktm https://github.com/oplus-ktm-dev/android_device_oneplus_ktm-AERA.git ktm
```
### BUILD!
```
cd ~/android/fox_14.1
source build/envsetup.sh
lunch twrp_ktm-eng
mka adbd recoveryimage
```
