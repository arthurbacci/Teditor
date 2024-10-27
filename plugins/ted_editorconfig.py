#!/usr/bin/env python3

from os.path import abspath
from sys import argv
from sys import exit

from editorconfig import get_properties

if len(argv) != 3:
  exit(102)

try:
  filename = abspath(argv[2])
except:
  exit(103)

try:
  props = get_properties(filename)
except:
  exit(104)

try:
  ret = props[argv[1]]
except:
  exit(101)


match argv[1]:
  case 'indent_style':
    if ret == 'tab':
      exit(1)
    elif ret == 'space':
      exit(2)
  case 'indent_size':
    if ret == 'tab':
      exit(0)
    elif int(ret) < 100:
      exit(int(ret))
    else:
      exit(99)
  case 'tab_width':
    if int(ret) < 100:
      exit(int(ret))
    else:
      exit(99)



    

