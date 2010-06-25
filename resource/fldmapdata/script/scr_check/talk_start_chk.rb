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

ary.each{|i|
  src = "../" + i
  dst = i
#  system("ruby com_del.rb #{src} #{dst}")
  system("ruby scr_check.rb #{dst}")
}
