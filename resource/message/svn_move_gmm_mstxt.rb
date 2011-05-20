require 'rexml/document'

# コマンドを実行する。コマンドのエコーを行う。失敗時に例外を投げる。
def exec_command(command)
	puts command
	command.gsub!("\\", "\\\\\\\\")
	output = `#{command}`

	if $? != 0 then
		raise("command failed --- #{command}")
	end

	return output
end

def read_listfile(filename)
	list = Array.new
	File.open(filename, "r") do |f|
		f.each_line do |line|
			line.chomp!
			next if line.empty?
			list << line
		end
	end
	return list
end

def is_exclude_file(exclude_files, filename)
	return exclude_files.include?(filename.gsub("\\","/"))
end

exclude_files = read_listfile("gmm_resource.list")

exec_command("svn add msproj --depth infinity")

info_xml = exec_command("svn info --xml src --depth infinity")
info = REXML::Document.new(info_xml)

File.open("delete_moved_files.bat", "w") do |out|
	info.each_element("/info/entry") do |element|
		if element.attribute("kind").value == "file" then
			filename = element.attribute("path").value
			next if is_exclude_file(exclude_files, filename)
			dst_filename = "msproj\\#{filename}".gsub(".gmm", ".mstxt")
			exec_command("svn copy #{filename} #{dst_filename}")
			exec_command("svn delete --keep-local #{filename}")
			exec_command("rm #{dst_filename}")
			out.print("del #{filename}\n")
		end
	end
end
