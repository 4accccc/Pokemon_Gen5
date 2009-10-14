############################################
#
#	Excelからタブ区切りテキスト生成
#
#	2009.10.14	k.ohno
#  互換性を試している余裕がなく作成 シートを番号指定で出力できる
#	2009.02.27	tamada
#	大野さんが使っていたライブラリを流用。
#
#
#	このファイルと同一ディレクトリにライブラリCXls2Xml.rbと
#	それが利用しているJAVAアーカイブxls2xml.jarが必要です。
#
#	使い方：
#	ruby tab_out.rb	(オプション）　ファイル名
#
#	オプション：
#	-t
#	-tab
#		タブ区切り（デフォルトの動作）
#	-c
#	--comma
#		カンマ区切り
#
#   -n x
#       ｘ番号のシートを出力(0から)
#
############################################

#このファイルが置かれている場所にライブラリもあるのでパスを検索
lib_path = File.dirname(__FILE__).gsub(/\\/,"/") + "/CXls2Xml"
require lib_path
#require 'CXls2Xml'

#エラーを定義
class CommnandLineArgumentError < Exception; end

class XlsTextReader < CXls2Xml
	def initialize
		#@xmldata=""
		@sheets ={}
    @sep = '\t'
    @sheetno = 0
	end
	def getXmlSheet(xlsName, sheetno)
		@xmldata = xls2xml(xlsName)
    @sheets = xml2sheet(@xmldata)
    @sheetno = sheetno
	end

	def setSeparater(sep)
		@sep = sep
	end

	def putpagetitle
		xml2pagetitle(@xmldata)
		#@xmldata
	end
	def output(sheet_number)
		sheet = @sheets[sheet_number]
		return output_sheet(sheet)
	end

	def output_sheet(sheet)
		contents = ""
		sheet.each{|row|
			row.each{|col|
				contents += sprintf("%s%s",col, @sep)
			}
			contents += sprintf("\n")
		}
		return contents
	end

	def output
    contents = ""
    loop = 0
    @sheets.each{|sheet|
      if @sheetno == -1
        contents += output_sheet(sheet)
      elsif @sheetno == loop
        contents += output_sheet(sheet)
      end
      loop=loop+1
		}
		#なぜか最初に空行が入ってしまうので対処。
		#本来的にはちゃんとデバッグする
		return contents.sub(/^\n/,"")
	end
end

page = nil

sep=' '  ##デフォルト
sheetno = -1
filename = ""
noget = false 

#オプションの解釈処理
ARGV.each{ |command|
  if noget
    sheetno = command.to_i
    noget = false
  else
    case command
    when "-tab"		then sep = '	'
    when "-t"		then sep = '	'
    when "-comma"	then sep = ','
    when "-c"		then sep = ','
    when "-n" then noget = true
    else
      filename = command
      ##when /\-.+/		then raise CommnandLineArgumentError, "無効なオプションです"
    end
  end
}

#raise CommnandLineArgumentError, "ファイル名を指定してください" if ARGV[0] == nil
#raise FileNotFoundException, "ファイル#{ARGV[0]}が見つかりません" unless FileTest.exists?(ARGV[0])

conv = XlsTextReader.new
conv.getXmlSheet(filename, sheetno)
conv.setSeparater(sep)
puts "#{conv.output}"
#puts "#{conv.putpagetitle}"

