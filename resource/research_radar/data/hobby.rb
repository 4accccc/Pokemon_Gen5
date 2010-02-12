####################################################################################
#
# ■brief:  趣味オブジェクト
# □file:   hobby.rb
# □author: obata
# □date:   2010.02.12
#
####################################################################################


#===================================================================================
# ■趣味
#===================================================================================
class Hobby

  def initialize
    @ID              = 0    # ID
    @ID_lavel        = nil  # ID (ラベル名)
    @stringJPN       = nil  # 趣味名 (かな)
    @stringJPN_KANJI = nil  # 趣味名 (漢字)
  end

  attr_reader :ID, :ID_lavel, :stringJPN, :stringJPN_KANJI

  #-----------------------------
  # □brief: IDを設定する
  # □param: id    ID
  # □param: lavel ID (ラベル名)
  def SetID( id, lavel )
    @ID       = id
    @ID_lavel = lavel
  end

  #-------------------------------
  # □brief: 趣味名を設定する
  # □param: stringJPN       かな
  # □param: stringJPN_KANJI 漢字
  def SetString( stringJPN, stringJPN_KANJI )
    @stringJPN       = stringJPN
    @stringJPN_KANJI = stringJPN_KANJI
  end

  #-----------------------------------------
  # □brief: デバッグ情報を出力する
  # □param: directory 出力先のディレクトリ
  def OutputDebug( directory )
    # 出力データ作成
    outData = Array.new
    outData << "ID              = #{@ID}"
    outData << "ID_lavel        = #{@ID_lavel}"
    outData << "stringJPN       = #{@stringJPN}"
    outData << "stringJPN_KANJI = #{@stringJPN_KANJI}"
    # 出力
    fileName = directory + "/hobby_#{@ID}.txt"
    file = File.open( fileName, "w" )
    file.puts( outData )
    file.close
  end
  
end
