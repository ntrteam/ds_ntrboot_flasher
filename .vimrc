let g:syntastic_cpp_compiler = $DEVKITARM.'/bin/arm-none-eabi-g++'
let g:syntastic_cpp_compiler_options = "-DARM9"
let g:syntastic_cpp_include_dirs = [".", "include", "external", $DEVKITPRO."/libnds/include"]
set colorcolumn=80

