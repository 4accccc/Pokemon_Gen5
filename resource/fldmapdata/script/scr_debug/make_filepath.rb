#==========================================================================
#
#
#		パスのバイナリデータ生成
#
#
#	※script_seq.narcのアーカイブ順に保持する
#
#==========================================================================
output = ""
pwd = Dir.pwd.sub(/^\/cygdrive\/c\//, "c:\/")
lines = STDIN.readlines

=begin
MAX_PATH_SIZE = 128

lines[0].chomp.split.each{|name|
	path = pwd + "\/" + name
	if path.length > MAX_PATH_SIZE 
		raise Exception, "#{path} size(#{path.length}) > MAX_PATH_SIZE"
	end
	output += path + '\0' * (MAX_PATH_SIZE - path.length)
}
File.open(ARGV[0],"wb"){|file| file.write output }
=end

MAX_PATH_SIZE = 32
output += sprintf("static const char scrDirPath[] = \"%s/\";\n\n", pwd )
output += sprintf("static const char * scrNamePath[] = {\n")

lines[0].chomp.split.each{|name|
	output += sprintf("\t\"%s\",\n", name )
}
output += "};\n"

File.open(ARGV[0],"w"){|file| file.write output }
