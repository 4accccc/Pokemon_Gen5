#EVƒtƒ@ƒCƒ‹—ñ‹“
all_ary = Dir.glob("../*.ev")
ary = Array.new
all_ary.each{|i|
  i.sub!(/..\//,"")
  ary << i
}

ary.each{|i|
  src = "../" + i
  dst = i
  system("ruby com_del.rb #{src} #{dst}")
}
