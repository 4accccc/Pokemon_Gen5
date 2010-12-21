#======================================================================
#	fmmdl_scripter_mdllist
#	スクリプター向けモデルリスト作成
#======================================================================

# インクルードパスの生成
lib_path = File.dirname(__FILE__).gsub(/\\/,"/") + "/../../../tools/char2bmp/char2bmp"
require 'win32ole'
require 'date'
require lib_path
load "rbdefine"

if ENV['PROJECT_DOC_ROOT'] == nil then
  ENV['PROJECT_DOC_ROOT'] = "C:/home/swan_doc/"
end

XLS_FILENAME  = ENV['PROJECT_DOC_ROOT']+'仕様書/フィールド/動作モデル一覧.xls'
PIC_DIR       = ENV['PROJECT_ROOT']+'resource/fldmmdl/mmdl_pict/'
CHARA_DIR     = ENV['PROJECT_ROOT']+'resource/fldmmdl/mmdl_nchara/'
POS_DATA_COM  = 3 # ポスデータに使用されているコメント行の数

#====================================
# データパーサ
def GetFldmmdlData( file_name , out )
  file_listcsv = File.open( file_name, "r" );

  # データ一覧を取得
  file_listcsv.each_with_index{|line,index|
    # 読み飛ばし(コメント行として出力しているので、コメントアウト)
    #if index == 0 then
    #  next
    #end

    # 終端の取得
    if line.include?("#END") then
      break
    end

    # データの追加
    line = line.split( "," )
    out << line
  }

  file_listcsv.close
end

#====================================
# データ取得関数
def GetData( csv_data , data_num , index )
  data = csv_data[data_num]
  return data[index]
end

#====================================
# 切り取りデータを取得する
def GetBbdCodePos( fname , pos_data)
  
  # ファイルから切り取り位置を取得する
  csv = File.open( fname, "r" );
  csv.each_with_index{|line,index|

    # 読み飛ばし
    if index <= POS_DATA_COM then
      next
    end
    if line.include?("#END") then
      break
    end

    # データをハッシュに登録する
    data = line.split( "," )
    if data[0].size > 0  then
      x = data[1].to_i
      y = data[2].to_i
      pos_data[ data[0] ]  =  [ x , y ]
    end
  }
  csv.close

end

#====================================
# ポイント値をピクセルに変換(エクセルのセル幅をピクセルで指定するときに使用)
def PointToPixcel(pix)
  return  (((pix * 72 / 96) - 3.75) / 6)
end

#====================================
# セルに文字列を書き込む
# A1を指定するには column = 0 ,row = 0
def PutCellStr( sheet , column , row , data ,border = false )
  
  # 列はAからスタート
  colstr = "A"
  column.times{
    colstr.succ!
  }

  # 行は１からスタート
  row+=1

  pos = colstr+row.to_s
  cell = sheet.Range(pos)
  cell.value = data
  cell.Font.Size = 10

  if border == true then
    borders = cell.Borders
    borders.LineStyle = 1
  end
end

#====================================
# 切り取りデータを取得する
def GetClipData( data , bbd_code_pos )
  retdata = { "x"=>0 , "y"=>0 , "Width"=>0 , "Height"=>0 }

  # 切り抜きデータ定義
  # 左下が0,0であることに注意
  pos_data = {
    "PROC_NON"            =>[0,0],
    "PROC_BLACT"          =>[8*12,8*28],
    "PROC_BLACT_ALANM4"   =>[8*12,8*28],
    "PROC_BLACT_ALANM16"  =>[8*12,8*28],
    "PROC_BLACT_ALANM32"  =>[8*12,8*28],
    "PROC_BLACT_ONELOOP"  =>[8*12,8*28], 
    "PROC_HERO"           =>[32*3,8*28],
    "PROC_CYCLEHERO"      =>[32*5,8*24],
    "PROC_SWIMHERO"       =>[8*4,8*28],
    "PROC_HERO_ITEMGET"   =>[8*4,8*28],
    "PROC_HERO_PCAZUKE"   =>[0,8*28],
    "PROC_ACT0"           =>[8*12,8*28],
    "PROC_POKE"           =>[0,0],          # ポケモンは違うシート
    "PROC_POKE_FLY"       =>[0,0],          # ポケモンは違うシート
    "PROC_MODEL"          =>[0,0],          # ポリゴンモデルは別シート
    "PROC_FISHINGHERO"    =>[8*24,8*16],
    "PROC_YUREHERO"       =>[8*16,8*28],
    "PROC_SHIN_MU"        =>[0,8*56],
    "PROC_SPIDER"         =>[0,8*28],
    "PROC_MELODYER"       =>[0,8*28],
    "PROC_ACT0_NON_PAUSE" =>[8*12,8*28],
  }

  # アニメIDから場所が決まるパターン(幅×下記の値が使用される)
  anm_id_pos = {
    "ANMID_FLIP"    =>2,
    "ANMID_ONECELL" =>0,
  }

  # 幅の取得
  if data[RBDEF_NUM_BBD_MDLSIZE] =~ /MDLSIZE_([0-9]+)x([0-9]+)/ then
    retdata["Width"] = $1.to_i
    retdata["Height"]= $2.to_i
  end

  # 場所の取得
  proc_data = data[RBDEF_NUM_BBD_DRAWPROC]
  if pos_data.key?(proc_data) then
    retdata["x"] = pos_data[proc_data][0].to_i
    retdata["y"]= pos_data[proc_data][1].to_i
  end

  # フリップパターンの設定
  amd_id = data[RBDEF_NUM_BBD_ANMID]
  if anm_id_pos.key?(amd_id) then
    retdata["x"] = retdata["Width"].to_i*anm_id_pos[amd_id].to_i
  end

  # 表示コードでの設定
  bbd_code = data[RBDEF_NUM_BBD_CODE]
  if bbd_code_pos.key?(bbd_code) then
    retdata["x"] = bbd_code_pos[bbd_code][0]
    retdata["y"] = bbd_code_pos[bbd_code][1]
  end

  return retdata
end

#====================================
# 既にあるファイルからコメントデータハッシュを作成する
def MakeCommentHash( filename , data , xl )
  book = xl.Workbooks.Open(filename)
  sheet = book.Worksheets.Item(1)

  i = 1
  end_code = ""
  until end_code == "#END" do
    # データの取得
    comment = sheet.Range("F"+i.to_s).value
    end_code = sheet.Range("A"+i.to_s).value
    bbd_code = sheet.Range("D"+i.to_s).value
    # ハッシュへのデータ追加
    unless comment == nil then
      data[bbd_code] = comment
    end
    i+=1
  end
  book.Close
end

#====================================
# ヘッダー行を取得する
def GetHeaderData( filename , header , xl , x, y )
  book = xl.Workbooks.Open(filename)
  sheet = book.Worksheets.Item(1)

  # 指定範囲のデータをハッシュにつめる
  row = 1
  x.times{
    col = "A"
    y.times{
      pos = col+row.to_s
      header[pos] = sheet.Range(col+row.to_s).value
      col.succ!
    }
    row+=1
  }
  book.Close
end

#===========================================================================
# メイン処理
#===========================================================================
FLDMMDL_FILENAME  = ARGV[0]  # 動作モデルリストファイル名
CUTPOS_FILENAME   = ARGV[1]  # 切り取り位置定義ファイル
HEADER_ROW_NUM    = 3 # ヘッダー行のサイズ

# 表示行
index_data =[
  RBDEF_NUM_CODENAME,
  RBDEF_NUM_COMMENT,
  RBDEF_NUM_BBD_CODE,
  RBDEF_NUM_BBD_ANMTYPE,
]

# エクセル操作開始
xl = WIN32OLE.new('Excel.Application')

# ファイルが存在する場合はコメントハッシュを作成し、ファイルを削除する
comment = Hash.new
header = Hash.new
if FileTest.exist?(XLS_FILENAME) then
  MakeCommentHash(XLS_FILENAME,comment,xl)
  GetHeaderData(XLS_FILENAME,header,xl,HEADER_ROW_NUM,10)
  File.delete(XLS_FILENAME)
end

# データ取得
csv_data = Array.new
GetFldmmdlData( FLDMMDL_FILENAME , csv_data )

# ファイル生成
book = xl.Workbooks.Add
sheet = book.Worksheets.Item(1)


# その他特殊パターン(表示コードで、x,y座標を指定できる)
bbd_code_pos = Hash.new
GetBbdCodePos( CUTPOS_FILENAME , bbd_code_pos)

csv_data.each_with_index{|data,index|
  
  # 画像データファイルパスを取得
  bbd_code = data[RBDEF_NUM_BBD_CODE].downcase
  pic_path = PIC_DIR + bbd_code + ".bmp"
  ncg_path = CHARA_DIR + bbd_code + ".ncg"
  ncl_path = CHARA_DIR + bbd_code + ".ncl"

  # セルのY座標
  cell_y = index + HEADER_ROW_NUM

  # セルへの画像書き込み
  unless bbd_code == "nondraw" then

    # リソースファイルが存在するか調べる
    if FileTest.exist?(ncg_path) then

      # 画像の切り出しデータを取得する
      clip = GetClipData( data , bbd_code_pos )

      # ビットマップの生成
      bmp = Bitmap.new
      bmp.setdata( pic_path , ncg_path , ncl_path )

      # データのコンバート
      bmp.dataconvert

      # キャラクターデータのコンバート
      bmp.cutdata(clip["x"],clip["y"],clip["Width"],clip["Height"])

      # 書き込み
      bmp.write

      #puts pic_path
      img = sheet.Pictures.Insert(pic_path).Cut

      # 画像の書き込み
      pos = "A"+(cell_y+1).to_s
      cell = sheet.Range(pos)
      cell.value = ""
      cell.RowHeight = clip["Height"]*0.75
      cell.ColumnWidth  = PointToPixcel(64)
      cell.PasteSpecial
      cell.Font.Size = 10
 
    else
      # 0行目はタイトル行なので表示しない
      unless cell_y == HEADER_ROW_NUM then
        # 画像の書き込み
        pos = "A"+(cell_y+1).to_s
        cell = sheet.Range(pos)
        cell.value = "(画像無し)"
        cell.Font.Size = 10
      end
    end
  else
    # 0行目はタイトル行なので表示しない
    unless cell_y == HEADER_ROW_NUM then
      # 画像の書き込み
      pos = "A"+(cell_y+1).to_s
      cell = sheet.Range(pos)
      cell.value = "(画像無し)"
      cell.Font.Size = 10
    end
  end

  # データ部分の書き込み
  index_data.each_with_index{|data_id,i|
    PutCellStr( sheet , i+1 , cell_y  , data[data_id] , true )
  }

  # コメントの書き込み
  if comment.key?(data[RBDEF_NUM_BBD_CODE]) then
    PutCellStr( sheet , index_data.size+1 , cell_y  , comment[data[RBDEF_NUM_BBD_CODE]] , true )
  else
    PutCellStr( sheet , index_data.size+1 , cell_y  , "" , true )
  end

  # デバッグ
  #if cell_y == HEADER_ROW_NUM + 5 then
  #break
  #end
}

# 終了文字列を次の行に書き込んでおく
PutCellStr( sheet , 0 , csv_data.size + HEADER_ROW_NUM  , '#END'  )

# コメント行にタイトルをつける
PutCellStr( sheet , index_data.size+1 , HEADER_ROW_NUM  , '配置上の注意' )

# オートフィットを適応
sheet.Columns("B:F").AutoFit

# ヘッダーデータを書き込む
header.keys.each{|key|
  sheet.Range(key).value = header[key]
}

# 日付の出力
PutCellStr( sheet , 5 , 0  , Time.now.strftime("%Y-%m-%d %H:%M:%S") )

# ファイルの終了
begin
  book.SaveAs(XLS_FILENAME)
  book.Close
  xl.Quit
rescue
  xl.Quit
end

# ファイル生成管理用ダミーファイル生成
#f = open("out_end","w")
#f.close

puts "スクリプター向けモデルリスト作成しました"
