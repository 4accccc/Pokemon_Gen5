
#----------------------------------------------------------------------------------
# @brief  アイテム名を取得する
# @param  label ラベル名
# @return 指定ラベルのアイテム名
#----------------------------------------------------------------------------------
def GetItemName( label )

	file = File::open("itemsym.h", "r");
	file_line = file.readlines
	file.close

	file_line.each do |line|
		if line =~ /#define.*#{label}.*\/\/(.*)/ then 
			return $1.strip # 前後の空白を削除
		end
	end

	abort("error: アイテム(#{label})は定義されていません。")
end

#----------------------------------------------------------------------------------
# @brief  ゾーンIDを取得する
# @param  filename .ev ファイル名
# @return ゾーンID
#----------------------------------------------------------------------------------
def GetZoneID_byEvFileName( filename )
  zone_id = File.basename( filename, ".ev" )
  zone_id = zone_id.upcase
  return zone_id
end


class ItemInfo

  def initialize
    @file_name  = nil
    @zone_id    = nil
    @zone_name  = nil
    @item_label = nil
    @item_name  = nil
  end

  attr_accessor :file_name, :zone_id, :zone_name, :item_label, :item_name
end
