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

nsbtx_files/tex_set_out01_spring.nsbtx: src_imd_files/out01_texset_spring.imd
	@echo src_imd_files/out01_texset_spring.imd Å® nsbtx_files/tex_set_out01_spring.nsbtx
	@$(COPY) src_imd_files/out01_texset_spring.imd marged_imd_files/tex_set_out01_spring.imd
	@$(G3DCVTR) marged_imd_files/tex_set_out01_spring.imd -o nsbtx_files/tex_set_out01_spring.nsbtx -etex

nsbtx_files/tex_set_out01_summer.nsbtx: src_imd_files/out01_texset_summer.imd
	@echo src_imd_files/out01_texset_summer.imd Å® nsbtx_files/tex_set_out01_summer.nsbtx
	@$(COPY) src_imd_files/out01_texset_summer.imd marged_imd_files/tex_set_out01_summer.imd
	@$(G3DCVTR) marged_imd_files/tex_set_out01_summer.imd -o nsbtx_files/tex_set_out01_summer.nsbtx -etex

nsbtx_files/tex_set_out01_autumn.nsbtx: src_imd_files/out01_texset_autumn.imd
	@echo src_imd_files/out01_texset_autumn.imd Å® nsbtx_files/tex_set_out01_autumn.nsbtx
	@$(COPY) src_imd_files/out01_texset_autumn.imd marged_imd_files/tex_set_out01_autumn.imd
	@$(G3DCVTR) marged_imd_files/tex_set_out01_autumn.imd -o nsbtx_files/tex_set_out01_autumn.nsbtx -etex

nsbtx_files/tex_set_out01_winter.nsbtx: src_imd_files/out01_texset_winter.imd
	@echo src_imd_files/out01_texset_winter.imd Å® nsbtx_files/tex_set_out01_winter.nsbtx
	@$(COPY) src_imd_files/out01_texset_winter.imd marged_imd_files/tex_set_out01_winter.imd
	@$(G3DCVTR) marged_imd_files/tex_set_out01_winter.imd -o nsbtx_files/tex_set_out01_winter.nsbtx -etex

nsbtx_files/tex_set_out04.nsbtx: src_imd_files/h01_texset.imd
	@echo src_imd_files/h01_texset.imd Å® nsbtx_files/tex_set_out04.nsbtx
	@$(COPY) src_imd_files/h01_texset.imd marged_imd_files/tex_set_out04.imd
	@$(G3DCVTR) marged_imd_files/tex_set_out04.imd -o nsbtx_files/tex_set_out04.nsbtx -etex

nsbtx_files/tex_set_out05.nsbtx: src_imd_files/tex_c3.imd
	@echo src_imd_files/tex_c3.imd Å® nsbtx_files/tex_set_out05.nsbtx
	@$(COPY) src_imd_files/tex_c3.imd marged_imd_files/tex_set_out05.imd
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

nsbtx_files/tex_set_dun01.nsbtx: src_imd_files/dun01_tex_set.imd src_imd_files/out01_texset_summer.imd src_imd_files/tex_griddmy.imd
	@echo src_imd_files/dun01_tex_set.imd + src_imd_files/out01_texset_summer.imd Å® nsbtx_files/tex_set_dun01.nsbtx
	@-rm marged_imd_files/tex_set_dun01.imd
	@-$(TEX_MAG) -q -o marged_imd_files/tex_set_dun01.imd src_imd_files/dun01_tex_set.imd src_imd_files/out01_texset_summer.imd src_imd_files/tex_griddmy.imd
	@$(G3DCVTR) marged_imd_files/tex_set_dun01.imd -o nsbtx_files/tex_set_dun01.nsbtx -etex

nsbtx_files/tex_set_dun02.nsbtx: src_imd_files/area00_tex_set.imd src_imd_files/out01_texset_autumn.imd src_imd_files/tex_griddmy.imd
	@echo src_imd_files/area00_tex_set.imd + src_imd_files/out01_texset_autumn.imd Å® nsbtx_files/tex_set_dun02.nsbtx
	@-rm marged_imd_files/tex_set_dun02.imd
	@-$(TEX_MAG) -q -o marged_imd_files/tex_set_dun02.imd src_imd_files/area00_tex_set.imd src_imd_files/out01_texset_autumn.imd src_imd_files/tex_griddmy.imd
	@$(G3DCVTR) marged_imd_files/tex_set_dun02.imd -o nsbtx_files/tex_set_dun02.nsbtx -etex

nsbtx_files/tex_set_dun03.nsbtx: src_imd_files/dun03_tex_set.imd
	@echo src_imd_files/dun03_tex_set.imd Å® nsbtx_files/tex_set_dun03.nsbtx
	@$(COPY) src_imd_files/dun03_tex_set.imd marged_imd_files/tex_set_dun03.imd
	@$(G3DCVTR) marged_imd_files/tex_set_dun03.imd -o nsbtx_files/tex_set_dun03.nsbtx -etex

nsbtx_files/tex_set_dun04.nsbtx: src_imd_files/dun04_tex_set.imd
	@echo src_imd_files/dun04_tex_set.imd Å® nsbtx_files/tex_set_dun04.nsbtx
	@$(COPY) src_imd_files/dun04_tex_set.imd marged_imd_files/tex_set_dun04.imd
	@$(G3DCVTR) marged_imd_files/tex_set_dun04.imd -o nsbtx_files/tex_set_dun04.nsbtx -etex

nsbtx_files/tex_set_test_h01.nsbtx: src_imd_files/test_h01_texset.imd
	@echo src_imd_files/test_h01_texset.imd Å® nsbtx_files/tex_set_test_h01.nsbtx
	@$(COPY) src_imd_files/test_h01_texset.imd marged_imd_files/tex_set_test_h01.imd
	@$(G3DCVTR) marged_imd_files/tex_set_test_h01.imd -o nsbtx_files/tex_set_test_h01.nsbtx -etex

nsbtx_files/tex_set_test_h02.nsbtx: src_imd_files/test_h02_texset.imd
	@echo src_imd_files/test_h02_texset.imd Å® nsbtx_files/tex_set_test_h02.nsbtx
	@$(COPY) src_imd_files/test_h02_texset.imd marged_imd_files/tex_set_test_h02.imd
	@$(G3DCVTR) marged_imd_files/tex_set_test_h02.imd -o nsbtx_files/tex_set_test_h02.nsbtx -etex

nsbtx_files/tex_set_loopbridge.nsbtx: src_imd_files/loopbridge.imd
	@echo src_imd_files/loopbridge.imd Å® nsbtx_files/tex_set_loopbridge.nsbtx
	@$(COPY) src_imd_files/loopbridge.imd marged_imd_files/tex_set_loopbridge.imd
	@$(G3DCVTR) marged_imd_files/tex_set_loopbridge.imd -o nsbtx_files/tex_set_loopbridge.nsbtx -etex

