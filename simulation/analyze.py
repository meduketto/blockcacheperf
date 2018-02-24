#!/usr/bin/python

import sys

nr_reads = 0
nr_writes = 0
read_set = set()
write_set = set()
low_sector = 2**62
high_sector = 0

for line in file(sys.argv[1]):
    is_write = False
    sector = None
    if line.startswith("r "):
        sector = int(line.split(" ", 1)[1])
    elif line.startswith("w "):
        is_write = True
        sector = int(line.split(" ", 1)[1])
    else:
        continue

    if sector == -1:
        continue

    low_sector = min(low_sector, sector)
    high_sector = max(high_sector, sector)

    if is_write:
        nr_writes += 1
        write_set.add(sector)
    else:
        nr_reads += 1
        read_set.add(sector)

stats = (
    nr_reads,
    nr_writes,
    len(read_set),
    len(write_set)
)
print "%s reads, %s writes, %s unique reads, %s unique writes of sectors." % stats

print "low sector %s, high sector %s." % (low_sector, high_sector)
