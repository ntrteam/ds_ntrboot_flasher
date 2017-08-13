import os.path
import glob

strings = [
    '#ifndef BINARIES_H\n',
    '#define BINARIES_H\n',
    '#ifdef __cplusplus\n',
    'extern "C" {\n',
    '#endif\n',
]

for fn in glob.glob('binaries/*'):
    basename = os.path.basename(fn)
    var_name = basename.replace('.', '_')
    with open(fn, 'rb') as r:
        strings.append('uint8_t %s[] = {\n' % var_name);
        buf = r.read()

        for idx, x in enumerate(buf):
            strings.append('0x%02X, ' % ord(x))
            if (idx and ((idx + 1) % 16) == 0):
                strings.append('\n')
        strings.append('};\n');
        strings.append('uint32_t %s_size = 0x%X;\n' % (var_name, len(buf)))

strings += [
    '#ifdef __cplusplus\n',
    '}\n',
    '#endif\n',
    '#endif\n',
]

with open('include/binaries.h', 'wb') as w:
    w.write(''.join(strings))

