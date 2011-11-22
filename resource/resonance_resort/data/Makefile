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
targetcity 		= resort_city
targetpresent = resort_present
targetpreset 	= resort_preset
targetcombo1	= resort_combo_view
targetcombo2	= resort_combo_shop
targetcombo3	= resort_combo_next

#------------------------------------------------------------------------------
#依存するファイル名
#------------------------------------------------------------------------------
xlsname			= resonance_resort_data
msgfile 		= ..\..\..\prog\include\msg\script/msg_resort_scr.h
scriptfile	= ..\..\fldmapdata\script\resort_scr_def.h
bmodelfile 	= ../../fldmapdata/build_model/output/buildmodel_indoor.naix
fieldevent	= ../../fldmapdata/eventdata/tmp/c16.h
landpatch 	= ../../fldmapdata/land_data_patch/land_data_patch_def.h
objcode 		= ../../fldmmdl/fldmmdl_objcode.h
fldtrade		= ../../fld_trade/fld_trade_list.h

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
CITYCONV		= resort_city_converter.rb
PRESETCONV		= resort_preset_converter.rb
PRESENTCONV		= resort_present_converter.rb
COMBOCONV		= resort_combo_converter.rb

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
do-build:	$(TARGETDIR_ARC)/$(targetall).narc $(TARGETDIR_ARC)/$(targetall).naix $(TARGETDIR_PROG)/$(targetpeopleh).h $(TARGETDIR_PROG)/$(targetshop).h $(TARGETDIR_PROG)/$(targetspot).h $(TARGETDIR_PROG)/$(targetpreset).h

# narc のコピーだけ実行する
$(TARGETDIR_ARC)/$(targetall).narc: $(targetall).narc
	$(COPY) $(targetall).narc $(TARGETDIR_ARC)

$(TARGETDIR_ARC)/$(targetall).naix: $(targetall).naix
	$(COPY) $(targetall).naix $(TARGETDIR_ARC)

$(TARGETDIR_PROG)/$(targetpeopleh).h:	$(targetpeopleh).h
	$(COPY) $(targetpeopleh).h $(TARGETDIR_PROG)

$(TARGETDIR_PROG)/$(targetshop).h:	$(targetshop).h
	$(COPY) $(targetshop).h $(TARGETDIR_PROG)

$(TARGETDIR_PROG)/$(targetspot).h:	$(targetspot).h
	$(COPY) $(targetspot).h $(TARGETDIR_PROG)

$(TARGETDIR_PROG)/$(targetpreset).h:	$(targetpreset).h
	$(COPY) $(targetpreset).h $(TARGETDIR_PROG)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetall).narc $(targetall).naix: $(targetpeople).dat $(targetshop).dat $(targetservice).dat $(targetinstitution).dat $(targetspot).dat $(targetnpc).dat $(targetkutikomi).dat $(targetcity).dat $(targetpresent).dat $(targetpreset).dat $(targetcombo1).dat $(targetcombo2).dat $(targetcombo3).dat
	nnsarc -r -l -n -i $(targetall) $^
	$(NAIXCUT) $(targetall).naix
	#-rm -f *.dat

#データコンバート
$(targetpeople).dat $(targetshop).dat $(targetservice).dat $(targetpeopleh).h $(targetinstitution).dat $(targetspot).dat $(targetnpc).dat $(targetkutikomi).dat $(targetcity).dat $(targetpresent).dat $(targetshop).h $(targetspot).h $(targetpreset).h $(targetpreset).dat $(targetcombo1).dat $(targetcombo2).dat $(targetcombo3).dat: $(xlsname).xls $(msgfile) $(scriptfile) $(PEOPLECONV) $(DEFINECONV) $(bmodelfile) $(fieldevent) $(landpatch) $(objcode)
	$(XLSCONV) -o $(targetpeople).csv -p 1 -s csv $(xlsname).xls
	$(XLSCONV) -o $(targetshop).csv -p 2 -s csv $(xlsname).xls
	$(XLSCONV) -o $(targetservice).csv -p 3 -s csv $(xlsname).xls
	$(XLSCONV) -o $(targetdefine).csv -p 4 -s csv $(xlsname).xls
	$(XLSCONV) -o $(targetinstitution).csv -p 5 -s csv $(xlsname).xls
	$(XLSCONV) -o $(targetspot).csv -p 6 -s csv $(xlsname).xls
	$(XLSCONV) -o $(targetnpc).csv -p 7 -s csv $(xlsname).xls
	$(XLSCONV) -o $(targetkutikomi).csv -p 8 -s csv $(xlsname).xls
	$(XLSCONV) -o $(targetcity).csv -p 9 -s csv $(xlsname).xls
	$(XLSCONV) -o $(targetpresent).csv -p 10 -s csv $(xlsname).xls
	$(XLSCONV) -o $(targetpreset).csv -p 11 -s csv $(xlsname).xls
	$(XLSCONV) -o $(targetcombo1).csv -p 12 -s csv $(xlsname).xls
	$(XLSCONV) -o $(targetcombo3).csv -p 13 -s csv $(xlsname).xls
	-rm -f *.dat
	ruby $(DEFINECONV) $(targetdefine).csv $(definehash) $(targetpeopleh)
	ruby $(PEOPLECONV) $(targetpeople).csv $(msgfile) $(scriptfile) $(targetpeople)
	ruby $(PRESENTCONV) $(targetpresent).csv $(targetpresent)
	ruby $(SHOPCONV) $(targetshop).csv $(targetservice).csv $(msgfile) $(bmodelfile) $(landpatch) $(targetpresent).h $(targetshop) $(targetservice)
	ruby $(INSTITUTIONCONV) $(targetinstitution).csv $(fieldevent) $(targetinstitution)
	ruby $(SPOTCONV) $(targetspot).csv $(msgfile) $(targetspot)
	ruby $(NPCCONV) $(targetnpc).csv $(targetkutikomi).csv $(msgfile) $(objcode) $(targetnpc) $(targetkutikomi)
	ruby $(CITYCONV) $(targetcity).csv $(targetpresent).h $(msgfile) $(targetcity)
	ruby $(PRESETCONV) $(targetpreset).csv $(msgfile) $(objcode) $(targetshop).h $(targetspot).h $(fldtrade) $(targetpreset)
	ruby $(COMBOCONV) $(targetcombo1).csv $(targetcombo3).csv $(targetcombo1) $(targetcombo2) $(targetcombo3)
	-rm -f *.csv
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


