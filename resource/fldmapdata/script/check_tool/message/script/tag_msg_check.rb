#################################################################################
# @brief  タグに登録が必要なメッセージを使用している箇所をリストアップ
# @file   tag_msg_check.rb
# @author onoue_masayuki
# @date   2012.02.03
################################################################################# 
require 'csv'
require ENV['PROJECT_ROOT']+"tools/mstxt_purser/"+"mstxt_purser.rb"

# 文字コード指定
$KCODE = 'SJIS'

# 出力データ
class CheckData
  attr_reader :filename, :index, :line_text, :msg_data ,:near_line
  def initialize( filename, index, line_text, msg_data,near_line )
    @filename  = filename
    @index      = index
    @line_text  = line_text
    @msg_data   = msg_data
    @near_line  = near_line
  end
end

# メッセージデータ
class MessageData
  attr_reader :filename ,:txt ,:idx
  def initialize( filename ,text ,tag_data )
    @filename = filename
    @txt = text

    # タグ情報を格納(未登録の時は自動的に0)
    if tag_data.param.key?("Idx" ) then
      @idx = tag_data.param["Idx"].to_i
    else
      @idx = 0
    end

  end
end

# チェック対象の .ev ファイル
EV_FILES = ENV['PROJECT_RSCDIR']+"fldmapdata/script/*.ev" 
#EV_FILES = ENV['PROJECT_RSCDIR']+"fldmapdata/script/c0*.ev" 
# メッセージファイル
MSTXT_FILES = ENV['PROJECT_RSCDIR']+"message/msproj/src/**/*.mstxt" 
#MSTXT_FILES = ENV['PROJECT_RSCDIR']+"message/msproj/src/script_message/c03*.mstxt" 

#--------------------------------------------------------------------------------
# ■すべてのメッセージIDを取得
#--------------------------------------------------------------------------------
search_tag_list = Array.new
mstxt_list = Dir[ MSTXT_FILES ]
mstxt_list.each_with_index{|filename,index|
  puts '%s(%d/%d)'%[File.basename(filename),index,mstxt_list.size]
  mstxt_data = MSTXT::Document.new(filename)

  # Name系ラベルが登録されいるものを調べる
  mstxt_data.values.each{|data|
    data.jpn_tag.each{|tag_data|
      if tag_data.group == "Name" then
        if tag_data.tag_id == "TrainerName" then
          search_tag_list << MessageData.new(filename,data,tag_data)
        end
      end
    }
  }
}

#--------------------------------------------------------------------------------
# ■すべての使用されているメッセージIDを取得
#--------------------------------------------------------------------------------
check_data_array = Array.new
ev_list = Dir[ EV_FILES ]
ev_list.each_with_index{|filename,index| 
  puts '%s(%d/%d)'%[File.basename(filename),index,ev_list.size]
  
  # 検索
  ev_file   = open(filename,"r")
  ev_lines  = ev_file.readlines
  ev_lines.each_with_index{|ev_line,line_index|
    search_tag_list.each_with_index{|text_data,index|
      if ev_line.include?(text_data.txt.label) then

        # 近くの行を表示抜き出す
        near_line_index_s = line_index-10
        if near_line_index_s < 0 then
          near_line_index_s = 0
        end
        near_line = Array.new
        for i in near_line_index_s..line_index
          near_line << ev_lines[i].strip+"\n"
        end

        check_data_array << CheckData.new( filename, (line_index+1) ,ev_line.strip ,text_data ,near_line)


      end
    }
  }
  ev_file.close
}

#--------------------------------------------------------------------------------
# ■解析結果を出力
#--------------------------------------------------------------------------------
CSV.open("tag_msg_check_result.csv","w"){|writer|
  writer << ["EVファイル名","行番号","mstxt名","ラベル","idx","コメント","スクリプト内容","近辺のライン","平仮名文","漢字文"]
  check_data_array.each{|data|
    writer << [File.basename(data.filename),data.index,File.basename(data.msg_data.filename),data.msg_data.txt.label,data.msg_data.idx,data.msg_data.txt.jpn_comment,data.line_text,data.near_line,data.msg_data.txt.jpn,data.msg_data.txt.jpn_kanji]
  }
  writer << ["チェック必要数:%d"%[check_data_array.size]]
}
puts "チェック必要数:%d"%[check_data_array.size]

