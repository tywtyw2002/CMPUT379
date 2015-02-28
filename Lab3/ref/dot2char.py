#!/usr/local/env python


import sys

if len(sys.argv) != 2:
    print "%s file" % sys.argv[0]
    exit()

sys.stdout.write ("{ ")
for i in open(sys.argv[1]):
    #print i
    i = i.strip()

    l = i.split(',')
    sys.stdout.write ("{ ")
    for x in l:
        if x == "x":
            sys.stdout.write ("1")
        else:
            sys.stdout.write ("0")
        sys.stdout.write (", ")
    sys.stdout.write ("}\n")
sys.stdout.write ("}")