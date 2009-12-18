#---------------------------------------------------------------
#	フィールドエフェクト アーカイブ用Makefile
#---------------------------------------------------------------
USERS = kagaya fuchino iwasawa obata_toshihiro ibe_mana yoshida lee_hyunjung
include $(PROJECT_RSCDIR)/macro_define

TARGET_FE 	=	fldeff.narc
HEADER_FE	=	$(TARGET_FE:.narc=.naix)

COPYDIR = $(PROJECT_ROOT)/prog/arc/fieldmap

#リソースディレクトリ
RESDIR	= ./res
#テンポラリディレクトリ
TMPDIR	= ./temp

#コンバート対象リスト
include	fldeff_list

#G3Dコンバータへのパス
#G3DCVTR	= $(NITROSYSTEM_ROOT)/tools/win/bin/g3dcvtr.exe
G3DCVTR	= g3dcvtr.exe

#G2Dコンバータへのパス
#G2DCVTR	= $(NITROSYSTEM_ROOT)/tools/win/bin/g2dcvtr.exe
G2DCVTR	= g2dcvtr.exe

#--------------------------------------------------
#	ファイルリスト置き換え
#--------------------------------------------------
#imd,nsbmd
FEDATA_MDL	= $(notdir $(G3D_FE_MDL:.imd=.nsbmd))
FEDATA_MDL := $(addprefix $(TMPDIR)/,$(FEDATA_MDL))

#imd,nsbtx
FEDATA_TEX	= $(notdir $(G3D_FE_TEX:.imd=.nsbtx))
FEDATA_TEX := $(addprefix $(TMPDIR)/,$(FEDATA_TEX))

#itp,nsbtp
FEDATA_ANMITP	= $(notdir $(G3D_FE_ANMITP:.itp=.nsbtp))
FEDATA_ANMITP := $(addprefix $(TMPDIR)/,$(FEDATA_ANMITP))

#ica,nsbca
FEDATA_ANMICA = $(notdir $(G3D_FE_ANMICA:.ica=.nsbca))
FEDATA_ANMICA := $(addprefix $(TMPDIR)/,$(FEDATA_ANMICA))

#ima,nsbma
FEDATA_ANMIMA = $(notdir $(G3D_FE_ANMIMA:.ima=.nsbma))
FEDATA_ANMIMA := $(addprefix $(TMPDIR)/,$(FEDATA_ANMIMA))

#ita,nsbta
FEDATA_ANMITA = $(notdir $(G3D_FE_ANMITA:.ita=.nsbta))
FEDATA_ANMITA := $(addprefix $(TMPDIR)/,$(FEDATA_ANMITA))

#iva,nsbva
FEDATA_ANMIVA = $(notdir $(G3D_FE_ANMIVA:.iva=.nsbva))
FEDATA_ANMIVA := $(addprefix $(TMPDIR)/,$(FEDATA_ANMIVA))

#G3D_FE_NONCV
FEDATA_NONCV = $(notdir $(G3D_FE_NONCV))
FEDATA_NONCV := $(addprefix $(RESDIR)/,$(FEDATA_NONCV))

#--------------------------------------------------
#	拡張子 生成ルール
#--------------------------------------------------
#.SUFFIXES:
RESDIR_EFFENC = $(RESDIR)/$(EFFENC_SGRASS) \
								$(RESDIR)/$(EFFENC_LGRASS) \
								$(RESDIR)/$(EFFENC_WATER) \
								$(RESDIR)/$(EFFENC_SEA) \
								$(RESDIR)/$(EFFENC_BIRD) \
								$(RESDIR)/$(EFFENC_CAVE)

RESDIR_D06 = $(RESDIR)/$(EFF_D06DENKI)

RESDIR_ALL = $(RESDIR) $(RESDIR_EFFENC) $(RESDIR_D06)

vpath %imd $(RESDIR_ALL)
vpath %itp $(RESDIR_ALL)
vpath %ica $(RESDIR_ALL)
vpath %ima $(RESDIR_ALL)
vpath %ita $(RESDIR_ALL)
vpath %iva $(RESDIR_ALL)

#imd --> nsbmd
$(TMPDIR)/%.nsbmd: %.imd
	@echo Model Data Convert: $^ $@
	@$(G3DCVTR) $^ -o $@

#imd --> nsbtx
$(TMPDIR)/%.nsbtx: %.imd
	@echo Texture Data Convert: $^ $@
	@$(G3DCVTR) $^ -o $@ -etex

#itp --> nsbtp
$(TMPDIR)/%.nsbtp: %.itp
	@echo Anime itp File Convert: $^ $@
	@$(G3DCVTR) $^ -o $@ 

#ica --> nsbca
$(TMPDIR)/%.nsbca: %.ica
	@echo Anime ica File Convert: $^ $@
	@$(G3DCVTR) $^ -o $@ 

#ima --> nsbma
$(TMPDIR)/%.nsbma: %.ima
	@echo Anime ima File Convert: $^ $@
	@$(G3DCVTR) $^ -o $@ 

#ita --> nsbta
$(TMPDIR)/%.nsbta: %.ita
	@echo Anime ita File Convert: $^ $@
	@$(G3DCVTR) $^ -o $@ 

#iva --> nsbva
$(TMPDIR)/%.nsbva: %.iva
	@echo Anime iva File Convert: $^ $@
	@$(G3DCVTR) $^ -o $@ 

#-------------------------------------------------
#	生成ルール定義
#-------------------------------------------------
do-build: $(COPYDIR)/$(TARGET_FE)

$(COPYDIR)/$(TARGET_FE): $(TARGET_FE)
	$(COPY) $(TARGET_FE) $(COPYDIR)
	$(COPY) $(HEADER_FE) $(COPYDIR)

ifeq	($(CONVERTUSER),true) #CONVERTUSER
$(TARGET_FE): $(FEDATA_TEX) $(FEDATA_MDL) $(FEDATA_ANMITP) $(FEDATA_ANMICA) $(FEDATA_ANMIMA) $(FEDATA_ANMITA) $(FEDATA_ANMIVA) $(FEDATA_NONCV)
	nnsarc -i -c -l -n $(TARGET_FE) $(FEDATA_TEX) $(FEDATA_MDL) $(FEDATA_ANMITP) $(FEDATA_ANMICA) $(FEDATA_ANMIMA) $(FEDATA_ANMITA) $(FEDATA_ANMIVA) $(FEDATA_NONCV) > arc_result.txt
	$(NAIXCUT) $(HEADER_FE)

all:
	$(MAKE) clean
	$(MAKE)
endif

#全消去
clean:
	-rm -f $(COPYDIR)/$(TARGET_FE)
	-rm -f $(COPYDIR)/$(HEADER_FE)
ifeq	($(CONVERTUSER),true) #CONVERTUSER
	-rm -f $(TARGET_FE)
	-rm -f $(HEADER_FE)
	-rm -f $(FEDATA_TEX)
	-rm -f $(FEDATA_MDL)
	-rm -f $(FEDATA_ANMITP)
	-rm -f $(FEDATA_ANMICA)
	-rm -f $(FEDATA_ANMIMA)
	-rm -f $(FEDATA_ANMITA)
	-rm -f $(FEDATA_ANMIVA)
endif
