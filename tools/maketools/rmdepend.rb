require "fileutils"

search_name = ARGV.shift
build_type = `make echo_buildtype`.chomp
#puts "result:#{build_type}"
depend_path = "./depend/#{build_type}/*.d"
command = "grep #{search_name} #{depend_path}"
#puts "command:#{command}"
dependfiles = `#{command}`
#puts dependfiles
dependfiles.each{|line|
  file = line.sub(/:.*$/,"").chomp
  puts "rm #{file}"
  FileUtils.rm( file )
  FileUtils.rm( file.sub(/\/depend\//,"\/obj\/").sub(/d$/,"o") )
}
