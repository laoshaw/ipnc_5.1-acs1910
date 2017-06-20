## THIS IS A GENERATED FILE -- DO NOT EDIT
.configuro: linker.cmd

linker.cmd: \
  package/cfg/alg_server_xv5T.ov5T \
  package/cfg/alg_server_xv5T.xdl
	$(SED) 's"^\"\(package/cfg/alg_server_xv5Tcfg.cmd\)\"$""\"/home/pamsimochen/arable_land/ipnc_5.1-acs1910/ipnc_rdk/av_capture/framework/alg/src/alg_config/\1\""' package/cfg/alg_server_xv5T.xdl > $@
