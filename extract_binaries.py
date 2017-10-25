import os.path
import glob

def chunks(b, n=16):
    return (b[i : i + n] for i in xrange(0, len(b), n))

header = [
    '#ifndef BINARIES_H',
    '#define BINARIES_H',
    '#ifdef __cplusplus',
    'extern "C" {',
    '#endif',
    '#include <stdint.h>',

]

source = [
    '#include "binaries.h"',
]

for fn in glob.glob('binaries/*'):
    basename = os.path.basename(fn)
    var_name = basename.replace('.', '_')
    with open(fn, 'rb') as r:
        header.append('extern uint8_t %s[];' % var_name)
        source.append('uint8_t %s[] = {' % var_name)
        buf = r.read()

        for chunk in chunks(buf):
            source.append(' '.join((('0x%02X,' % ord(x)) for x in chunk)))
        source.append('};');
        header.append('extern uint32_t %s_size;' % var_name)
        source.append('uint32_t %s_size = 0x%X;' % (var_name, len(buf)))

header += [
    '#ifdef __cplusplus',
    '}',
    '#endif',
    '#endif',
]

with open('include/binaries.h', 'wb') as w:
    w.write('\n'.join(header))

with open('source/binaries.cpp', 'wb') as w:
    w.write('\n'.join(source))
