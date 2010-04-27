FILENAME = ARGV[0]

file = File.open( FILENAME, "r" )
lines = file.readlines
file.close


file = File.open( FILENAME, "w" )

lines.each do |line|
  if line =~ /.*\/.*\/.*\// then
    puts line + "==> cut!"
  else
    file.puts( line )
  end
end

file.close
