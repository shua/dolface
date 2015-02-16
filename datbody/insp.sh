#!/bin/sh

(./datbody -o $1; ./datinsp $1 0 $2 4) | sort -u | awk 'BEGIN{p=""} { if($2 == "Unknown") { print "--"$2"--" } else { if($2 != "") {print p1" "p2;p1=$1;p2=$2}}}'
