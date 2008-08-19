
##ここに各種リソースと 作成するnarc名を書く
##
lsources = wm.ncl
gsources = wm.ncg wifi.ncg
targetname = wmi


.SUFFIXES: .NCLR .ncl .NCGR .ncg

DIFFCOPY = ruby $(PROJECT_ROOT)/tools/diffcopy.rb

lobjects = $(subst .ncl,.NCLR,$(lsources))
gobjects = $(subst .ncg,.NCGR,$(gsources))

.ncl.NCLR:
	g2dcvtr $<

.ncg.NCGR:
	g2dcvtr $< -bg

$(targetname).narc: $(lobjects) $(gobjects)
	nnsarc -r -l -n -i $(targetname) $^
	$(DIFFCOPY) $(targetname).narc $(PROJECT_ARCDIR)
	$(DIFFCOPY) $(targetname).naix $(PROJECT_ARCDIR) 

clean:
	rm -rf *.NCGR
	rm -rf *.NCLR

