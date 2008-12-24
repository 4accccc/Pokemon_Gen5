# area_map_tex.rbÇ…ÇÊÇËê∂ê¨Ç≥ÇÍÇ‹ÇµÇΩ
nsbtx_files/tex_set_map.nsbtx: src_imd_files/area00_tex_set.imd
	@echo src_imd_files/area00_tex_set.imd Å® nsbtx_files/tex_set_map.nsbtx
	@$(COPY) src_imd_files/area00_tex_set.imd marged_imd_files/tex_set_map.imd
	@$(G3DCVTR) marged_imd_files/tex_set_map.imd -o nsbtx_files/tex_set_map.nsbtx -etex

nsbtx_files/tex_set_hos1.nsbtx: src_imd_files/area00_tex_set.imd
	@echo src_imd_files/area00_tex_set.imd Å® nsbtx_files/tex_set_hos1.nsbtx
	@$(COPY) src_imd_files/area00_tex_set.imd marged_imd_files/tex_set_hos1.imd
	@$(G3DCVTR) marged_imd_files/tex_set_hos1.imd -o nsbtx_files/tex_set_hos1.nsbtx -etex

nsbtx_files/tex_set_area01.nsbtx: src_imd_files/area00_tex_set.imd src_imd_files/area01_tex_set.imd src_imd_files/tex_griddmy.imd
	@echo src_imd_files/area00_tex_set.imd + src_imd_files/area01_tex_set.imd Å® nsbtx_files/tex_set_area01.nsbtx
	@-rm marged_imd_files/tex_set_area01.imd
	@-$(TEX_MAG) -q -o marged_imd_files/tex_set_area01.imd src_imd_files/area00_tex_set.imd src_imd_files/area01_tex_set.imd src_imd_files/tex_griddmy.imd
	@$(G3DCVTR) marged_imd_files/tex_set_area01.imd -o nsbtx_files/tex_set_area01.nsbtx -etex

nsbtx_files/tex_set_area02.nsbtx: src_imd_files/area00_tex_set.imd src_imd_files/area02_tex_set.imd src_imd_files/tex_griddmy.imd
	@echo src_imd_files/area00_tex_set.imd + src_imd_files/area02_tex_set.imd Å® nsbtx_files/tex_set_area02.nsbtx
	@-rm marged_imd_files/tex_set_area02.imd
	@-$(TEX_MAG) -q -o marged_imd_files/tex_set_area02.imd src_imd_files/area00_tex_set.imd src_imd_files/area02_tex_set.imd src_imd_files/tex_griddmy.imd
	@$(G3DCVTR) marged_imd_files/tex_set_area02.imd -o nsbtx_files/tex_set_area02.nsbtx -etex

nsbtx_files/tex_set_area03.nsbtx: src_imd_files/area00_tex_set.imd
	@echo src_imd_files/area00_tex_set.imd Å® nsbtx_files/tex_set_area03.nsbtx
	@$(COPY) src_imd_files/area00_tex_set.imd marged_imd_files/tex_set_area03.imd
	@$(G3DCVTR) marged_imd_files/tex_set_area03.imd -o nsbtx_files/tex_set_area03.nsbtx -etex

nsbtx_files/tex_set_area04.nsbtx: src_imd_files/area00_tex_set.imd src_imd_files/area04_tex_set.imd src_imd_files/tex_griddmy.imd
	@echo src_imd_files/area00_tex_set.imd + src_imd_files/area04_tex_set.imd Å® nsbtx_files/tex_set_area04.nsbtx
	@-rm marged_imd_files/tex_set_area04.imd
	@-$(TEX_MAG) -q -o marged_imd_files/tex_set_area04.imd src_imd_files/area00_tex_set.imd src_imd_files/area04_tex_set.imd src_imd_files/tex_griddmy.imd
	@$(G3DCVTR) marged_imd_files/tex_set_area04.imd -o nsbtx_files/tex_set_area04.nsbtx -etex

nsbtx_files/tex_set_area05.nsbtx: src_imd_files/area00_tex_set.imd src_imd_files/area05_tex_set.imd src_imd_files/tex_griddmy.imd
	@echo src_imd_files/area00_tex_set.imd + src_imd_files/area05_tex_set.imd Å® nsbtx_files/tex_set_area05.nsbtx
	@-rm marged_imd_files/tex_set_area05.imd
	@-$(TEX_MAG) -q -o marged_imd_files/tex_set_area05.imd src_imd_files/area00_tex_set.imd src_imd_files/area05_tex_set.imd src_imd_files/tex_griddmy.imd
	@$(G3DCVTR) marged_imd_files/tex_set_area05.imd -o nsbtx_files/tex_set_area05.nsbtx -etex

nsbtx_files/tex_set_area06.nsbtx: src_imd_files/area00_tex_set.imd src_imd_files/area06_tex_set.imd src_imd_files/tex_griddmy.imd
	@echo src_imd_files/area00_tex_set.imd + src_imd_files/area06_tex_set.imd Å® nsbtx_files/tex_set_area06.nsbtx
	@-rm marged_imd_files/tex_set_area06.imd
	@-$(TEX_MAG) -q -o marged_imd_files/tex_set_area06.imd src_imd_files/area00_tex_set.imd src_imd_files/area06_tex_set.imd src_imd_files/tex_griddmy.imd
	@$(G3DCVTR) marged_imd_files/tex_set_area06.imd -o nsbtx_files/tex_set_area06.nsbtx -etex

nsbtx_files/tex_set_area07.nsbtx: src_imd_files/area00_tex_set.imd src_imd_files/area07_tex_set.imd src_imd_files/tex_griddmy.imd
	@echo src_imd_files/area00_tex_set.imd + src_imd_files/area07_tex_set.imd Å® nsbtx_files/tex_set_area07.nsbtx
	@-rm marged_imd_files/tex_set_area07.imd
	@-$(TEX_MAG) -q -o marged_imd_files/tex_set_area07.imd src_imd_files/area00_tex_set.imd src_imd_files/area07_tex_set.imd src_imd_files/tex_griddmy.imd
	@$(G3DCVTR) marged_imd_files/tex_set_area07.imd -o nsbtx_files/tex_set_area07.nsbtx -etex

nsbtx_files/tex_set_area08.nsbtx: src_imd_files/area00_tex_set.imd src_imd_files/area08_tex_set.imd src_imd_files/tex_griddmy.imd
	@echo src_imd_files/area00_tex_set.imd + src_imd_files/area08_tex_set.imd Å® nsbtx_files/tex_set_area08.nsbtx
	@-rm marged_imd_files/tex_set_area08.imd
	@-$(TEX_MAG) -q -o marged_imd_files/tex_set_area08.imd src_imd_files/area00_tex_set.imd src_imd_files/area08_tex_set.imd src_imd_files/tex_griddmy.imd
	@$(G3DCVTR) marged_imd_files/tex_set_area08.imd -o nsbtx_files/tex_set_area08.nsbtx -etex

nsbtx_files/tex_set_area09.nsbtx: src_imd_files/area00_tex_set.imd src_imd_files/area09_tex_set.imd src_imd_files/tex_griddmy.imd
	@echo src_imd_files/area00_tex_set.imd + src_imd_files/area09_tex_set.imd Å® nsbtx_files/tex_set_area09.nsbtx
	@-rm marged_imd_files/tex_set_area09.imd
	@-$(TEX_MAG) -q -o marged_imd_files/tex_set_area09.imd src_imd_files/area00_tex_set.imd src_imd_files/area09_tex_set.imd src_imd_files/tex_griddmy.imd
	@$(G3DCVTR) marged_imd_files/tex_set_area09.imd -o nsbtx_files/tex_set_area09.nsbtx -etex

nsbtx_files/tex_set_area10.nsbtx: src_imd_files/dpgs_tex_set.imd src_imd_files/area10_tex_set.imd src_imd_files/tex_griddmy.imd
	@echo src_imd_files/dpgs_tex_set.imd + src_imd_files/area10_tex_set.imd Å® nsbtx_files/tex_set_area10.nsbtx
	@-rm marged_imd_files/tex_set_area10.imd
	@-$(TEX_MAG) -q -o marged_imd_files/tex_set_area10.imd src_imd_files/dpgs_tex_set.imd src_imd_files/area10_tex_set.imd src_imd_files/tex_griddmy.imd
	@$(G3DCVTR) marged_imd_files/tex_set_area10.imd -o nsbtx_files/tex_set_area10.nsbtx -etex

nsbtx_files/tex_set_room01.nsbtx: src_imd_files/room01_tex_set.imd
	@echo src_imd_files/room01_tex_set.imd Å® nsbtx_files/tex_set_room01.nsbtx
	@$(COPY) src_imd_files/room01_tex_set.imd marged_imd_files/tex_set_room01.imd
	@$(G3DCVTR) marged_imd_files/tex_set_room01.imd -o nsbtx_files/tex_set_room01.nsbtx -etex

nsbtx_files/tex_set_room02.nsbtx: src_imd_files/room02_tex_set.imd
	@echo src_imd_files/room02_tex_set.imd Å® nsbtx_files/tex_set_room02.nsbtx
	@$(COPY) src_imd_files/room02_tex_set.imd marged_imd_files/tex_set_room02.imd
	@$(G3DCVTR) marged_imd_files/tex_set_room02.imd -o nsbtx_files/tex_set_room02.nsbtx -etex

nsbtx_files/tex_set_room03.nsbtx: src_imd_files/room03_tex_set.imd
	@echo src_imd_files/room03_tex_set.imd Å® nsbtx_files/tex_set_room03.nsbtx
	@$(COPY) src_imd_files/room03_tex_set.imd marged_imd_files/tex_set_room03.imd
	@$(G3DCVTR) marged_imd_files/tex_set_room03.imd -o nsbtx_files/tex_set_room03.nsbtx -etex

nsbtx_files/tex_set_room04.nsbtx: src_imd_files/room04_tex_set.imd
	@echo src_imd_files/room04_tex_set.imd Å® nsbtx_files/tex_set_room04.nsbtx
	@$(COPY) src_imd_files/room04_tex_set.imd marged_imd_files/tex_set_room04.imd
	@$(G3DCVTR) marged_imd_files/tex_set_room04.imd -o nsbtx_files/tex_set_room04.nsbtx -etex

nsbtx_files/tex_set_room05.nsbtx: src_imd_files/room05_tex_set.imd
	@echo src_imd_files/room05_tex_set.imd Å® nsbtx_files/tex_set_room05.nsbtx
	@$(COPY) src_imd_files/room05_tex_set.imd marged_imd_files/tex_set_room05.imd
	@$(G3DCVTR) marged_imd_files/tex_set_room05.imd -o nsbtx_files/tex_set_room05.nsbtx -etex

nsbtx_files/tex_set_room06.nsbtx: src_imd_files/room06_tex_set.imd
	@echo src_imd_files/room06_tex_set.imd Å® nsbtx_files/tex_set_room06.nsbtx
	@$(COPY) src_imd_files/room06_tex_set.imd marged_imd_files/tex_set_room06.imd
	@$(G3DCVTR) marged_imd_files/tex_set_room06.imd -o nsbtx_files/tex_set_room06.nsbtx -etex

nsbtx_files/tex_set_room07.nsbtx: src_imd_files/room07_tex_set.imd
	@echo src_imd_files/room07_tex_set.imd Å® nsbtx_files/tex_set_room07.nsbtx
	@$(COPY) src_imd_files/room07_tex_set.imd marged_imd_files/tex_set_room07.imd
	@$(G3DCVTR) marged_imd_files/tex_set_room07.imd -o nsbtx_files/tex_set_room07.nsbtx -etex

nsbtx_files/tex_set_room08.nsbtx: src_imd_files/room08_tex_set.imd
	@echo src_imd_files/room08_tex_set.imd Å® nsbtx_files/tex_set_room08.nsbtx
	@$(COPY) src_imd_files/room08_tex_set.imd marged_imd_files/tex_set_room08.imd
	@$(G3DCVTR) marged_imd_files/tex_set_room08.imd -o nsbtx_files/tex_set_room08.nsbtx -etex

nsbtx_files/tex_set_room09.nsbtx: src_imd_files/room09_tex_set.imd
	@echo src_imd_files/room09_tex_set.imd Å® nsbtx_files/tex_set_room09.nsbtx
	@$(COPY) src_imd_files/room09_tex_set.imd marged_imd_files/tex_set_room09.imd
	@$(G3DCVTR) marged_imd_files/tex_set_room09.imd -o nsbtx_files/tex_set_room09.nsbtx -etex

nsbtx_files/tex_set_room10.nsbtx: src_imd_files/room10_tex_set.imd
	@echo src_imd_files/room10_tex_set.imd Å® nsbtx_files/tex_set_room10.nsbtx
	@$(COPY) src_imd_files/room10_tex_set.imd marged_imd_files/tex_set_room10.imd
	@$(G3DCVTR) marged_imd_files/tex_set_room10.imd -o nsbtx_files/tex_set_room10.nsbtx -etex

nsbtx_files/tex_set_dun01.nsbtx: src_imd_files/dun01_tex_set.imd src_imd_files/dun01sub_tex_set.imd src_imd_files/tex_griddmy.imd
	@echo src_imd_files/dun01_tex_set.imd + src_imd_files/dun01sub_tex_set.imd Å® nsbtx_files/tex_set_dun01.nsbtx
	@-rm marged_imd_files/tex_set_dun01.imd
	@-$(TEX_MAG) -q -o marged_imd_files/tex_set_dun01.imd src_imd_files/dun01_tex_set.imd src_imd_files/dun01sub_tex_set.imd src_imd_files/tex_griddmy.imd
	@$(G3DCVTR) marged_imd_files/tex_set_dun01.imd -o nsbtx_files/tex_set_dun01.nsbtx -etex

nsbtx_files/tex_set_dun02.nsbtx: src_imd_files/area00_tex_set.imd src_imd_files/dun02sub_tex_set.imd src_imd_files/tex_griddmy.imd
	@echo src_imd_files/area00_tex_set.imd + src_imd_files/dun02sub_tex_set.imd Å® nsbtx_files/tex_set_dun02.nsbtx
	@-rm marged_imd_files/tex_set_dun02.imd
	@-$(TEX_MAG) -q -o marged_imd_files/tex_set_dun02.imd src_imd_files/area00_tex_set.imd src_imd_files/dun02sub_tex_set.imd src_imd_files/tex_griddmy.imd
	@$(G3DCVTR) marged_imd_files/tex_set_dun02.imd -o nsbtx_files/tex_set_dun02.nsbtx -etex

nsbtx_files/tex_set_dun03.nsbtx: src_imd_files/dun03_tex_set.imd
	@echo src_imd_files/dun03_tex_set.imd Å® nsbtx_files/tex_set_dun03.nsbtx
	@$(COPY) src_imd_files/dun03_tex_set.imd marged_imd_files/tex_set_dun03.imd
	@$(G3DCVTR) marged_imd_files/tex_set_dun03.imd -o nsbtx_files/tex_set_dun03.nsbtx -etex

nsbtx_files/tex_set_dun04.nsbtx: src_imd_files/dun04_tex_set.imd
	@echo src_imd_files/dun04_tex_set.imd Å® nsbtx_files/tex_set_dun04.nsbtx
	@$(COPY) src_imd_files/dun04_tex_set.imd marged_imd_files/tex_set_dun04.imd
	@$(G3DCVTR) marged_imd_files/tex_set_dun04.imd -o nsbtx_files/tex_set_dun04.nsbtx -etex

nsbtx_files/tex_set_dun05.nsbtx: src_imd_files/dun05_tex_set.imd src_imd_files/dun05_subtex.imd src_imd_files/tex_griddmy.imd
	@echo src_imd_files/dun05_tex_set.imd + src_imd_files/dun05_subtex.imd Å® nsbtx_files/tex_set_dun05.nsbtx
	@-rm marged_imd_files/tex_set_dun05.imd
	@-$(TEX_MAG) -q -o marged_imd_files/tex_set_dun05.imd src_imd_files/dun05_tex_set.imd src_imd_files/dun05_subtex.imd src_imd_files/tex_griddmy.imd
	@$(G3DCVTR) marged_imd_files/tex_set_dun05.imd -o nsbtx_files/tex_set_dun05.nsbtx -etex

nsbtx_files/tex_set_dun06.nsbtx: src_imd_files/area00_tex_set.imd src_imd_files/dun06_tex_set.imd src_imd_files/tex_griddmy.imd
	@echo src_imd_files/area00_tex_set.imd + src_imd_files/dun06_tex_set.imd Å® nsbtx_files/tex_set_dun06.nsbtx
	@-rm marged_imd_files/tex_set_dun06.imd
	@-$(TEX_MAG) -q -o marged_imd_files/tex_set_dun06.imd src_imd_files/area00_tex_set.imd src_imd_files/dun06_tex_set.imd src_imd_files/tex_griddmy.imd
	@$(G3DCVTR) marged_imd_files/tex_set_dun06.imd -o nsbtx_files/tex_set_dun06.nsbtx -etex

nsbtx_files/tex_set_dun07.nsbtx: src_imd_files/dun07_tex_set.imd
	@echo src_imd_files/dun07_tex_set.imd Å® nsbtx_files/tex_set_dun07.nsbtx
	@$(COPY) src_imd_files/dun07_tex_set.imd marged_imd_files/tex_set_dun07.imd
	@$(G3DCVTR) marged_imd_files/tex_set_dun07.imd -o nsbtx_files/tex_set_dun07.nsbtx -etex

nsbtx_files/tex_set_dun08.nsbtx: src_imd_files/dun08_tex_set.imd
	@echo src_imd_files/dun08_tex_set.imd Å® nsbtx_files/tex_set_dun08.nsbtx
	@$(COPY) src_imd_files/dun08_tex_set.imd marged_imd_files/tex_set_dun08.imd
	@$(G3DCVTR) marged_imd_files/tex_set_dun08.imd -o nsbtx_files/tex_set_dun08.nsbtx -etex

nsbtx_files/tex_set_dun09.nsbtx: src_imd_files/dun09_tex_set.imd
	@echo src_imd_files/dun09_tex_set.imd Å® nsbtx_files/tex_set_dun09.nsbtx
	@$(COPY) src_imd_files/dun09_tex_set.imd marged_imd_files/tex_set_dun09.imd
	@$(G3DCVTR) marged_imd_files/tex_set_dun09.imd -o nsbtx_files/tex_set_dun09.nsbtx -etex

nsbtx_files/tex_set_dun10.nsbtx: src_imd_files/dun10_tex_set.imd
	@echo src_imd_files/dun10_tex_set.imd Å® nsbtx_files/tex_set_dun10.nsbtx
	@$(COPY) src_imd_files/dun10_tex_set.imd marged_imd_files/tex_set_dun10.imd
	@$(G3DCVTR) marged_imd_files/tex_set_dun10.imd -o nsbtx_files/tex_set_dun10.nsbtx -etex

