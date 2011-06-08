
require "csv"

DATA_NAME = ARGV[0]
OUT_NAME = ARGV[1]

ci_data = CSV.open(DATA_NAME,"r")
out = open(OUT_NAME,"w")

# ヘッダ定義
out.puts "#! ruby -Ks"
out.puts "$cutin_id_array = ["
	
ci_data.each_with_index{|data,i|
  if i == 0 then
    next
  end
  if data.include?("#END") then
    break
  end
	out.puts '  "'+data[0]+'",'
}
out.puts "]"

#ncgファイル列挙
out.puts "$cutin_ncg_hash = {"
all_ary = Dir.glob("replace_tex/*/*.ncg")
all_ary.each_with_index{|target,i|
  dirname = File.dirname(target)
  ncg     = File.basename(target)
	out.puts '  "'+ncg+'" => "'+dirname+'/",'
}
out.puts "}"


#nclファイル列挙
out.puts "$cutin_ncl_hash = {"
all_ary = Dir.glob("replace_tex/*/*.ncl")
all_ary.each_with_index{|target,i|
  dirname = File.dirname(target)
  ncl     = File.basename(target)
	out.puts '  "'+ncl+'" => "'+dirname+'/",'
}
out.puts "}"
out.close

