#!/bin/sh
datasz=$(datbody -h $1 | grep datasz | awk --non-decimal-data '{printf ("%x\n", ("0x"$3)/4)}')
(datbody -b $1; datinsp $1 0 $datasz 4) | sort -u | awk 'BEGIN{p=""} { if($2~/^[A-Z]/) { print "--"$2"--" } else { if($2 != "") {print p1" "p2;p1=$1;p2=$2}}}'
