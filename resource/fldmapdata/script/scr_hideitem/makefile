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
TARGET_EV		=	hide_item.ev
TARGET_CDAT	= hide_item.cdat
SRCFILE	= hide_item.xls
SRCTXT	= hide_item.txt

DEPENDS	= $(PROJECT_ROOT)tools/hash/item_hash.rb hideitem.rb

XLS2TAB = ruby $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out.rb -c

.PHONY:	do-build clean

#--------------------------------------------------------------
#--------------------------------------------------------------
do-build:	$(DSTDIR)/$(TARGET_EV)


$(DSTDIR)/$(TARGET_EV): $(SRCTXT) $(DEPENDS)
	ruby hideitem.rb $(SRCTXT) $(TARGET_CDAT) $(TARGET_EV)
	unix2dos -D $(TARGET_EV)
	$(COPY) $(TARGET_EV) $(DSTDIR)


$(SRCTXT): $(SRCFILE)
	$(XLS2TAB) $(SRCFILE) > $(SRCTXT)


#--------------------------------------------------------------
#--------------------------------------------------------------
clean:
	rm -f $(TARGET_EV) $(SRCTXT) $(TARGET_CDAT)

