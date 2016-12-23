import sys

with open(sys.argv[1], 'rb') as r:
    d = r.read()[0x2000:0x3000]

    print 'uint8_t orig_header[] = {'
    for idx, x in enumerate(d):
        print ('0x%02X,' % ord(x)),
        if (idx and ((idx + 1) % 16) == 0):
            print
    print '};'

