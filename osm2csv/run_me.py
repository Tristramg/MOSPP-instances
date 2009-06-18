#!/usr/bin/env python
import fileinput
import elevation
import os
import os.path
import sys
import subprocess

if len(sys.argv) != 2:
    print "Usage: %s database.osm" % sys.argv[0]
    sys.exit(2)

if not os.path.exists("./osm2csv"):
    print "osm2mumoro executable not found"
    sys.exit(1)

if not os.path.exists(sys.argv[1]):
    print "File doesn't exist: %s" % sys.argv[1]
    sys.exit(1)

retcode = subprocess.call(["./osm2csv", sys.argv[1]])
if retcode != 0 or not os.path.exists("edges.csv") or not os.path.exists("nodes.csv"):
    print "Some error occured during the parsing"
    sys.exit(1)

print "Last step: updating the nodes to include elevation data"

eld = elevation.ElevationData("Eurasia")
file = open("nodes.csv", "r")
out_file = open("nodes2.csv", "w")
for line in file:
    elements = line.split(',')
    if not elements[1] == '"longitude"':
        lon = float(elements[1])
        lat = float(elements[2])
        alt = eld.altitude(lat, lon)
        out_file.write(line.rstrip() + str(alt)+ "\n")
    else:
        out_file.write(line)


out_file.close()
os.rename("nodes2.csv", "nodes.csv")

print "Everything seems to have gone fine :-)"
