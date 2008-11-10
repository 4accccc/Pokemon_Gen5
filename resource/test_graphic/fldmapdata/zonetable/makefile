#
#	マップ管理表コンバート用Makefile
#
#	2005.10.25	tamada
#
#

include $(PROJECT_RSCDIR)macro_define

TARGET	=	zonetable.dat
DATAFILE	= zonetable.xls
OUTPUT	=	$(TARGET) zone_id.h eventlist.txt
OUTPUT	+=	mapname.bin mapname.dat msg_header.h eventarc.txt doorevent.h

#EXCEL2TAB	=	../../../convert/exceltool/exceltabconv
EXCEL2TAB	=	$(PROJECT_ROOT)/tools/exceltool/exceltabconv.exe

$(TARGET):	$(DATAFILE)
	$(EXCEL2TAB)	$(DATAFILE)
	$(MAKE) convert

#..\..\..\convert\exceltool\exceltabconv zonetable.xls
#ruby zonetable.rb zonetable.txt

convert:
	ruby zonetable.rb $(DATAFILE:.xls=.txt)

clean:
	-rm $(OUTPUT)
	-rm diff.txt zonetable.txt

cp_output:
	$(foreach AFILE,$(OUTPUT),cp $(AFILE) $(AFILE).old $(AND)) true
diff_output:
	$(foreach AFILE,$(OUTPUT),diff $(AFILE) $(AFILE).old >> diff.txt $(AND)) true
diff:
	$(MAKE) cp_output
	$(MAKE) convert
	-echo "#diff result" > diff.txt
	$(MAKE) diff_output
	cat diff.txt
	-rm $(addsuffix .old, $(OUTPUT))


#	-cp zone_id.h zone_id.h.old
#	-cp zonetable.dat zonetable.dat.old
#	-cp eventlist.txt eventlist.txt.old
#	$(MAKE) convert
#	-diff zone_id.h zone_id.h.old > diff.txt
#	-diff zonetable.dat zonetable.dat.old >> diff.txt
#	-diff eventlist.txt eventlist.txt.old >> diff.txt
#	cat diff.txt
#	-rm $(addsuffix .old, $(OUTPUT))

