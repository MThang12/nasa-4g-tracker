#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys
import os
from time import *
from shutil import copyfile

if __name__ == "__main__":
    if len(sys.argv) < 2:
        # print('USAGE: buildConfig inputfile outputfile')
        exit(-1)

    inFilePath = sys.argv[1]

    definitions = ''

    with open(inFilePath,'r') as inFd:
        lines = inFd.readlines()
        for line in lines:
            line = line.strip()
            if len(line) <= 0 or '#' == line[0] or '/' == line[0] or '=' not in line:
                continue
            items = line.split('=')
            if 'y' == items[-1].strip() or 'Y' == items[-1].strip():
                definitions += '-D' + items[0].strip() + '=ON '

    definitions = definitions.strip()
    print(definitions)
