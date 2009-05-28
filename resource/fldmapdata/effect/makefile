#---------------------------------------------------------------
#	フィールドエフェクト アーカイブ用Makefile
#---------------------------------------------------------------
USERS = kagaya
include $(PROJECT_RSCDIR)/macro_define

TARGET_FE 	=	fldeff.narc
HEADER_FE	=	$(TARGET_FE:.narc=.naix)

COPYDIR = $(PROJECT_ROOT)/prog/arc/fieldmap

RESDIR	:= ./res

#コンバート対象リスト
include	fldeff_list

#G3Dコンバータへのパス
G3DCVTR	= $(NITROSYSTEM_ROOT)/tools/win/bin/g3dcvtr.exe

#G2Dコンバータへのパス
G2DCVTR	= $(NITROSYSTEM_ROOT)/tools/win/bin/g2dcvtr.exe

#--------------------------------------------------
#	ファイルリスト置き換え
#--------------------------------------------------
#G3D_FE_MDLからディレクトリ名部分を削除、拡張子をimdからnsbmdに置き換え
FEDATA_MDL	= $(notdir $(G3D_FE_MDL:.imd=.nsbmd))

#G3D_FE_TEXからディレクトリ名部分を削除、拡張子をimdからnsbtxに置き換え
FEDATA_TEX	= $(notdir $(G3D_FE_TEX:.imd=.nsbtx))

#G3D_FE_ANMDATからディレクトリ名部分を削除、拡張子をitpからnsbtpに置き換え
FEDATA_ANMITP	= $(notdir $(G3D_FE_ANMITP:.itp=.nsbtp))

#G3D_FE_ANMICAからディレクトリ名部分を削除、拡張子をicaからnsbcaに置き換え
FEDATA_ANMICA = $(notdir $(G3D_FE_ANMICA:.ica=.nsbca))

#G3D_FE_ANMIMAからディレクトリ名部分を削除、拡張子をimaからnsbmaに置き換え
FEDATA_ANMIMA = $(notdir $(G3D_FE_ANMIMA:.ima=.nsbma))

#G3D_FE_ANMITAからディレクトリ名部分を削除、拡張子をitaからnsbtaに置き換え
FEDATA_ANMITA = $(notdir $(G3D_FE_ANMITA:.ita=.nsbta))

#G3D_FE_ANMIVAからディレクトリ名部分を削除、拡張子をivaからnsbvaに置き換え
FEDATA_ANMIVA = $(notdir $(G3D_FE_ANMIVA:.iva=.nsbva))

#G3D_FE_NONCV
FEDATA_NONCV = $(notdir $(G3D_FE_NONCV))

#--------------------------------------------------
#	拡張子で生成ルールを指定する
#--------------------------------------------------
#imd --> nsbmdへの変換ルール定義
%.nsbmd: $(RESDIR)/%.imd
	@echo Model Data Convert: $^ $@
	@$(G3DCVTR) $(RESDIR)/$(@:.nsbmd=.imd) -o $@
	
#imd --> nsbtxへの変換ルール定義
%.nsbtx: $(RESDIR)/%.imd
	@echo Texture Data Convert: $^ $@
	@$(G3DCVTR) $(RESDIR)/$(@:.nsbtx=.imd) -o $@ -etex

#itp --> nsbtpへの変換ルール定義
%.nsbtp: $(RESDIR)/%.itp
	@echo Anime itp File Convert: $^ $@
	@$(G3DCVTR) $(RESDIR)/$(@:.nsbtp=.itp) -o $@ 

#ica --> nsbcaへの変換ルール定義
%.nsbca: $(RESDIR)/%.ica
	@echo Anime ica File Convert: $^ $@
	@$(G3DCVTR) $(RESDIR)/$(@:.nsbca=.ica) -o $@ 

#ima --> nsbmaへの変換ルール定義
%.nsbma: $(RESDIR)/%.ima
	@echo Anime ima File Convert: $^ $@
	@$(G3DCVTR) $(RESDIR)/$(@:.nsbma=.ima) -o $@ 

#ita --> nsbtaへの変換ルール定義
%.nsbta: $(RESDIR)/%.ita
	@echo Anime ita File Convert: $^ $@
	@$(G3DCVTR) $(RESDIR)/$(@:.nsbta=.ita) -o $@ 

#iva --> nsbvaへの変換ルール定義
%.nsbva: $(RESDIR)/%.vai
	@echo Anime iva File Convert: $^ $@
	@$(G3DCVTR) $(RESDIR)/$(@:.nsbva=.iva) -o $@ 

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
