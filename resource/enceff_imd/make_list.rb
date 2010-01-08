$KCODE = "SJIS"

txt_file = open(ARGV.shift,"r")

list_file = open(ARGV.shift,"w")

list_file.printf("TARGET_DATA =")

while line = txt_file.gets
  list_file.printf(" \\\n")

  str = line.sub(/data\//,"")
  str2 = str.sub(/imd/,"nsbmd")
  str3 = str2.sub(/ica/,"nsbca")

  dst = str3.chomp("\n").chomp("\r")

  list_file.printf("\t%s", dst)
end

