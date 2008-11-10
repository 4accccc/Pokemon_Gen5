#
#	マップ管理表コンバート用Makefile
#
#	2005.10.25	tamada
#	2008.11.10	tamada	WB環境に移植開始
#
#

include $(PROJECT_RSCDIR)macro_define

.PHONY:	do-build clean diff

TARGET	=	zonetable.dat
DATAFILE	= zonetable.xls
OUTPUT	=	$(TARGET) zone_id.h	#eventlist.txt
#OUTPUT	+=	mapname.bin mapname.dat msg_header.h eventarc.txt doorevent.h

EXCEL2TAB	=	$(PROJECT_ROOT)/tools/exceltool/exceltabconv.exe

#------------------------------------------------
#------------------------------------------------
do-build:	$(TARGET)

$(TARGET):	$(DATAFILE)
	$(EXCEL2TAB)	$(DATAFILE)
	$(MAKE) convert

convert:
	ruby zonetable.rb $(DATAFILE:.xls=.txt)

#------------------------------------------------
#------------------------------------------------
clean:
	-rm $(OUTPUT)
	-rm diff.txt zonetable.txt

#------------------------------------------------
#------------------------------------------------
cp_output:
	-$(foreach AFILE,$(OUTPUT),cp $(AFILE) $(AFILE).old $(AND)) true

diff_output:
	-$(foreach AFILE,$(OUTPUT),diff $(AFILE) $(AFILE).old >> diff.txt $(AND)) true

diff:
	$(MAKE) cp_output
	$(EXCEL2TAB)	$(DATAFILE)
	$(MAKE) convert
	-echo "#diff result" > diff.txt
	$(MAKE) diff_output
	cat diff.txt
	-rm $(addsuffix .old, $(OUTPUT))


