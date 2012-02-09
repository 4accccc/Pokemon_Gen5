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

# タグデータクラス
class MSTXT::TagData
  # アクセサー
  attr_accessor :index,:group,:tag_id,:param

  # データ初期化
  def initialize( index, group, id )
    # タグには必ずこのデータが存在している必要がある
    @index  = index
    @group  = group
    @tag_id = id
    @param  = Hash.new
  end

  # パラメータを登録する
  def add_param( id, param )
    if @param.key?(id) then 
      raise "パラメータを上書きすることはできません"
    end
    # パラメータ登録
    @param[id] = param
  end
  
end

# テキストデータクラス
class MSTXT::TextData
  # アクセサー
  attr_accessor :label,:jpn,:jpn_kanji ,:jpn_comment
  
  public
  # データ初期化
  def initialize()
    @label      = ""
    @jpn        = ""
    @jpn_kanji  = ""
    @jpn_tag_array    = nil
    @kanji_tag_array  = nil
    @jpn_comment      = ""
  end

  # ひらがな文章のタグを取得
  def jpn_tag
    # 未取得の場合は解析して返す
    if @jpn_tag_array == nil then
      @jpn_tag_array = get_tag_array( @jpn )
    end
    return @jpn_tag_array
  end

  # 漢字文章のタグを取得
  def kanji_tag
    # 未取得の場合は解析して返す
    if @kanji_tag_array == nil then
      @kanji_tag_array = get_tag_array( @jpn_kanji )
    end
    return @kanji_tag_array
  end

  private
  # タグを検索して、配列化する
  def get_tag_array( msg )
    tag_array = Array.new

    # 正規表現メモ:*?は最短位置
    scan_array = msg.scan(/\[.*?\]/)

    # タグ全てをパースする
    scan_array.each_with_index{|tag_str,index|

      # タグを分解($1=group,$2=id)
      if tag_str =~ /\[(\S+):(\S+).*\]/ then

        # タグデータを生成
        tag = MSTXT::TagData.new(index,$1,$2)

        # パラメータ部分を取得(group,id,最後のカッコを削除し、空白区切りに変換)
        param_list = tag_str.gsub(/\[\S+:\S+\s/,"").gsub(/\]/,"").split("\s")
        
        # パラメータを名前部分と値部分に分離する
        param_list.each{|param|
          # $1=id,$2=値
          if param =~ /(\S+)="(.*)"/ then
            tag.add_param($1,$2)
          end
        }
        tag_array << tag
      end
    }

    # 生成したタグ配列を返す
    return tag_array
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

      # コメントセル
      XPath.each(text_label,"CommentCell"){|comment_cell|

        # 言語判断
        language = comment_cell.attributes["Language"]

        # テキストの存在確認
        original_txt = XPath.first( comment_cell, "Comment" ).text
        if original_txt == nil then
          # messageが存在しない場合は空白を入れる
          original_txt = ""
        end

        # テキストデータ取得
        text = get_win31j( original_txt )

        # 言語ごとに保存
        case language
          when "JPN"
            txt_data.jpn_comment = text 
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

