require "fileutils"

del_ary = Array.new

file = open( ARGV[0], "r" )
pass = ARGV[1]

while line = file.gets
  str = line.chomp("\n").chomp("\r")
  str_3dmd = pass + str + ".3dmd"
  str_imd = pass + str + ".imd"
  str_bin = pass + str + ".bin"
  
  del_ary << str_3dmd
  del_ary << str_imd
  del_ary << str_bin
end

FileUtils.rm(del_ary, :force=>true)

p "del finished"
