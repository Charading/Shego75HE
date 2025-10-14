import fileinput
import sys

for line in fileinput.input(inplace=True, backup='.bak'):
    sys.stdout.write(line.replace("+3.3V", "+3V3"))
