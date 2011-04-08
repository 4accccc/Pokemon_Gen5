#------------------------------------------------------------------------------
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= nagihashi_toru shimoyamada

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetall			= resonance_resort_data
targetpeople	= resort_people
targetdefine	= resort_define
targetpeopleh	= resort_people_def
targetshop 		= resort_shop
targetservice = resort_service
definehash		= resort_define_hash
targetinstitution = resort_institution
targetspot 		= resort_spot
targetnpc 		= resort_npc
targetkutikomi = resort_kutikomi
targetoffreco  = resort_offreco

#------------------------------------------------------------------------------
#依存するファイル名
#------------------------------------------------------------------------------
xlsname			= resonance_resort_data
msgfile 		= ..\..\..\prog\include\msg\script/msg_resort_scr.h #TODO
scriptfile	= ..\..\fldmapdata\script\resort_scr_def.h #TODO
bmodelfile 	= ../../fldmapdata/build_model/output/buildmodel_outdoor.naix
fieldevent	= ../../fldmapdata/eventdata/tmp/c16.h
landpatch 	= ../../fldmapdata/land_data_patch/land_data_patch_def.h
objcode 		= ../../fldmmdl/fldmmdl_objcode.h

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR_ARC = $(PROJECT_ARCDIR)
TARGETDIR_PROG = ..\..\..\prog\src/field/resonance_resort/

#------------------------------------------------------------------------------
#TOOLS
#------------------------------------------------------------------------------
XLSCONV 		= $(PROJECT_ROOT)tools/exceltool/ExcelSeetConv.exe
DEFINECONV 	= resort_define_converter.rb
PEOPLECONV 	= resort_people_converter.rb
SHOPCONV 		= resort_shop_converter.rb
INSTITUTIONCONV 		= resort_institution_converter.rb
SPOTCONV 		= resort_spot_converter.rb
NPCCONV 			= resort_npc_converter.rb

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= 

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

.PHONY:	do-build clean

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR_ARC)/$(targetall).narc $(TARGETDIR_ARC)/$(targetall).naix $(TARGETDIR_PROG)/$(targetpeopleh).h

# narc のコピーだけ実行する
$(TARGETDIR_ARC)/$(targetall).narc: $(targetall).narc
	$(COPY) $(targetall).narc $(TARGETDIR_ARC)

$(TARGETDIR_ARC)/$(targetall).naix: $(targetall).naix
	$(COPY) $(targetall).naix $(TARGETDIR_ARC)

$(TARGETDIR_PROG)/$(targetpeopleh).h:	$(targetpeopleh).h
	$(COPY) $(targetpeopleh).h $(TARGETDIR_PROG)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetall).narc $(targetall).naix: $(targetpeople).dat $(targetshop).dat $(targetservice).dat $(targetinstitution).dat $(targetspot).dat $(targetnpc).dat $(targetkutikomi).dat $(targetoffreco).dat
	nnsarc -r -l -n -i $(targetall) $^
	$(NAIXCUT) $(targetall).naix

#データコンバート
$(targetpeople).dat $(targetshop).dat $(targetservice).dat $(targetpeopleh).h $(targetinstitution).dat $(targetspot).dat $(targetnpc).dat $(targetkutikomi).dat $(targetoffreco).dat: $(xlsname).xls $(msgfile) $(scriptfile) $(PEOPLECONV) $(DEFINECONV) $(bmodelfile) $(fieldevent) $(landpatch) $(objcode)
	$(XLSCONV) -o $(targetpeople).csv -p 1 -s csv $(xlsname).xls
	$(XLSCONV) -o $(targetshop).csv -p 2 -s csv $(xlsname).xls
	$(XLSCONV) -o $(targetservice).csv -p 3 -s csv $(xlsname).xls
	$(XLSCONV) -o $(targetdefine).csv -p 4 -s csv $(xlsname).xls
	$(XLSCONV) -o $(targetinstitution).csv -p 5 -s csv $(xlsname).xls
	$(XLSCONV) -o $(targetspot).csv -p 6 -s csv $(xlsname).xls
	$(XLSCONV) -o $(targetnpc).csv -p 7 -s csv $(xlsname).xls
	$(XLSCONV) -o $(targetkutikomi).csv -p 8 -s csv $(xlsname).xls
	$(XLSCONV) -o $(targetoffreco).csv -p 9 -s csv $(xlsname).xls
	rm -f *.dat
	ruby $(DEFINECONV) $(targetdefine).csv $(definehash) $(targetpeopleh)
	ruby $(PEOPLECONV) $(targetpeople).csv $(msgfile) $(scriptfile) $(targetpeople)
	ruby $(SHOPCONV) $(targetshop).csv $(targetservice).csv $(msgfile) $(bmodelfile) $(landpatch) $(targetshop) $(targetservice)
	ruby $(INSTITUTIONCONV) $(targetinstitution).csv $(fieldevent) $(targetinstitution)
	ruby $(SPOTCONV) $(targetspot).csv $(msgfile) $(targetspot)
	ruby $(NPCCONV) $(targetnpc).csv $(targetkutikomi).csv $(targetoffreco).csv $(msgfile) $(objcode) $(targetnpc) $(targetkutikomi) $(targetoffreco)
endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.dat
	rm -f *.h
	rm -f *.csv
	rm -f *.narc
	rm -f *.naix
	rm -f $(targetall).narc
	rm -f $(targetall).naix
endif
	rm -f $(TARGETDIR_ARC)/$(targetall).narc
	rm -f $(TARGETDIR_ARC)/$(targetall).naix


