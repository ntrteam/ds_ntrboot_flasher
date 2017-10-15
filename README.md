## NDS version NTRBOOTHAX flasher
Only tested NDSL + AK2I & R4i Gold

### Build
1. copy these 4 binaries to `binaries` directory.
  - `blowfish_dev.bin`
  - `blowfish_retail.bin`
  - `boot9strap_ntr_dev.firm`
  - `boot9strap_ntr.firm`
2. `python extract_binaries.py`
3. then `make`

#### Special `make` options
* `NDSI_MODE=1`
  - build nds compatible mode
  - remove cart swap and load & restore command
* `DEBUG_DUMP=1`
  - print first 16bytes at load
* `DEBUG_DUMP=2`
  - print all dumps
* `DEBUG_PRINT=1`
  - print flashcart_core's log message

### Credits
* [Normmatt][normmatt] - Original implement 3DS version ntrcardhax flasher
* [SciresM][sciresm] - Parent of `boot9strap`
* [kitling][kitling] - Made `flashcart_core`
* [hedgeberg][hedgeberg] - RE Cartridge
* [HandsomeMatt][HandsomeMatt] - RE Cartridge
* And other talented hackers

[normmatt]: https://github.com/Normmatt
[sciresm]: https://twitter.com/SciresM
[kitling]: https://github.com/kitling
[hedgeberg]: https://github.com/hedgeberg
[HandsomeMatt]: https://github.com/HandsomeMatt
