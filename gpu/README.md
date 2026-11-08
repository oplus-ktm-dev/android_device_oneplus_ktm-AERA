# Ktm GPU recovery prebuilts

These proprietary ARM64 Adreno userspace libraries, kernel module, and Gen8
firmware were extracted from a OnePlus 13 stock OTA by the owner
(`MIO-KITCHEN/OP13`) and are reused on the Ace 6: both devices run the same
SM8750/`sun` platform and Adreno 830 backend. They stay in the Ktm device tree
and must be validated against the Ace 6 stock GPU image on first recovery boot.

`msm_kgsl.ko` is ABI-matched to the recovery kernel: the OTA and recovery copies
of `msm_drm.ko` have the same SHA-256. All KGSL hard and soft dependencies are
already loaded by the stock recovery module set. `BoardConfig.mk` requests KGSL
through `AERA_LOAD_VENDOR_MODULES`, using the same dependency-aware loader as the
rest of Ktm's vendor modules.

The userspace closure includes only EGL/GLES2 and the mapper/gralloc libraries
needed by Qualcomm's Android EGL subdriver. Vulkan, SurfaceFlinger, and the rest
of the Android graphics services are intentionally excluded. The software
renderer remains the fail-safe path.

The device-tree copy of `libadreno_utils.so` has its unused
`DT_NEEDED: libnativewindow.so` entry removed with `patchelf`. The extracted
binary has no undefined native-window symbols. No code or data sections are
otherwise changed.

The validation probe successfully created an EGL 1.5 pbuffer, rendered with
`Qualcomm / Adreno (TM) 830`, and read back the expected RGBA pixel. This proves
the kernel, firmware, EGL, GLES2, and shader compiler path before it is allowed
to drive the recovery UI.
