## NDS version NTRBOOTHAX flasher
Only tested NDSL + AK2I

DO NOT USE THIS ON NDSI

### Known issues
* Probably cannot use with non AK2I cart.

  We should make NDSI mode instead NDS. But that mode cannot use cartridge
  swap, and probably lost flashcart feature until someone provide restore
  image.

* Removed restore feature, I don't know reason, flasher occur cartridge brick.

  But, AK2I will work as flashcart whenever flashed or not.
  So, just feature blocked.

### Build
1. copy these 4 binaries to `binaries` directory.
  - `blowfish_dev.bin`
  - `blowfish_retail.bin`
  - `boot9strap_ntr_dev.firm`
  - `boot9strap_ntr.firm`
2. `python extract_binaries.py`
3. then `make`

### Credits
* [Normmatt][normmatt] - Original implement 3DS version ntrcardhax flasher
* [SciresM][sciresm] - Parent of `boot9strap`
* [kitling][kitling] - Made `flashcart_core`
* [hedgeberg][hedgeberg] - RE Cartridge
