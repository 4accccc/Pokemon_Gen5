INPUT_FILE	=	ARGV[0]
DEP_FILE	=	ARGV[1]
ARC_FILE	=	ARGV[2]

input_file = File.open(INPUT_FILE, "r")
dep_file = File.open(DEP_FILE, "w")
arc_file = File.open(ARC_FILE, "w")

dep_file.puts "DEPEND_FILES	=	\\\n"

input_file.gets

while line = input_file.gets
	column = line.split
	name = "#{column[1]}.nsbmd"
	dep_file.puts "\t$(RESDIR)/#{name} \\\n"
	arc_file.puts "\"nsbmdfiles/#{name}\""
end

input_file.close
dep_file.close
arc_file.close
