#----------------------------------------------------------------------------
#
#		script コンバート
#
#----------------------------------------------------------------------------

.PHONY:	do-build	clean

#共通マクロ
include $(PROJECT_RSCDIR)/macro_define

#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
#マップ依存のスクリプト（t01.ev,sp_t01.ev etc.）：定義ファイルは自動生成
SCRIPTFILES	=
include zone_script.list

#マップ非依存スクリプト（common_scr.ev init_scr.ev等）：定義ファイルは自動生成
#UNIQ_SCRIPTFILES =
include uniq_script.list

#
HEADER_TARGETS	= $(UNIQ_SCRIPTFILES) $(SCRIPTFILES)
ifeq ($(PM_DEBUG),yes)
HEADER_TARGETS += $(UNIQ_DEBUG_EVENT_SCRFILES)
endif

do-build:
		@ruby scr_tools/make_depend.rb $(HEADER_TARGETS) > makefile_h.depend
		make -f makefile_h do-build
		make -f makefile_ev do-build

clean:
		make -f makefile_h clean
		make -f makefile_ev clean

test:
	echo "$(PM_DEBUG)"
