#======================================================================
#
#
#		隠しアイテムコンバート用Makefile
#
#		2009.12.18 	tamada GAMEFREAK inc.
#
#======================================================================


include $(PROJECT_RSCDIR)macro_define

#--------------------------------------------------------------
#--------------------------------------------------------------
DSTDIR	= ..
TARGET	=	hide_item.ev
SRCFILE	= hide_item.xls
SRCTXT	= hide_item.txt
CDATFILE	= hide_item.cdat

XLS2TAB = ruby $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out.rb -c

.PHONY:	do-build clean

#--------------------------------------------------------------
#--------------------------------------------------------------
do-build:	$(DSTDIR)/$(TARGET)


$(DSTDIR)/$(TARGET): $(SRCTXT)
	ruby hideitem.rb $(SRCTXT) $(CDATFILE) $(TARGET)
	$(COPY) $(TARGET) $(DSTDIR)


$(SRCTXT): $(SRCFILE)
	$(XLS2TAB) $(SRCFILE) > $(SRCTXT)


#--------------------------------------------------------------
#--------------------------------------------------------------
clean:
	rm -f $(TARGET) $(SRCTXT) $(CDATFILE)

