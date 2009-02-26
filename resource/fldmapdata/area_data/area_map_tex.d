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

nsbtx_files/tex_set_out01.nsbtx: src_imd_files/tex_spring.imd
	@echo src_imd_files/tex_spring.imd Å® nsbtx_files/tex_set_out01.nsbtx
	@$(COPY) src_imd_files/tex_spring.imd marged_imd_files/tex_set_out01.imd
	@$(G3DCVTR) marged_imd_files/tex_set_out01.imd -o nsbtx_files/tex_set_out01.nsbtx -etex

nsbtx_files/tex_set_out02.nsbtx: src_imd_files/tex_summer.imd
	@echo src_imd_files/tex_summer.imd Å® nsbtx_files/tex_set_out02.nsbtx
	@$(COPY) src_imd_files/tex_summer.imd marged_imd_files/tex_set_out02.imd
	@$(G3DCVTR) marged_imd_files/tex_set_out02.imd -o nsbtx_files/tex_set_out02.nsbtx -etex

nsbtx_files/tex_set_out03.nsbtx: src_imd_files/tex_autumn.imd
	@echo src_imd_files/tex_autumn.imd Å® nsbtx_files/tex_set_out03.nsbtx
	@$(COPY) src_imd_files/tex_autumn.imd marged_imd_files/tex_set_out03.imd
	@$(G3DCVTR) marged_imd_files/tex_set_out03.imd -o nsbtx_files/tex_set_out03.nsbtx -etex

nsbtx_files/tex_set_out04.nsbtx: src_imd_files/tex_winter.imd
	@echo src_imd_files/tex_winter.imd Å® nsbtx_files/tex_set_out04.nsbtx
	@$(COPY) src_imd_files/tex_winter.imd marged_imd_files/tex_set_out04.imd
	@$(G3DCVTR) marged_imd_files/tex_set_out04.imd -o nsbtx_files/tex_set_out04.nsbtx -etex

nsbtx_files/tex_set_out05.nsbtx: src_imd_files/testroom00_00c.imd src_imd_files/tex_spring.imd src_imd_files/tex_griddmy.imd
	@echo src_imd_files/testroom00_00c.imd + src_imd_files/tex_spring.imd Å® nsbtx_files/tex_set_out05.nsbtx
	@-rm marged_imd_files/tex_set_out05.imd
	@-$(TEX_MAG) -q -o marged_imd_files/tex_set_out05.imd src_imd_files/testroom00_00c.imd src_imd_files/tex_spring.imd src_imd_files/tex_griddmy.imd
	@$(G3DCVTR) marged_imd_files/tex_set_out05.imd -o nsbtx_files/tex_set_out05.nsbtx -etex

nsbtx_files/tex_set_in01.nsbtx: src_imd_files/testroom00_00c.imd
	@echo src_imd_files/testroom00_00c.imd Å® nsbtx_files/tex_set_in01.nsbtx
	@$(COPY) src_imd_files/testroom00_00c.imd marged_imd_files/tex_set_in01.imd
	@$(G3DCVTR) marged_imd_files/tex_set_in01.imd -o nsbtx_files/tex_set_in01.nsbtx -etex

nsbtx_files/tex_set_in02.nsbtx: src_imd_files/testroom00_00c.imd
	@echo src_imd_files/testroom00_00c.imd Å® nsbtx_files/tex_set_in02.nsbtx
	@$(COPY) src_imd_files/testroom00_00c.imd marged_imd_files/tex_set_in02.imd
	@$(G3DCVTR) marged_imd_files/tex_set_in02.imd -o nsbtx_files/tex_set_in02.nsbtx -etex

nsbtx_files/tex_set_in03.nsbtx: src_imd_files/testpc00_00c.imd
	@echo src_imd_files/testpc00_00c.imd Å® nsbtx_files/tex_set_in03.nsbtx
	@$(COPY) src_imd_files/testpc00_00c.imd marged_imd_files/tex_set_in03.imd
	@$(G3DCVTR) marged_imd_files/tex_set_in03.imd -o nsbtx_files/tex_set_in03.nsbtx -etex

nsbtx_files/tex_set_in04.nsbtx: src_imd_files/testroom00_00c.imd
	@echo src_imd_files/testroom00_00c.imd Å® nsbtx_files/tex_set_in04.nsbtx
	@$(COPY) src_imd_files/testroom00_00c.imd marged_imd_files/tex_set_in04.imd
	@$(G3DCVTR) marged_imd_files/tex_set_in04.imd -o nsbtx_files/tex_set_in04.nsbtx -etex

nsbtx_files/tex_set_in05.nsbtx: src_imd_files/testroom00_00c.imd
	@echo src_imd_files/testroom00_00c.imd Å® nsbtx_files/tex_set_in05.nsbtx
	@$(COPY) src_imd_files/testroom00_00c.imd marged_imd_files/tex_set_in05.imd
	@$(G3DCVTR) marged_imd_files/tex_set_in05.imd -o nsbtx_files/tex_set_in05.nsbtx -etex

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

nsbtx_files/tex_set_loopbridge.nsbtx: src_imd_files/loopbridge.imd
	@echo src_imd_files/loopbridge.imd Å® nsbtx_files/tex_set_loopbridge.nsbtx
	@$(COPY) src_imd_files/loopbridge.imd marged_imd_files/tex_set_loopbridge.imd
	@$(G3DCVTR) marged_imd_files/tex_set_loopbridge.imd -o nsbtx_files/tex_set_loopbridge.nsbtx -etex

nsbtx_files/tex_set_c3.nsbtx: src_imd_files/tex_c3.imd
	@echo src_imd_files/tex_c3.imd Å® nsbtx_files/tex_set_c3.nsbtx
	@$(COPY) src_imd_files/tex_c3.imd marged_imd_files/tex_set_c3.imd
	@$(G3DCVTR) marged_imd_files/tex_set_c3.imd -o nsbtx_files/tex_set_c3.nsbtx -etex

