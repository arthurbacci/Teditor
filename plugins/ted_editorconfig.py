#!/usr/bin/env python3

from os.path import abspath
from sys import argv
from sys import exit

from editorconfig import get_properties

if len(argv) != 3:
    exit(2)

try:
    filename = abspath(argv[2])
except:
    exit(3)

try:
    props = get_properties(filename)
except:
    exit(4)

try:
    ret = props[argv[1]]
except:
    exit(1)

print(ret)



    

