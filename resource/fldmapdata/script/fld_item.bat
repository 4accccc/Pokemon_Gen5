@echo update fld_item
@..\..\..\tools\exceltool\excelseetconv.exe -o scr_flditem/fld_item.csv -p 1 -s csv scr_flditem/fld_item.xls
@ruby scr_flditem/fld_item.rb scr_flditem/fld_item.csv ../../../prog/include/item/itemsym.h scr_flditem/fld_item_base.ev
@ruby scr_tools/ev_def.rb fld_item.ev scrid_offset/scr_offset_id.h
@unix2dos -D fld_item.ev
