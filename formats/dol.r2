e asm.arch=ppc
e cfg.bigendian=true
pf.dol_header [7]x[11]x[7]x[11]x[7]x[11]xxxx[7]: text_off data_off text_addr data_addr text_size data_size bss_addr bss_size entry

(parrmem arr mem addr, $0[$1] @ $2 | cut -f2 -d= | tr -d [:blank:][])
(pdhdrarrmem sec, .(parrmem pf.dol_header.$0 `?vi $$` 0))
(pdhdrvar var, pf.dol_header.$0 @ 0 | cut -f2 -d=)
(pdseci sec, .(pdhdrarrmem $0_off), .(pdhdrarrmem $0_addr), .(pdhdrarrmem $0_size), ?e $0`?vi $$`)
(pdseci2 sec, .(pdseci $0) | paste -d' ' - - - -)
(Sdolsec sec, S .(pdseci2 $0))
(Sdolsecs sec max, .(Sdolsec $0) @@=`?s 0 $1`)
(Sdol tmax dmax, .(Sdolsecs text $0), .(Sdolsecs data $1))
