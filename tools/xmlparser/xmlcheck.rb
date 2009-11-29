#  XMLがパーサーで読めるかを検査します。
#  読めれば、rexmlで構文がかけます
#   k.ohno  2009.03.27

require "rexml/document"
require "rexml/parseexception"
include REXML
require "kconv"

# XMLファイル名を入力するファンクション
def getXMLFileName

    fname = ""
    while fname == ""
        printf("XMLファイル名を入力してください: ")
        fname = $stdin.gets.strip
    end

    return fname

end

##### main処理です

# XMLファイル名を取得します
xml_file_name = ARGV[0]

if xml_file_name == nil
    xml_file_name = getXMLFileName
end

printf("--- xmlファイル名: %s\n", xml_file_name)

# XMLファイルをopenします
doc = nil

begin



    File.open(xml_file_name) {|xmlfile|
        doc = REXML::Document.new xmlfile
    }

        printf("---エラーはありませんでした\n")

# パースエラーが発生したとき
    rescue REXML::ParseException=>error
        printf("--- パースエラーです\n")
        printf("to_s=%s", error.to_s())

# ファイルの読み込みエラーなどが発生したとき
    rescue Exception=>error
        printf("--- エラー : %s", error)

end
