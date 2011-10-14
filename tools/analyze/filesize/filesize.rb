#----------------------------------------------------------------------------
#
#
# バイナリファイルのサイズを容量順に抽出する仕組み
#
# 2009.11.06  tamada
#
#----------------------------------------------------------------------------
# tlf（TWLROM List File)のフォーマットについて
# 行端の1文字目がその行の属性を現す
# '#' コメント
# 'T' ターゲットのカレントディレクトリ？
# 'V' バージョン？
# 'H' ヘッダファイル指定？
# '9'
# '7'
# 'F' ファイル指定
# 'P' パディング指定？

# ファイル指定行の各項目
# F,開始アドレス、終了アドレス、？、ファイルID？、？、ファイル指定パス、DS内指定パス、ユニックスタイム？、アライメント、？
#


#----------------------------------------------------------------------------
# arc_file.h の解析　"filetree/*/*/*/*" --> 元ファイル名に変換する
#----------------------------------------------------------------------------
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

#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
def read_rsf( path )
  size = 0
  File.open(path){|file|
    file.each{|line|
      if line =~/\WRomSize\W*/ then
        columns = line.chomp!.split
        romsize_id = columns[1]
        case romsize_id
        when "64M" then
          return 64 * 1024 * 1024 / 8
        when "128M" then
          return 128 * 1024 * 1024 / 8
        when "256M" then
          return 256 * 1024 * 1024 / 8
        when "512M" then
          return 512 * 1024 * 1024 / 8
        when "1G" then
          return 1* 1024 * 1024 * 1024 / 8
        when "2G" then
          return 2 * 1024 * 1024 * 1024 / 8
        when "4G" then
          return 4 * 1024 * 1024 * 1024 / 8
        else
          STDERR.printf(" 対応していないロムタイプです！！！[%s]\n", romsize_id )
          exit -1
        end
      end
    }
  }
  STDERR.printf(" RomSizeの指定が見つかりませんでした！！\n")
  exit -1
end
#----------------------------------------------------------------------------
# バイナリデータ情報保持用オブジェクト
#----------------------------------------------------------------------------
class BinData
  attr_reader :is_file
  attr_reader :sadr,:eadr,:size,:name
  attr_reader :path
  def initialize line
    column = line.split(',')
    @is_file = column[0] == 'F' ? true : false
    @sadr = column[1].to_i(16)
    @eadr = column[2].to_i(16)
    @size = @eadr - @sadr
    if @is_file == true then
      @path = column[6].gsub(/"/,"")
      @name = File.basename(@path)
    else
      @path = @name = "PADDING(" + column[1] + "-" + column[2] + ")"
    end
  end
end

#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
def reader( file )

  line = file.gets until line =~ /\# File Images/
  datas = Array.new
  while line = file.gets
    #行頭が'F'の行はファイル設定行
    if line =~/^[FP],/ then
      datas << BinData.new(line)
    end
  end
  return datas
end


#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
def round_n( value, keta )
  num = value * (10 ** (-keta) )
  return num.round() * (10**keta)
end

def per_str( value, max )
  per_val = round_n( value * 10000 / max, -5 )
  return sprintf("%2.2f%", per_val / 100 )
end
#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
DICT_ARCFILE = make_arcfile_dict()
max_size = read_rsf( ARGV[1] )

File.open(ARGV[0]){|file|
  datas = reader( file )
  datas.sort{|a,b| a.size <=> b.size }.each{|bin|
    if DICT_ARCFILE.has_key?(bin.path) then
      name = DICT_ARCFILE[bin.path]
    else
      name = bin.path
    end
    printf("%08x:%6s:%s\n", bin.size, per_str( bin.size, max_size), name )
  }
  e_adrs = datas.map{|bin| bin.eadr }.max
  printf(" %08x / %08x = %-6s\n", e_adrs, max_size, per_str( e_adrs, max_size ) )
  #p DICT_ARCFILE
  #p datas
}
