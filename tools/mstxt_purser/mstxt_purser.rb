#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
# @file		mstxt_purser.rb
#	@brief  mstxtパーサー
#	@author	Masayuki Onoue
#	@date		2012.01.16
#
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

# ================================================================
# mstxt解析クラス
# MSTXT::Document.new(ファイル名)でファイル名を解析します
# 
# 配列として受け取りたい時はvaluesを使い、
# ハッシュとして受け取りたい時はtxt_hashを参照してください。
#
# txt_hashのキーはラベル名になっています。
#
# またテキストデータはMSTXT::TextDataのデータになっております。
#
# ◆その他仕様
# テキストが存在しない場合は、空文字列を出力しています
#
# 使用例----------------------------------------------------------
# test = MSTXT::Document.new("mailbox.mstxt")
# test.values.each{|data|
#  puts data.label
#  puts "hira :"  + data.jpn
#  puts "kanji:"  + data.jpn_kanji
# }
#
# ================================================================

require 'nkf'
require "rexml/document" 
include REXML

class MSTXT
end

# テキストデータクラス
class MSTXT::TextData
  # アクセサー
  attr_accessor :label,:jpn,:jpn_kanji

  # データ初期化
  def initialize()
    @label      = ""
    @jpn        = ""
    @jpn_kanji  = ""
  end
end

# mstxtデータ本体
class MSTXT::Document

  # アクセサー
  attr_reader :txt_hash

  # ドキュメント読み込み
  def initialize( filename )

    # データ初期化
    @txt_hash = Hash.new
    
    # ファイル読み込み
    f = open(filename)
    file_str = f.read
    f.close
    
    # xml解析
    xmldoc = REXML::Document.new( file_str )
    mstxt_elem = xmldoc.elements["MessageStudioText"]

    # xml解析
    xmldoc = REXML::Document.new( file_str )
    mstxt_elem = xmldoc.elements["MessageStudioText"]

    # 全ラベル取得
    XPath.each(mstxt_elem,"TextRow"){|text_label|
      
      # テキストデータ生成
      txt_data = MSTXT::TextData.new

      # ラベル取得
      txt_data.label = text_label.attributes["Label"]
      XPath.each(text_label,"LanguageCell"){|language_cell|

        # 言語判断
        language = language_cell.attributes["Language"]

        # テキストの存在確認
        original_txt = XPath.first( language_cell, "Text" ).text
        if original_txt == nil then
          # messageが存在しない場合は空白を入れる
          original_txt = ""
        end

        # テキストデータ取得
        text = get_win31j( original_txt )

        # 言語ごとに保存
        case language
          when "JPN"
            txt_data.jpn = text 
          when "JPN_KANJI"
            txt_data.jpn_kanji = text 
        end

      }

      # データ格納
      @txt_hash[txt_data.label] = txt_data
    }

  end
  
  # 文字コード変換
  def get_win31j(s)
    if ( RUBY_VERSION < "1.9" )
      return NKF::nkf( '-Wsm0', s )
    else
      return s.encode("Windows-31J")
    end
  end

  # 配列としてデータを受け取る
  def values
    return @txt_hash.values
  end

end

