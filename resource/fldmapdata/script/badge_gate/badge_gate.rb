
ORG_SYMBOL = "r10r0001"
COPY_SYMBOL = [
  "r10r0101", "r10r0201", "r10r0301", "r10r0401",
  "r10r0501", "r10r0601", "r10r0701", "r10r0801" ]


org_main_text = File.open(ORG_SYMBOL + ".ev" ){|file| file.read }
org_sp_text = File.open( "sp_" + ORG_SYMBOL + ".ev"){|file| file.read }

def replace_text( copy_sym, count, org_txt )
  copy_text = org_txt.gsub(ORG_SYMBOL, copy_sym).gsub(ORG_SYMBOL.upcase, copy_sym.upcase)
  replace = sprintf("%02d", count)
  copy_text = copy_text.gsub( /00/, replace )
  replace = sprintf("%02d", count + 1)
  copy_text = copy_text.gsub( /99/, replace )
  return copy_text
end

count = 1
COPY_SYMBOL.each{|copy_sym|

  copy_main_file = "../" + copy_sym + ".ev"
  copy_main_text = replace_text( copy_sym, count, org_main_text )
  File.open(copy_main_file, "w"){|file|
    file.write( copy_main_text)
  }
  system("unix2dos -D " + copy_main_file )

  copy_sp_file = "../sp_" + copy_sym + ".ev"
  copy_sp_text = replace_text( copy_sym, count, org_sp_text )
  File.open(copy_sp_file, "w"){|file|
    file.write( copy_sp_text )
  }
  system("unix2dos -D " + copy_sp_file )

  count += 1
}
