#################################################################################
#
# @brief  wms 解析クラス
#
# @file   wms_parser.rb
# @author onoue_masayuki
# @date   2012.01.30
#
#################################################################################
class WMSParam

  # 検索キーワード
  @@keyward_Width           = "#Width"
  @@keyward_Height          = "#Height"
  @@keyward_block_x_offset  = "#block x offset"
  @@keyward_block_z_offset  = "#block z offset"
  @@keyward_block_size      = "#block size"

  # 初期化
  def initialize
    @Width = 0
    @Height = 0
    @x_offset = 0
    @z_offset = 0
    @block_size = ""
  end

  # ゲッター
  attr_reader :Width, :Height, :x_offset, :z_offset, :block_size

  public
  #-------------------
  # データを読み込む
  #-------------------
  def load( filename )

    # ファイルを配列化
    wms_file = open(filename,"r")
    data = wms_file.readlines
    wms_file.close

    data.each_with_index do |line,row|
      case
      when data[ row ].include?( @@keyward_Width )          then @Width             = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_Height )         then @Height             = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_block_x_offset ) then @x_offset             = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_block_z_offset ) then @z_offset             = data[ row+1 ].chomp
      when data[ row ].include?( @@keyward_block_size )     then @block_size             = data[ row+1 ].chomp
      end
    end
  end

end
