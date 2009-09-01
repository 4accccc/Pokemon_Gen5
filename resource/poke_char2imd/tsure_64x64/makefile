include pair_poke64x64

TEX_IMD_DIR			= tex_imd
TSURE_DATA_DIR		= tsure_data
REV_INFO			= rev_info

#----------------------------------------------------------------------------
#	ツールへのパス指定
#----------------------------------------------------------------------------
NCG2IMD_RB	= ruby -I ../rb ../ncg2imd.rb

TARGET_IMD = $(subst $(TSURE_DATA_DIR),$(TEX_IMD_DIR),$(64X64_DATA:.ncg=.imd))

%.imd: ../$(TSURE_DATA_DIR)/%.ncg ../$(TSURE_DATA_DIR)/%.ncl
	$(NCG2IMD_RB) $(subst $(TEX_IMD_DIR),$(TSURE_DATA_DIR),$(@:.imd=.ncg)) ../$(TEX_IMD_DIR) 8 2 $(subst $(TEX_IMD_DIR),$(REV_INFO),$(@:.imd=.txt))
#	mv $@ ../$(TEX_IMD_DIR)

do-build: $(TARGET_IMD)
