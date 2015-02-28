#!/usr/local/env python


import sys, re


def color2b(color):
    color = color.replace("#","").strip()
    r = int(eval("0x%s" % color[0:2])) * 1000 / 255.0
    g = int(eval("0x%s" % color[2:4])) * 1000 / 255.0
    b = int(eval("0x%s" % color[4:6])) * 1000 / 255.0

    return "%s, %s, %s" %( int(r),int(g),int(b) )

if len(sys.argv) != 2:
    print "Usage: %s file" % sys.argv[0]
    exit()


for i in open(sys.argv[1]):
    i = i.strip()
    l = i.split(':')
    m = re.search("color([0-9]+)", l[0])
    if m:
        print "init_color( %s, %s);" % (m.group(1), color2b(l[1]))
    else:
        continue


