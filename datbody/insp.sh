#!/bin/sh
datasz=$(datbody -h $1 | grep datasz | awk --non-decimal-data '{printf ("%x\n", ("0x"$3)/4)}')
inspawk='
BEGIN{p="";o="";po="";} 
{ 
    if(NF==1) {
        po=o
        o=$1
    } else if($2~/^[A-Z]/) { 
        print "--"$2"--" 
    } else if(NR==1) {
        p1=$1;p2=$2; p3=$3
    } else { 
        printf "%s ", p1
        if("0x"p1=="0x"o || "0x"p1=="0x"po) {
            printf ":%8s:", p2
        } else {
            printf " %8s  %s", p2, p3
        }
        print ""
        p1=$1;p2=$2;p3=$3
    }
}'
ninspawk='{ print $0 }'
(datbody -r $1; (datbody -b $1; datrelt $1; datinsp $1 0 $datasz) | sort -u | awk "${inspawk}") | less

