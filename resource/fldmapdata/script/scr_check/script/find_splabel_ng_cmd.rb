#EVƒtƒ@ƒCƒ‹—ñ‹“
all_ary = Dir.glob("../ev_tmp/*.ev")
ary = Array.new
all_ary.each{|i|
  i.sub!(/..\/ev_tmp\//,"")
  idx = i.index("sp_")
  if (idx == nil) || (idx != 0) then
    ary << i
  end
}

list = ARGV[0]
target_label = ARGV[1]
type = ARGV[2]

ary.each{|i|
  src = "../ev_tmp/" + i
#  system("ruby com_del.rb #{src} #{dst}")
  system("ruby sp_label_ng_cmd.rb #{src} #{list} #{target_label} #{type}")
}
