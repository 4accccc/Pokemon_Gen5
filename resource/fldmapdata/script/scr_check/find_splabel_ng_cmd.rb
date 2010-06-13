#EVƒtƒ@ƒCƒ‹—ñ‹“
all_ary = Dir.glob("../*.ev")
ary = Array.new
all_ary.each{|i|
  i.sub!(/..\//,"")
  idx = i.index("sp_")
  if (idx == nil) || (idx != 0) then
    ary << i
  end
}

list = ARGV[0]
target_label = ARGV[1]

ary.each{|i|
  src = "../" + i
  dst = i
#  system("ruby com_del.rb #{src} #{dst}")
  system("ruby sp_label_ng_cmd.rb #{dst} #{list} #{target_label}")
}
