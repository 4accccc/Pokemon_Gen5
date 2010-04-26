#################################################################################
#
# @brief  OBJ イベント 解析クラス
#
# @file   obj_event_parser.rb
# @author obata
# @date   2010.04.24
#
#################################################################################
class OBJEvent

  # 検索キーワード
  @@keyward_SECTION_START         = "#SECTION START:OBJ_EVENT"
  @@keyward_SECTION_END           = "#SECTION END:OBJ_EVENT"
  @@keyward_event_number          = "#event number:"
  @@keyward_type                  = "#type"
  @@keyward_ID_name               = "#ID name"
  @@keyward_OBJ_CODE_Number       = "#OBJ CODE Number"
  @@keyward_MOVE_CODE_Number      = "#MOVE CODE Number"
  @@keyward_EVENT_TYPE_Number     = "#EVENT TYPE Number"
  @@keyward_Flag_Name             = "#Flag Name"
  @@keyward_Event_Script_Name     = "#Event Script Name"
  @@keyward_Direction_Type_Number = "#Direction Type Number"
  @@keyward_Parameter_0_Number    = "#Parameter 0 Number"
  @@keyward_Parameter_1_Number    = "#Parameter 1 Number"
  @@keyward_Parameter_2_Number    = "#Parameter 2 Number"
  @@keyward_Move_Limit_X_Number   = "#Move Limit X Number"
  @@keyward_Move_Limit_Z_Number   = "#Move Limit Z Number"
  @@keyward_Pos_Type              = "#Pos Type"
  @@keyward_position              = "#position"

  # 初期化
  def initialize
    @event_number          = 0
    @type                  = nil
    @ID_name               = nil
    @OBJ_CODE_Number       = nil
    @MOVE_CODE_Number      = nil
    @EVENT_TYPE_Number     = nil
    @Flag_Name             = nil
    @Event_Script_Name     = nil
    @Direction_Type_Number = nil
    @Parameter_0_Number    = nil
    @Parameter_1_Number    = nil
    @Parameter_2_Number    = nil
    @Move_Limit_X_Number   = nil
    @Move_Limit_Z_Number   = nil
    @Pos_Type              = nil
    @position              = nil
  end

  # ゲッター
  attr_reader :event_number, :type, :ID_name, 
              :OBJ_CODE_Number, :MOVE_CODE_Number, :EVENT_TYPE_Number, :Flag_Name, 
              :Event_Script_Name, :Direction_Type_Number, 
              :Parameter_0_N, :Parameter_1_Number, :Parameter_2_Number, 
              :Move_Limit_X_Number, :Move_Limit_Z_Number, :Pos_Type, :position

  private
  #======================
  # 該当データを抽出する
  #======================
  def get_event_data( section_data, event_number ) 
    data = Array.new
    in_section = false # OBJイベントのセクション内部かどうか
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
      when data[ row ].include?( @@keyward_type )                  then @type                  = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_ID_name )               then @ID_name               = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_OBJ_CODE_Number )       then @OBJ_CODE_Number       = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_MOVE_CODE_Number )      then @MOVE_CODE_Number      = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_EVENT_TYPE_Number )     then @EVENT_TYPE_Number     = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_Flag_Name )             then @Flag_Name             = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_Event_Script_Name )     then @Event_Script_Name     = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_Direction_Type_Number ) then @Direction_Type_Number = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_Parameter_0_Number )    then @Parameter_0_Number    = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_Parameter_1_Number )    then @Parameter_1_Number    = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_Parameter_2_Number )    then @Parameter_2_Number    = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_Move_Limit_X_Number )   then @Move_Limit_X_Number   = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_Move_Limit_Z_Number )   then @Move_Limit_Z_Number   = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_Pos_Type )              then @Pos_Type              = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_position )              then @position              = data[ row+1 ].chomp
      end
    end
  end

end
