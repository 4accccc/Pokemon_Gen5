=begin
実行手順
ruby palace_zone_conv.rb [CSVファイルへのパス]

出力ファイル
palace_zone_setting.cdat
=end


#--------------------------------------------------------------
#  定数定義
#--------------------------------------------------------------
OBJID_MAX = 4;
DATA_MAX = 3;
REWARD_MAX = 4;

#--------------------------------------------------------------
#  グローバル変数
#--------------------------------------------------------------
InputExcelFile = ARGV[0];


#--------------------------------------------------------------
#  ミッションデータ
#--------------------------------------------------------------
class ZONE_DATA
  def initialize()
    @zone_id = "0"             #ZONE_ID
    @reverse_zone_id = "0"     #裏ZONE_ID
    @warp_zone_id = "0"        #WARP_ZONE_ID
    @warp_grid_x = "0"         #WARPグリッドX
    @warp_grid_y = "0"         #WARPグリッドY
    @warp_grid_z = "0"         #WARPグリッドZ
    @sub_x = "0"               #下画面X
    @sub_y = "0"               #下画面Y
    @connect = "0"             #パレスとの接続可否
    @plyaer_update = "0"       #通信プレイヤーの更新有
    @comment = "0"             #備考欄
  end

  attr_accessor :zone_id, :reverse_zone_id, :warp_zone_id;
  attr_accessor :warp_grid_x, :warp_grid_y, :warp_grid_z;
  attr_accessor :sub_x, :sub_y, :connect, :player_update, :comment;
end


ZoneData = [];

=begin
//==================================================================
/**
 * Excel(カンマ区切り)ファイルを開き行単位でチェック
 *
 * @param   sp_array		
 *
 * @retval  def		
 */
//==================================================================
=end
def CsvConvFileCheck()
  
  s = 0;
  title = 0;
  
  require 'csv'
  csv = CSV.open(InputExcelFile, 'r');
  csv.each_with_index{|line, i|
    if ((/^#END/ =~ line[0]) != nil)
      break;
    end
    if(title == 0)
      title += 1;
      next;
    end

    cell = 0;
    ZoneData[s] = ZONE_DATA.new();

    ZoneData[s].zone_id = line[cell];
    cell += 1;
    ZoneData[s].reverse_zone_id = line[cell];
    cell += 1;
    ZoneData[s].warp_zone_id = line[cell];
    cell += 1;
    ZoneData[s].warp_grid_x = line[cell].sub(/\.0/, ""); #csvの時点で小数点がついているので取り除く
    cell += 1;
    ZoneData[s].warp_grid_y = line[cell].sub(/\.0/, "");
    cell += 1;
    ZoneData[s].warp_grid_z = line[cell].sub(/\.0/, "");
    cell += 1;
    ZoneData[s].sub_x = line[cell].sub(/\.0/, "");
    cell += 1;
    ZoneData[s].sub_y = line[cell].sub(/\.0/, "");
    cell += 1;
    ZoneData[s].connect = line[cell];
    cell += 1;
    ZoneData[s].player_update = line[cell];
    cell += 1;
    ZoneData[s].comment = line[cell];
    cell += 1;

    s += 1;
  }
  
end


=begin
//==================================================================
/**
 * 読み込んだファイルを出力形式に合わせて変換
 */
//==================================================================
=end
def DataConv()
  for i in 0..ZoneData.size-1
    ZoneData[i].zone_id = "ZONE_ID_" + ZoneData[i].zone_id;
    if(ZoneData[i].reverse_zone_id == "-")
      ZoneData[i].reverse_zone_id = "ZONE_ID_MAX";
    else
      ZoneData[i].reverse_zone_id = "ZONE_ID_" + ZoneData[i].reverse_zone_id;
    end
    ZoneData[i].warp_zone_id = "ZONE_ID_" + ZoneData[i].warp_zone_id;
  end
end


=begin
//==================================================================
/**
 * 読み込んだファイルをCデータとして出力
 */
//==================================================================
=end
def DataFileOutput()
  
  File.open("palace_zone_setting.cdat", "w"){|file|
    file.printf("//============================================================\n");
    file.printf("//       palace_zone_conv.rb で出力されたファイルです\n");
    file.printf("//============================================================\n\n");
    file.printf("const PALACE_ZONE_SETTING PalaceZoneSetting[] = {\n");

    for i in 0..ZoneData.size-1
      file.printf("\t{\t//%d  %s\n", i, ZoneData[i].comment);
      file.printf("\t\t%s,\t\t//zone_id\n", ZoneData[i].zone_id);
      file.printf("\t\t%s,\t\t//reverse_zone_id\n", ZoneData[i].reverse_zone_id);
      file.printf("\t\t%s,\t\t//warp_zone_id\n", ZoneData[i].warp_zone_id);
      file.printf("\t\t%s,\t\t//warp_grid_x\n", ZoneData[i].warp_grid_x);
      file.printf("\t\t%s,\t\t//warp_grid_y\n", ZoneData[i].warp_grid_y);
      file.printf("\t\t%s,\t\t//warp_grid_z\n", ZoneData[i].warp_grid_z);
      file.printf("\t\t%s,\t\t//sub_x\n", ZoneData[i].sub_x);
      file.printf("\t\t%s,\t\t//sub_y\n", ZoneData[i].sub_y);
      file.printf("\t},\n");
    end
    
    file.printf("};\n");
  }

  File.open("palace_zone_id_tbl.cdat", "w"){|file|
    file.printf("//============================================================\n");
    file.printf("//       palace_zone_conv.rb で出力されたファイルです\n");
    file.printf("//============================================================\n\n");
    file.printf("ALIGN4 const u16 PalaceZoneIDTbl[] = {\n");

    for i in 0..ZoneData.size-1
      if (ZoneData[i].connect == "○")
        file.printf("\t%s,\n", ZoneData[i].zone_id);
      end
    end
    
    file.printf("};\n");

    update_count = 0;
    file.printf("\n\n\n//---------------------------------\n");
    file.printf("//  通信プレイヤー更新有効マップ\n");
    file.printf("//---------------------------------\n");
    file.printf("ALIGN4 const u16 PalaceZoneIDTbl_PlayerUpdate[] = {\n");
    for i in 0..ZoneData.size-1
      if (ZoneData[i].player_update == "○")
        file.printf("\t%s,\n", ZoneData[i].zone_id);
        update_count = update_count + 1;
      end
    end
    file.printf("};\n");
    file.printf("\nconst u32 PalaceZoneIDTbl_PlayerUpdate_TblCount = %d;\n", update_count);
  }
  
end


#==============================================================================
#  メイン
#==============================================================================
CsvConvFileCheck();
DataConv();
DataFileOutput();
