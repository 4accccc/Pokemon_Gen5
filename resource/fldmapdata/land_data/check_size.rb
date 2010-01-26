size_str = ARGV[0]
filename = ARGV[1]

exit 1 if size_str == nil || filename == nil
unless File.exists?( filename )
  printf("\"%s\"が存在しません！\n", filename )
  exit 1
end
limit_size = Integer(size_str)
filesize = File.stat( filename ).size
if filesize > limit_size then
  printf( "\"%s\"のサイズ(%d)が%dをオーバーしています！\n", filename, filesize, limit_size )
  exit 1
end
