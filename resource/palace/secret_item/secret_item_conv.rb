=begin
実行手順
ruby secret_item_conv.rb [CSVファイルへのパス]

出力ファイル
intrude_secret_item.cdat
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
class SECRET_DATA
  def initialize()
    @no = "0"                  #配置場所ID
    @grid_x = "0"              #グリッドX
    @grid_y = "0"              #グリッドY
    @grid_z = "0"              #グリッドZ
    @zone_id = "0"             #ゾーンID
  end

  attr_accessor :no, :grid_x, :grid_y, :grid_z, :zone_id;
end


SecretData = [];
$secret_data_max = 0;


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
    if ((/^配置場所ID/ =~ line[0]) != nil)
      title += 1;
      next;
    end
    if(title == 0)
      next;
    end

    cell = 0;
    SecretData[s] = SECRET_DATA.new();

    SecretData[s].no = line[cell];
    cell += 1;
    SecretData[s].grid_x = line[cell];
    cell += 1;
    SecretData[s].grid_y = line[cell];
    cell += 1;
    SecretData[s].grid_z = line[cell];
    cell += 1;
    SecretData[s].zone_id = line[cell];
    cell += 1;

    s += 1;
    $secret_data_max = s;
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

end


=begin
//==================================================================
/**
 * 読み込んだファイルをCデータとして出力
 */
//==================================================================
=end
def DataFileOutput()
  
  File.open("intrude_secret_item.cdat", "w"){|file|
    file.printf("//============================================================\n");
    file.printf("//       secret_item_conv.rb で出力されたファイルです\n");
    file.printf("//============================================================\n\n");
    file.printf("///侵入隠しアイテム配置データ\n");
    file.printf("const INTRUDE_SECRET_ITEM_POSDATA IntrudeSecretItemPosDataTbl[] = {\n");

    for i in 0..SecretData.size-1
      file.printf("\t{%s, %s, %s, %s},\t\t//%d\n", SecretData[i].grid_x, SecretData[i].grid_y, SecretData[i].grid_z, SecretData[i].zone_id, i);
    end
    
    file.printf("};\n");
  }

  File.open("intrude_secret_item_def.h", "w"){|file|
    file.printf("//============================================================\n");
    file.printf("//       secret_item_conv.rb で出力されたファイルです\n");
    file.printf("//============================================================\n\n");
    file.printf("#pragma once\n\n");
    
    file.printf("///IntrudeSecretItemPosDataTblのテーブル数\n");
    file.printf("#define SECRET_ITEM_DATA_TBL_MAX\t\t\t%d\n", $secret_data_max);
  }
  
end


#==============================================================================
#  メイン
#==============================================================================
CsvConvFileCheck();
DataConv();
DataFileOutput();
