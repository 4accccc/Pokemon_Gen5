####################################################################################
#
# ■brief:  タブ区切りファイル リーダー
# □file:   tab_separated_file_rader.rb
# □author: obata
# □date:   2010.02.10 # #
# ◆タブで区切られたエクセルデータを読み込み, 任意のセルの値を取得するためのクラス
#
####################################################################################


#===================================================================================
# ■タブ区切りファイル リーダー
#===================================================================================
class TabSeparatedFileReader

  def initialize
    @fileName = nil  # ファイル名
    @fileRows = nil  # ファイルの各行
  end

  #------------------------------
  # □brief: ファイルを読み込む
  # □param: fileName ファイル名
  def ReadFile( fileName )
    # ファイルが存在しない
    if File.exist?( fileName ) == FALSE then abort( "#{fileName} is not exist" ) end
    # 読み込む
    file      = File.open( fileName, "r" )
    @fileRows = file.readlines
    @fileName = fileName
    file.close
  end

  #------------------------------------
  # □brief:  値を取得する
  # □param:  rowIdx    行インデックス
  # □param:  columnIdx 列インデックス
  # □return: 指定したセルの値
  def GetCell( rowIdx, columnIdx )
    # 開いていない
    if @fileName == nil then abort( "file is not opened" ) end 
    # 指定位置のデータを返す
    contents = @fileRows[ rowIdx ].split(/\t/)
    return contents[ columnIdx ]
  end

  #------------------------------------
  # □brief:  行数を取得する
  # □return: 読み込んだファイルの行数
  def GetRowNum
    return @fileRows.size
  end

end 
