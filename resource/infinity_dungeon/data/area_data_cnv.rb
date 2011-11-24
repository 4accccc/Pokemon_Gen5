#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
# 
# 	@file		area_data_cnv.rb
# 	@brief  エリアデータを生成する
# 	@author	Masayuki Onoue
# 	@date		2011.11.18
# 
# 
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

require "csv"
require "room_type_hash"
require "room_member_num_hash"

# 構造体定義
$SAreaData = Struct.new("AREA_DATA",:data_end ,:fs_x ,:fs_y ,:area_line ,:fnum ,:room_num ,:member_num,:max_tr_num,:min_tr_num)

# =========================================
# エリアデータ解析クラス
class AREA_PURSER
  attr_accessor :area_no, :room_data ,:area_size
  def initialize( data )
    # データの初期化
    @all_area_data = data
    @area_data = Array.new
    @area_size = 0
    
    # エリアサイズ
    get_area_data()

    @area_size.times{|area_id|
      puts "エリア%d:%s×%s"%[area_id,@area_data[area_id].fs_x,@area_data[area_id].fs_x]
      puts "%d階建て"%[@area_data[area_id].fnum]

      @area_data[area_id].fnum.times{|i|
        puts "部屋人数%d人"%[@area_data[area_id].member_num[i]]

        if i > 0 then
          if @area_data[area_id].member_num[0] != @area_data[area_id].member_num[i] then
            raise "エリア%dの階ごとのトレーナー人数が異なっています。\n"%[area_id+1]
          end
        end

        # 部屋数カウント
        room_sum = 0
        $room_type_hash.keys.size.times{|room_type_index|
          room_sum += @area_data[area_id].room_num[i][room_type_index]
        }

        # 最大部屋数-エレベータ部屋
        if room_sum != ((@area_data[area_id].fs_x*@area_data[area_id].fs_y)-1) then
          raise "エリア%d:部屋人数の指定が間違っています\n"%[area_id+1]
        end

        # 最大人数チェック
        if @area_data[area_id].member_num[i] > 32 then
          raise "エリア%d:32人制限を越えています\n"%[area_id+1]
        end
      }
      printf "\n"
    }
    
  end

  def get_area_data()

    @all_area_data.each_with_index{|line,index|
      case index
      when 0,2 # ヘッダ行
        next
      when 1 # タイトル行
        # エリア数取得
        @area_size = line.size/3

        # エリアワーク作成、フロアサイズ取得
        @area_size.times{|area_id|
          area_line = area_id*3
          area_size = line[area_line+1].scan(/(\d)×(\d)/)

          @area_data[area_id] = $SAreaData.new
          @area_data[area_id].area_line = area_line
          @area_data[area_id].fs_x = area_size[0][0].to_i
          @area_data[area_id].fs_y = area_size[0][1].to_i
          @area_data[area_id].fnum = 0
          @area_data[area_id].room_num = Array.new
          @area_data[area_id].member_num = Array.new
          @area_data[area_id].max_tr_num = Array.new
          @area_data[area_id].min_tr_num = Array.new

        }
      else
        # フロアデータ解析
        @area_size.times{|area_id|
          area_line = @area_data[area_id].area_line

          if !line[area_line].nil? then
            if line[area_line].include?("#END") then
              @area_data[area_id].data_end = true
            end
          end

          # データが終わっている場合はスキップ
          if @area_data[area_id].data_end == true then
            next
          end

          # 階数カウント
          if !line[area_line].nil? then
            room_num_index = @area_data[area_id].fnum
            @area_data[area_id].room_num[ room_num_index ] = Array.new
            @area_data[area_id].member_num[ room_num_index ] = 0
            @area_data[area_id].max_tr_num[ room_num_index ] = 0
            @area_data[area_id].min_tr_num[ room_num_index ] = 0xfff
            # 部屋数の初期化
            $room_type_hash.keys.each_with_index{|key,i|
              @area_data[area_id].room_num[room_num_index][i] = 0
            }
            @area_data[area_id].fnum+=1 
          end

          # 部屋タイプチェック
          room_type = line[area_line+1]
          if !$room_type_hash.key?(room_type) then
            raise "存在しない部屋タイプ:%s"%room_type
          end
           
          # 部屋数保存
          fnum = @area_data[area_id].fnum
          room_index = $room_type_hash[room_type]
          @area_data[area_id].room_num[fnum-1][room_index] += line[area_line+2].to_i
          room_member_num = $room_member_hash[room_type].to_i
          @area_data[area_id].member_num[fnum-1] +=  room_member_num* @area_data[area_id].room_num[fnum-1][room_index]

          # メンバー数最小
          if @area_data[area_id].min_tr_num[fnum-1] > room_member_num then
            @area_data[area_id].min_tr_num[fnum-1] =room_member_num
          end
          # メンバー数最大
          if @area_data[area_id].max_tr_num[fnum-1] < room_member_num then
            @area_data[area_id].max_tr_num[fnum-1] =room_member_num
          end
        }
      end
    }
  end

  # フロア数データ出力
  def out_fnum out_data
    @area_size.times{|area_id|
      out_data.puts "#define INFINITYDUNGEON_FLOOR_AREA%d_MAX (%d)"%[area_id+1,@area_data[area_id].fnum]
    }
    out_data.printf "\n"
  end
  
  # エリアデータ出力
  def out_area_data out_data
    @area_size.times{|area_id|
      out_data.print "  { // エリア%d\n"%[area_id+1]
      @area_data[area_id].fnum.times{|i|
        out_data.print "    { "
        $room_type_hash.keys.size.times{|room_type_index|
          out_data.print "%d,"%[@area_data[area_id].room_num[i][room_type_index]]
        }
        out_data.print "},\n"
      }
      out_data.print "  },\n"
    }

  end
  
  # フロアサイズデータ出力
  def out_floor_size out_data
    @area_size.times{|area_id|
      out_data.puts " { %d ,%d }, // エリア%d"%[@area_data[area_id].fs_x,@area_data[area_id].fs_x,area_id+1]
    }
  end
  
  # エリアトレーナー人数ハッシュ生成
  def out_area_member_num out_data
    @area_size.times{|area_id|
      b_room_max = 0
      old_b_room_max = 0
      @area_data[area_id].fnum.times{|i|
        mm = @area_data[area_id].member_num[i]
        max = @area_data[area_id].max_tr_num[i]
        min = @area_data[area_id].min_tr_num[i]
        # 合計値
        mm_sum = mm
        # ボス部屋時最大人数
        b_room_max = ((mm_sum-min)+(max-min))
        puts "エリア%d:%d-------------"%[area_id,i]
        puts "最大:%d"%max
        puts "最小:%d"%min
        puts "単純合計:%d"%mm_sum
        puts "ボス部屋最大値:%d"%b_room_max
        if b_room_max > 32 then
          raise "エリア%d-%d階の人数が32人を超えています"%b_room_max
        end

        # 人数チェック
        if i > 0 then
          if b_room_max != old_b_room_max then
            raise "エリア%dの階ごとのトレーナー人数が異なっています。\n"%[area_id+1]
          end
        else
          if mm_sum > b_room_max then
            out_data.puts '  %2d, # エリア %d'%[mm_sum,area_id+1]
          else
            out_data.puts '  %2d, # エリア %d'%[b_room_max,area_id+1]
          end
        end
        old_b_room_max = b_room_max

      }
      print "\n"
    }
  end
   
end

# =========================================
# メイン
W_AREA_CSV_FILENAME   = ARGV[0]
B_AREA_CSV_FILENAME   = ARGV[1]
OUT_FILENAME          = ARGV[2]
FLOOR_NUM_FILENAME = ARGV[3]
FLOOR_SIZE_CDAT_FILENAME = ARGV[4]

# データの読み込み
w_data_csv = CSV.open( W_AREA_CSV_FILENAME ,"r")
b_data_csv = CSV.open( B_AREA_CSV_FILENAME ,"r")

w_area_data = AREA_PURSER.new( w_data_csv )
b_area_data = AREA_PURSER.new( b_data_csv )

header =  <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		%s
 *	@brief  無限ダンジョンフロア数データ
 *	@author	Masayuki Onoue
 *	@date		2011.11.18
 *  @note   コンバータによって出力されたファイルです
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

EOS

out_data = open( FLOOR_NUM_FILENAME ,"w")
out_data.puts header%[FLOOR_NUM_FILENAME]
w_area_data.out_fnum( out_data )
out_data.close

header =  <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		%s
 *	@brief  無限ダンジョンフロアデータ
 *	@author	Masayuki Onoue
 *	@date		2011.11.18
 *  @note   コンバータによって出力されたファイルです
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// ホワイトデータ
static const u8 sc_floor_data_w[INFINITY_DUNGEON_AREA_ID_MAX][INFINITYDUNGEON_FLOOR_MAX][%d] =
{
EOS

out_data = open( OUT_FILENAME ,"w")
out_data.puts header%[OUT_FILENAME,$room_type_hash.keys.size]
w_area_data.out_area_data( out_data )
out_data.puts "};"


header =  <<EOS
// ブラックデータデータ
static const u8 sc_floor_data_b[INFINITY_DUNGEON_AREA_ID_MAX][INFINITYDUNGEON_FLOOR_MAX][%d] =
{
EOS
out_data.puts header%[$room_type_hash.keys.size]
b_area_data.out_area_data( out_data )
out_data.puts "};"
out_data.close


header =  <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		%s
 *	@brief  無限ダンジョンフロアサイズデータ
 *	@author	Masayuki Onoue
 *	@date		2011.11.18
 *  @note   コンバータによって出力されたファイルです
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

//-------------------------------------
// 各エリアのフロアサイズ
//=====================================
static const struct 
{
  int fs_x;
  int fs_y;
}sc_floor_size[INFINITY_DUNGEON_AREA_ID_MAX] =
{
EOS

out_data = open( FLOOR_SIZE_CDAT_FILENAME ,"w")
out_data.puts header%[FLOOR_SIZE_CDAT_FILENAME]
w_area_data.out_floor_size( out_data )
out_data.puts "};"
out_data.close

header =  <<EOS
#/[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#**
#*
#*	@file		%s
#*	@brief  無限ダンジョンフロアごとのトレーナー人数ハッシュ(ボス部屋配置時最大)
#*	@author	Masayuki Onoue
#*	@date		2011.11.24
#*  @note   コンバータによって出力されたファイルです
#*
#*/
#/]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

$area_tr_member_num_w = [
EOS

filename = "area_tr_member_num.rb"
out_data = open( filename ,"w")
out_data.puts header%[filename]
w_area_data.out_area_member_num( out_data )
out_data.puts "]"


header =  <<EOS
$area_tr_member_num_b = [
EOS
out_data.puts header
b_area_data.out_area_member_num( out_data )
out_data.puts "]"
out_data.close

