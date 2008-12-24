# area_map_tex.rbÇ…ÇÊÇËê∂ê¨Ç≥ÇÍÇ‹ÇµÇΩ
nsbtx_files/tex_set_map.nsbtx: src_imd_files/area00_tex_set.imd
	@echo src_imd_files/area00_tex_set.imd Å® nsbtx_files/tex_set_map.nsbtx
	@$(COPY) src_imd_files/area00_tex_set.imd marged_imd_files/tex_set_map.imd
	@$(G3DCVTR) marged_imd_files/tex_set_map.imd -o nsbtx_files/tex_set_map.nsbtx -etex

nsbtx_files/tex_set_hos1.nsbtx: src_imd_files/testpc00_00c.imd src_imd_files/testroom00_00c.imd src_imd_files/tex_griddmy.imd
	@echo src_imd_files/testpc00_00c.imd + src_imd_files/testroom00_00c.imd Å® nsbtx_files/tex_set_hos1.nsbtx
	@-rm marged_imd_files/tex_set_hos1.imd
	@-$(TEX_MAG) -q -o marged_imd_files/tex_set_hos1.imd src_imd_files/testpc00_00c.imd src_imd_files/testroom00_00c.imd src_imd_files/tex_griddmy.imd
	@$(G3DCVTR) marged_imd_files/tex_set_hos1.imd -o nsbtx_files/tex_set_hos1.nsbtx -etex

nsbtx_files/tex_set_area01.nsbtx: src_imd_files/tex_spring.imd
	@echo src_imd_files/tex_spring.imd Å® nsbtx_files/tex_set_area01.nsbtx
	@$(COPY) src_imd_files/tex_spring.imd marged_imd_files/tex_set_area01.imd
	@$(G3DCVTR) marged_imd_files/tex_set_area01.imd -o nsbtx_files/tex_set_area01.nsbtx -etex

nsbtx_files/tex_set_area02.nsbtx: src_imd_files/tex_summer.imd
	@echo src_imd_files/tex_summer.imd Å® nsbtx_files/tex_set_area02.nsbtx
	@$(COPY) src_imd_files/tex_summer.imd marged_imd_files/tex_set_area02.imd
	@$(G3DCVTR) marged_imd_files/tex_set_area02.imd -o nsbtx_files/tex_set_area02.nsbtx -etex

nsbtx_files/tex_set_area03.nsbtx: src_imd_files/tex_autumn.imd
	@echo src_imd_files/tex_autumn.imd Å® nsbtx_files/tex_set_area03.nsbtx
	@$(COPY) src_imd_files/tex_autumn.imd marged_imd_files/tex_set_area03.imd
	@$(G3DCVTR) marged_imd_files/tex_set_area03.imd -o nsbtx_files/tex_set_area03.nsbtx -etex

nsbtx_files/tex_set_area04.nsbtx: src_imd_files/tex_winter.imd
	@echo src_imd_files/tex_winter.imd Å® nsbtx_files/tex_set_area04.nsbtx
	@$(COPY) src_imd_files/tex_winter.imd marged_imd_files/tex_set_area04.imd
	@$(G3DCVTR) marged_imd_files/tex_set_area04.imd -o nsbtx_files/tex_set_area04.nsbtx -etex

nsbtx_files/tex_set_area05.nsbtx: src_imd_files/testroom00_00c.imd src_imd_files/tex_spring.imd src_imd_files/tex_griddmy.imd
	@echo src_imd_files/testroom00_00c.imd + src_imd_files/tex_spring.imd Å® nsbtx_files/tex_set_area05.nsbtx
	@-rm marged_imd_files/tex_set_area05.imd
	@-$(TEX_MAG) -q -o marged_imd_files/tex_set_area05.imd src_imd_files/testroom00_00c.imd src_imd_files/tex_spring.imd src_imd_files/tex_griddmy.imd
	@$(G3DCVTR) marged_imd_files/tex_set_area05.imd -o nsbtx_files/tex_set_area05.nsbtx -etex

nsbtx_files/tex_set_room01.nsbtx: src_imd_files/testroom00_00c.imd
	@echo src_imd_files/testroom00_00c.imd Å® nsbtx_files/tex_set_room01.nsbtx
	@$(COPY) src_imd_files/testroom00_00c.imd marged_imd_files/tex_set_room01.imd
	@$(G3DCVTR) marged_imd_files/tex_set_room01.imd -o nsbtx_files/tex_set_room01.nsbtx -etex

nsbtx_files/tex_set_room02.nsbtx: src_imd_files/testroom00_00c.imd
	@echo src_imd_files/testroom00_00c.imd Å® nsbtx_files/tex_set_room02.nsbtx
	@$(COPY) src_imd_files/testroom00_00c.imd marged_imd_files/tex_set_room02.imd
	@$(G3DCVTR) marged_imd_files/tex_set_room02.imd -o nsbtx_files/tex_set_room02.nsbtx -etex

nsbtx_files/tex_set_room03.nsbtx: src_imd_files/testroom00_00c.imd
	@echo src_imd_files/testroom00_00c.imd Å® nsbtx_files/tex_set_room03.nsbtx
	@$(COPY) src_imd_files/testroom00_00c.imd marged_imd_files/tex_set_room03.imd
	@$(G3DCVTR) marged_imd_files/tex_set_room03.imd -o nsbtx_files/tex_set_room03.nsbtx -etex

nsbtx_files/tex_set_room04.nsbtx: src_imd_files/testroom00_00c.imd
	@echo src_imd_files/testroom00_00c.imd Å® nsbtx_files/tex_set_room04.nsbtx
	@$(COPY) src_imd_files/testroom00_00c.imd marged_imd_files/tex_set_room04.imd
	@$(G3DCVTR) marged_imd_files/tex_set_room04.imd -o nsbtx_files/tex_set_room04.nsbtx -etex

nsbtx_files/tex_set_room05.nsbtx: src_imd_files/testroom00_00c.imd
	@echo src_imd_files/testroom00_00c.imd Å® nsbtx_files/tex_set_room05.nsbtx
	@$(COPY) src_imd_files/testroom00_00c.imd marged_imd_files/tex_set_room05.imd
	@$(G3DCVTR) marged_imd_files/tex_set_room05.imd -o nsbtx_files/tex_set_room05.nsbtx -etex

nsbtx_files/tex_set_dun01.nsbtx: src_imd_files/dun01_tex_set.imd src_imd_files/tex_summer.imd src_imd_files/tex_griddmy.imd
	@echo src_imd_files/dun01_tex_set.imd + src_imd_files/tex_summer.imd Å® nsbtx_files/tex_set_dun01.nsbtx
	@-rm marged_imd_files/tex_set_dun01.imd
	@-$(TEX_MAG) -q -o marged_imd_files/tex_set_dun01.imd src_imd_files/dun01_tex_set.imd src_imd_files/tex_summer.imd src_imd_files/tex_griddmy.imd
	@$(G3DCVTR) marged_imd_files/tex_set_dun01.imd -o nsbtx_files/tex_set_dun01.nsbtx -etex

nsbtx_files/tex_set_dun02.nsbtx: src_imd_files/area00_tex_set.imd src_imd_files/tex_autumn.imd src_imd_files/tex_griddmy.imd
	@echo src_imd_files/area00_tex_set.imd + src_imd_files/tex_autumn.imd Å® nsbtx_files/tex_set_dun02.nsbtx
	@-rm marged_imd_files/tex_set_dun02.imd
	@-$(TEX_MAG) -q -o marged_imd_files/tex_set_dun02.imd src_imd_files/area00_tex_set.imd src_imd_files/tex_autumn.imd src_imd_files/tex_griddmy.imd
	@$(G3DCVTR) marged_imd_files/tex_set_dun02.imd -o nsbtx_files/tex_set_dun02.nsbtx -etex

nsbtx_files/tex_set_dun03.nsbtx: src_imd_files/dun03_tex_set.imd
	@echo src_imd_files/dun03_tex_set.imd Å® nsbtx_files/tex_set_dun03.nsbtx
	@$(COPY) src_imd_files/dun03_tex_set.imd marged_imd_files/tex_set_dun03.imd
	@$(G3DCVTR) marged_imd_files/tex_set_dun03.imd -o nsbtx_files/tex_set_dun03.nsbtx -etex

nsbtx_files/tex_set_dun04.nsbtx: src_imd_files/dun04_tex_set.imd
	@echo src_imd_files/dun04_tex_set.imd Å® nsbtx_files/tex_set_dun04.nsbtx
	@$(COPY) src_imd_files/dun04_tex_set.imd marged_imd_files/tex_set_dun04.imd
	@$(G3DCVTR) marged_imd_files/tex_set_dun04.imd -o nsbtx_files/tex_set_dun04.nsbtx -etex

nsbtx_files/tex_set_dun05.nsbtx: src_imd_files/dun05_tex_set.imd src_imd_files/tex_winter.imd src_imd_files/tex_griddmy.imd
	@echo src_imd_files/dun05_tex_set.imd + src_imd_files/tex_winter.imd Å® nsbtx_files/tex_set_dun05.nsbtx
	@-rm marged_imd_files/tex_set_dun05.imd
	@-$(TEX_MAG) -q -o marged_imd_files/tex_set_dun05.imd src_imd_files/dun05_tex_set.imd src_imd_files/tex_winter.imd src_imd_files/tex_griddmy.imd
	@$(G3DCVTR) marged_imd_files/tex_set_dun05.imd -o nsbtx_files/tex_set_dun05.nsbtx -etex

