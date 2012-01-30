#################################################################################
#
# @brief  BG イベント 解析クラス
#
# @file   bg_event_parser.rb
# @author onoue_masayuki
# @date   2012.01.30
#
#################################################################################
class BGEvent

  # 検索キーワード
  @@keyward_SECTION_START         = "#SECTION START:BG_EVENT"
  @@keyward_SECTION_END           = "#SECTION END:BG_EVENT"
  @@keyward_event_number          = "#event number:"
  @@keyward_Event_Label           = "#Bg Event Label"
  @@keyward_Event_Type            = "#Bg Event Type"
  @@keyward_Bg_Event_Reaction_Direction_ID  = "#Bg Event Reaction Direction ID"
  @@keyward_Script_Name           = "#Bg Script Name"
  @@keyward_Pos_Type              = "#Pos Type"
  @@keyward_position              = "#position"

  # 初期化
  def initialize
    @event_number = 0
    @Event_Label  = nil
    @Event_Type   = 0
    @Script_Name  = nil
    @Pos_Type     = nil
    @position     = nil
    @Bg_Event_Reaction_Direction_ID = nil
  end

  # ゲッター
  attr_reader :event_number, :Event_Label, :Event_Type, :Script_Name, :Pos_Type, :position, :Bg_Event_Reaction_Direction_ID

  private
  #======================
  # 該当データを抽出する
  #======================
  def get_event_data( section_data, event_number ) 
    data = Array.new
    in_section = false # BGイベントのセクション内部かどうか
    in_event   = false # 指定されたイベントの内部かどうか

    section_data.each do |line|
      # イベントの内外を判定
      if in_section && in_event then 
        if line.include?( @@keyward_event_number ) then in_event = false; break end
      elsif in_section then
        if line =~ /#{@@keyward_SECTION_END}/ then in_section = false end
        if line =~ /#{@@keyward_event_number}\s*#{event_number}/ then in_event = true end
      else
        if line =~ /#{@@keyward_SECTION_START}/ then in_section = true end
      end 
      # イベント内データを抽出
      if in_section && in_event then data << line end
    end

    return data
  end

  public
  #-------------------
  # データを読み込む
  #-------------------
  def load( section_data, event_number )

    @event_number = event_number
    data = get_event_data( section_data, event_number )

    0.upto( data.size - 1 ) do |row|
      case
      when data[ row ].include?( @@keyward_Event_Label )             then @Event_Label             = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_Event_Type )             then @Event_Type             = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_Bg_Event_Reaction_Direction_ID )             then @Bg_Event_Reaction_Direction_ID             = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_Script_Name )             then @Script_Name             = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_Pos_Type )             then @Pos_Type             = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_position )             then @position             = data[ row+1 ].chomp
      end
    end
  end

end
