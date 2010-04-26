#################################################################################
#
# @brief  OBJ イベントセクション 解析クラス
#
# @file   obj_event_section_parser.rb
# @author obata
# @date   2010.04.24
#
#################################################################################
require "obj_event_parser.rb"


class OBJEventSection

  # 検索キーワード
  @@keyward_SECTION_START = "#SECTION START:OBJ_EVENT"
  @@keyward_SECTION_END = "#SECTION End:OBJ_EVENT"
  @@keyward_hold_event_number = "#hold event number:"
  @@keyward_hold_event_type = "#hold event type:"

  # 初期化
  def initialize
    @hold_event_number = 0
    @hold_event_type = 0
    @event = Array.new
  end

  # ゲッター
  attr_reader :hold_event_number, :hold_event_type, :event

  private
  #=========================================
  # OBJイベントセクションのデータを抽出する
  #=========================================
  def get_section_data( filename )
    file = File.open( filename, "r" )
    lines = file.readlines
    file.close()

    section_data = Array.new
    in_section = false

    lines.each do |line|
      if in_section then
        if line =~ /#{@@keyward_SECTION_END}/ then in_section = false end
      else
        if line =~ /#{@@keyward_SECTION_START}/ then in_section = true end
      end

      if in_section then section_data << line end
    end

    return section_data
  end

  public
  #------------------
  # データを読み込む
  #------------------
  def load( filename )

    section_data = get_section_data( filename )

    0.upto( section_data.size - 1 ) do |row|
      case
      when section_data[ row ].include?( @@keyward_hold_event_number ) then @hold_event_number = section_data[ row+1 ].to_i
      when section_data[ row ].include?( @@keyward_hold_event_type ) then @hold_event_type = section_data[ row+1 ]
      end
    end

    0.upto( @hold_event_number - 1 ) do |event_number|
      event = OBJEvent.new
      event.load( section_data, event_number )
      @event << event
    end
  end

end
