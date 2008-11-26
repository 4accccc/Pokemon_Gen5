require "fileutils"

files = Dir.glob "*.imd"
files.each{|file|
	dmd = file.sub(/\.imd$/,"\.3dmd")
	if FileTest.exists? dmd then
		puts "exists! #{dmd}"
	else
		puts "not exists.. #{dmd}"
		FileUtils.cp "map00_00c.3dmd", dmd
	end
}
