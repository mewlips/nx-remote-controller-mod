# nx-remote-controller-mod

Remote Controller Mod for NX1/NX500/NX300

## Project Home

https://mewlips.github.io/nx-remote-controller-mod

## API Documenation

https://github.com/mewlips/nx-remote-controller-mod/wiki/API-Documentation

## License

AGPL-3.0

## Emergency Recovery (Samsung NX500)

If a mod installation or `nx-on-wake` / `systemctl rescue` issue soft-bricks a **Samsung NX500**, you can enter the built-in firmware recovery mode without disassembly (verified on NX500; **not verified on NX1** or NX300):

1. Place official, unmodified `nx500.bin` on the root of a clean FAT32 SD card and insert it.
2. Remove the battery, flip the screen fully upward into the selfie position, and reinsert the battery.
3. Hold **MOBILE (Wi-Fi) + AEL + full Shutter**, then power **ON**.
4. When **Recovery Mode** appears, hold the shutter button to flash `nx500.bin` (do not interrupt power until `Update Finish`).
