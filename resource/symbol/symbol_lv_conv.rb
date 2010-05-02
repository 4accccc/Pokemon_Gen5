=begin
実行手順
ruby cross_comm_conv.rb [CSVファイルへのパス]

出力ファイル
lv_tbl.bin
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
class SYMBOL_DATA
  def initialize()
    @no = "0"             #No
    @mons_name = "0"      #ポケモン名
    @level = "0"          #レベル
  end

  attr_accessor :no, :mons_name, :level;
end


SymbolData = [];

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
    SymbolData[s] = SYMBOL_DATA.new();

    SymbolData[s].no = line[cell];
    cell += 1;
    SymbolData[s].mons_name = line[cell];
    cell += 1;
    SymbolData[s].level = line[cell].sub(/\.0/, ""); #csvの時点で小数点がついているので取り除く
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
 * 読み込んだファイルをバイナリとして出力
 */
//==================================================================
=end
def DataFileOutput()
  
  File.open("lv_tbl.bin", "wb"){|file|
    first_data = 0  #1origin用のダミーデータ
    file.write([first_data].pack("C"))
    for i in 0..SymbolData.size-1
      file.write([SymbolData[i].level.to_i].pack("C"))
    end
  }
  
end


#==============================================================================
#  メイン
#==============================================================================
CsvConvFileCheck();
DataConv();
DataFileOutput();
