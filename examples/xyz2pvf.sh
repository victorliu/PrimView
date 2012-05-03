#!/bin/sh

# Example of generate a PVF from an XYZ

echo 'PVF(1)'
awk '{print "p{",$1,",",$2,",",$3,"}b{-1,0.1}"}' $1
