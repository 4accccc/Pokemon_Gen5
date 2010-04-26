#################################################################################
#
# @brief  .mev ファイル解析クラス
#
# @file   obj_event_parser.rb
# @author obata
# @date   2010.04.24
#
#################################################################################
require "obj_event_section_parser.rb"

class MEVFile

  # 初期化
  def initialize
    @filename     = nil                 # ファイル名
    @obj_section  = OBJEventSection.new # OBJイベントセクション
  end

  # ゲッター
  attr_reader :path_to_file, :filename, :obj_section

  public
  #--------------------
  # ファイルを読み込む
  #--------------------
  def load( filename )
    @filename = filename
    @obj_section.load( filename )
  end

end
