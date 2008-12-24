#==========================================================
#
#	
#
#==========================================================
load 'area_common.def'
arg = $*
area_tbl = arg[0]
tex_list =arg[1]
TEX_PATH = "$(TEXSETIMDDIR)"

#f = open(tex_list,"w")
#f.puts "G3D_TEX_SET = \\"
$file_lst = Array.new

def add_lists(lists, name, put_flag)
	tmp = name.sub(/\.xls/,".imd")
	if lists.include?(tmp) == FALSE then
		if put_flag then
			puts "\t" + TEX_PATH + "/" + tmp + "\t\\"
		end
		lists << tmp
	end
end

File.open(area_tbl,"r"){ |file|
  file.gets
  file.gets

  puts "DEPEND_TEX_FILES = \\"

  while line = file.gets
	column = line.split "\t"
	if column[3]=="dummy" then 
		add_lists($file_lst, column[2], true)
=begin
		tmp = column[2].sub(/\.xls/,".imd")
		if $file_lst.include?(tmp) == FALSE then
		  f.puts "\t" + TEX_PATH + "/" + tmp + "\t\\"
		  $file_lst << tmp
		end
=end
	else
		add_lists($file_lst, column[2], false)
		add_lists($file_lst, column[3], true)
		add_lists($file_lst, column[4], true)
=begin
		tmp = column[2].sub(/\.xls/,".imd")
		if $file_lst.include?(tmp) == FALSE then
		  $file_lst << tmp
		end
		tmp = column[3].sub(/\.xls/,".imd")
		if $file_lst.include?(tmp) == FALSE then
		  f.puts "\t" + TEX_PATH + "/" + tmp + "\t\\"
		  $file_lst << tmp
		end
		tmp = column[4].sub(/\.xls/,".imd")
		if $file_lst.include?(tmp) == FALSE then 
		  f.puts "\t" + TEX_PATH + "/" + tmp + "\t\\"
		  $file_lst << tmp
		end
=end
	end
  end
}

#for m in $file_lst do
#  puts m
#end

#f.puts ""
#f.close

