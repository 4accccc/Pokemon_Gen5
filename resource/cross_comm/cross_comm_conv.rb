=begin
実行手順
ruby cross_comm_conv.rb [CSVファイルへのパス]

出力ファイル
cross_comm.cdat
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
    @No = "0"             #No
    @ID = "0"             #ID
    @comment = "0"        #詳細
    @pri = "0"            #プライオリティ
    @life = "0"           #送信寿命
  end

  attr_accessor :No, :ID, :comment, :pri, :life;
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

    ZoneData[s].No = line[cell];
    cell += 1;
    ZoneData[s].ID = line[cell];
    cell += 1;
    ZoneData[s].comment = line[cell].sub(/\.0/, ""); #csvの時点で小数点がついているので取り除く
    cell += 1;
    ZoneData[s].pri = line[cell].sub(/\.0/, "");
    cell += 1;
    ZoneData[s].life = line[cell].sub(/\.0/, "");
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
end


=begin
//==================================================================
/**
 * 読み込んだファイルをCデータとして出力
 */
//==================================================================
=end
def DataFileOutput()
  
  File.open("cross_comm.cdat", "w"){|file|
    file.printf("//============================================================\n");
    file.printf("//       cross_comm_conv.rb で出力されたファイルです\n");
    file.printf("//============================================================\n\n");
    file.printf("static const struct{\n\tu32 priority:8;\n\tu32 life:24;\n}CrossCommData[] = {\n");

    for i in 0..ZoneData.size-1
      file.printf("\t{\t//%d  %s(%s)\n", i, ZoneData[i].comment, ZoneData[i].ID);
      file.printf("\t\t%s,\t\t//priority\n", ZoneData[i].pri);
      file.printf("\t\t%s * 60,\t\t//life\n", ZoneData[i].life);
      file.printf("\t},\n");
    end
    
    file.printf("};\n");
  }
  
end


#==============================================================================
#  メイン
#==============================================================================
CsvConvFileCheck();
DataConv();
DataFileOutput();
