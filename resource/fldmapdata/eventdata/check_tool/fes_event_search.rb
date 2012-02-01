#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#   @file		  fes_event_search.rb
#   @brief    mevを解析してフェスイベントのリストを作成する
#   @author		Masayuki Onoue
#   @date		  2011.01.30
#
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

$KCODE = "SJIS"
require "mev_parser.rb"
require 'csv'

#ポジションタイプ定数
EVENTDATA_POS_TYPE_GRID = "EVENTDATA_POSTYPE_GRID"
EVENTDATA_POS_TYPE_RAIL = "EVENTDATA_POSTYPE_RAIL"

# チェックするmevのディレクトリ
PATH_TO_DIR     = "../data/"
TARGET_FILENAME = "*.mev"

# 結果出力先
result_file = ARGV[0]

# イベントデータ型定義
$fes_event = Struct.new("FesData", :filename, :id ,:type ,:x,:y,:z ,"is_rail" )

# フェスイベントを設定する
def set_fes_event( dst ,filename ,event ,mev_data)
  
  # 位置を取得
  pos = event.position.split("\s")
 
  # ブロックサイズ取得
  bs = mev_data.wms_param.block_size.split("\s")
  
  if event.Pos_Type.include?( EVENTDATA_POS_TYPE_GRID ) then
    # ブロックオフセット(Mayaではブロックの中心が０になっている)
    x_ofxs = (bs[0].to_i)/2
    z_ofxs = (bs[1].to_i)/2

    # ワールド座標に変換
    pos[0] = ((pos[0].to_f-8)/16).round + x_ofxs + mev_data.wms_param.x_offset.to_i*32
    pos[1] = (pos[1].to_i/16).round
    pos[2] = z_ofxs - ((pos[2].to_f+8)/16).round + mev_data.wms_param.z_offset.to_i*32
    # グリッドデータを設定
    dst << $fes_event.new( filename ,event.Event_Label ,event.Event_Type ,pos[0],pos[1],pos[2] ,"×" )
  

  elsif event.Pos_Type.include?( EVENTDATA_POS_TYPE_RAIL ) then
    
    # レールデータ
    dst << $fes_event.new( filename ,event.Event_Label ,event.Event_Type ,pos[0],pos[1],pos[2],"○" )
  end
end


# fesデータ配列
event_list = Array.new


filenames = Dir[ PATH_TO_DIR + TARGET_FILENAME ]
filenames.each{|filename|
  puts filename
  mev_data = MEVFile.new
  mev_data.load( filename )
 
  # =====================================
  # fesデータ収集
  mev_data.bg_section.event.each{|event|
    case event.Event_Type
      when "BG_TALK_TYPE_DUMMY_FES" then
        set_fes_event(event_list,filename,event,mev_data)
      when "BG_TALK_TYPE_DUMMY_FES_NPC" then
        set_fes_event(event_list,filename,event,mev_data)
    end
  }

}
 
# =====================================
# データ出力
CSV.open(result_file, 'w') do |writer|
  writer << [ "ゾーン名", "ID"  ,"場所" ,"BGイベントタイプ","レール" ]
  # 検索結果出力
  event_list.each{|data|
    pos_str = "%d %d %d"%[data.x,data.y,data.z]
    writer << [ File.basename(data.filename,".mev"), data.id ,pos_str ,data.type,data.is_rail ]
  }
    writer << [ "イベント総数：%d"%[event_list.size] ]
end

puts "イベント総数：%d"%[event_list.size]

