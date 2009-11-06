
def make_arcfile_dict
  dict = Hash.new
  File.open(ENV["PROJECT_ARCDIR"] + "arc_file.h"){|file|
    file.each{|line|
      if line =~/\}/ || line =~/\{/ then next end
      column = line.chomp.split(',')
      shortpath = "filetree\/" + column[0].gsub(/"/,"").gsub(/\s/,"")
      longpath = column[1].sub(/^.*\/\//,"")
      dict[shortpath] = longpath
    }
  }
  return dict
end

class BinData
  attr_reader :sadr,:eadr,:size,:name
  attr_reader :path
  def initialize line
    column = line.split(',')
    @sadr = column[1].to_i(16)
    @eadr = column[2].to_i(16)
    @size = @eadr - @sadr
    @path = column[6].gsub(/"/,"")
    @name = File.basename(@path)
  end
end

def reader file

  line = file.gets until line =~ /\# File Images/
  datas = Array.new
  while line = file.gets
    #行頭が'F'の行はファイル設定行
    if line =~/^F,/ then
      datas << BinData.new(line)
    end
  end
  return datas
end

DICT_ARCFILE = make_arcfile_dict()

File.open(ARGV[0]){|file|
  datas = reader( file )
  datas.sort{|a,b| a.size <=> b.size }.each{|bin|
    if DICT_ARCFILE.has_key?(bin.path) then
      name = DICT_ARCFILE[bin.path]
    else
      name = bin.path
    end
    printf("%08x:%s\n", bin.size, name )
  }
  #p DICT_ARCFILE
  #p datas
}
