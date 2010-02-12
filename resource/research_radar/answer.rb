####################################################################################
#
# ■brief:  回答オブジェクト定義
# □file:   answer.rb
# □author: obata
# □date:   2010.02.12
#
####################################################################################


#===================================================================================
# ■回答
#===================================================================================
class Answer

  def initialize
    @ID              = 0    # 回答ID
    @stringJPN       = nil  # 回答文字列 (かな)
    @stringJPN_KANJI = nil  # 回答文字列 (漢字)
    @color_lavel     = nil  # 表示色 (ラベル 0x000000-0xffffff)
    @colorR          = 0    # 表示色 (赤 0-31)
    @colorG          = 0    # 表示色 (緑 0-31)
    @colorB          = 0    # 表示色 (青 0-31)
  end

  attr_reader :ID, :stringJPN, :stringJPN_KANJI, :color_lavel, :colorR, :colorG, :colorB

  #----------------------------
  # □brief: 回答IDを設定する
  # □param: id 回答ID
  def SetID( id )
    @ID = id
  end

  #--------------------------------------------
  # □brief: 回答文字列を設定する
  # □param: stringJPN       回答文字列 (かな)
  # □param: stringJPN_KANJI 回答文字列 (漢字)
  def SetString( stringJPN, stringJPN_KANJI )
    @stringJPN       = stringJPN 
    @stringJPN_KANJI = stringJPN_KANJI 
  end

  #---------------------------------------------------
  # □brief: 表示色を設定する
  # □param: color_lavel 表示色(ラベル 000000-ffffff)
  def SetColor( color_lavel )
    red   = ( color_lavel.hex & 0xff0000 ) >> 16
    green = ( color_lavel.hex & 0x00ff00 ) >> 8
    blue  = ( color_lavel.hex & 0x0000ff )
    red   = 0x1f * red   / 0xff
    green = 0x1f * green / 0xff
    blue  = 0x1f * blue  / 0xff
    @colorR = red
    @colorG = green
    @colorB = blue
    @color_lavel = color_lavel
  end

  #-----------------------------------------
  # □brief: デバッグ情報を出力する
  # □param: directory 出力先ディレクトリ名
  def OutputDebug( directory )
    # 出力データ作成
    outData = Array.new
    outData << "ID              = #{@ID}"
    outData << "stringJPN       = #{@stringJPN}"
    outData << "stringJPN_KANJI = #{@stringJPN_KANJI}"
    outData << "color           = #{@color_lavel}(%d %d %d)" % [@colorR, @colorG, @colorB]
    # 出力
    fileName = directory + "/answer_#{@ID}.txt"
    file = File.open( fileName, "w" )
    file.puts( outData )
    file.close
  end

end
