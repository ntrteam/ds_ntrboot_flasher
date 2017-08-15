## NDS version NTRBOOTHAX flasher
Only tested NDSL + AK2I & R4i Gold

DO NOT USE THIS ON NDSI

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
* And other talented hackers

[normmatt]: https://github.com/Normmatt
[sciresm]: https://twitter.com/SciresM
[kitling]: https://github.com/kitling
[hedgeberg]: https://github.com/hedgeberg
