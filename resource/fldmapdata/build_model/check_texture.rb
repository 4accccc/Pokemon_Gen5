
Dir.glob("imdfiles/*.imd").each{|imd|
  cmd = "g3dcvtr " + imd + " -etex -o texture_test.nsbtx"
  puts "#{cmd}"
  system cmd
}
system("rm texture_test.nsbtx")

