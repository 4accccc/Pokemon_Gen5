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
require "bg_event_section_parser.rb"
require "wms_parser.rb"

class MEVFile

  # 初期化
  def initialize
    @filename     = nil                 # ファイル名
    @obj_section  = OBJEventSection.new # OBJイベントセクション
    @bg_section   = BGEventSection.new  # BGイベントセクション
    @wms_param    = WMSParam.new        # WMSパラメータ
  end

  # ゲッター
  attr_reader :path_to_file, :filename, :obj_section ,:bg_section ,:wms_param

  public
  #--------------------
  # ファイルを読み込む
  #--------------------
  def load( filename )
    @filename = filename
    @obj_section.load( filename )
    @bg_section.load( filename )
    @wms_param.load( @obj_section.wms_filename )
  end

end

