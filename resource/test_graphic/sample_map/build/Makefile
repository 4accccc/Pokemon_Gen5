
DUMMY_TARGET 	= dummy_file

.PHONY:	do-build clean

do-build:	$(DUMMY_TARGET)

$(DUMMY_TARGET):	$(wildcard *.imd)
	g3dcvtr sample_tree.imd -etex
	g3dcvtr pc_01_h.imd
	g3dcvtr pc_01_l.imd
	g3dcvtr buil_01_h.imd
	g3dcvtr buil_01_l.imd
	mv *.nsbmd ..
	mv *.nsbtx ..
	touch $(DUMMY_TARGET)

clean:
	-rm -f $(DUMMY_TARGET)

