#!ruby -Ks
#!/usr/bin/ruby

#===================================================================
#
# @brief  フェスミッション ミッション用アイテムデータテーブル生成コンバーター
#
# @data   11.09.14 
# @author miyuki iwasawa
#
#===================================================================

#====================================================
# 設定
#====================================================
# 文字コード指定


$KCODE = 'SJIS'

require "../../tools/hash/item_hash.rb"
require "tool/cdef_perser.rb"

#====================================================
# 定数宣言
#====================================================
#ヘッダファイル出力
DST_BRIEF_ITEM_DAT = "フェス アイテムミッション用アイテムデータテーブル"
DST_BRIEF_ITEM_HDAT = "フェス アイテムミッション用アイテムデータテーブル ヘッダ"
DST_BRIEF_NPC_DAT = "フェス NPCミッション用アイテムデータテーブル"
DST_BRIEF_NPC_HDAT = "フェス NPCミッション用アイテムデータテーブル ヘッダ"
DST_AUTOR = "miyuki iwasawa"
DST_NOTE = "このファイルはresource/fmission_data/tool/fmission_item_conv.rbによって自動生成されたものです。"

KIRA_ITEM_SET_NUM = 6

TRADE_ITEM_TBL_MAX = 3
TRADE_ITEM_CATEGORY_MAX = 4
TRADE_ITEM_RANK_MAX = 6

SHOPPING_ITEM_TYPE_MAX = 5
SHOPPING_ITEM_LIST_MAX = 16

GPRESENT_ITEM_TYPE_MAX = 2
GPRESENT_ITEM_DATA_MAX = 6

TRAINER_SET_NUM = 6

PRESENT_ITEM_DATA_NAME = "const u16 DATA_NpcItemPresent[]"
TRADE_ITEM_DATA_NAME = "const NITEM_TRADE_DATA DATA_NpcItemTrade[]"
SHOPPING_ITEM_DATA_PROB_NAME = "static const u8 DATA_NpcItemShoppingProb[]" 
SHOPPING_ITEM_DATA_NAME = "const NITEM_SHOPPING_DATA DATA_NpcItemShopping[]" 
GPRESENT_ITEM_DATA_NAME = "const NITEM_GET_A_PRESENT_DATA DATA_NpcItemGetAPresent[]"
TRAINER_DATA_NAME = ("const u16 DATA_NpcTrainer[][FES_NPC_TRAINER_SET_NUM]")

arg = $*

$fpath_out_item_cdat = arg[0]
$fpath_out_item_hdat = arg[1]
$fpath_out_npc_cdat = arg[2]
$fpath_out_npc_hdat = arg[3]
$fpath_in_csv   = arg[4]

#構造体定義

$_kira_item = Struct.new("KiraItem",\
  :item, :per, :name )

$_kira_item_list = Struct.new("KiraItemList",\
  :list_name, :list )

$_npc_present_item = Struct.new("NpcPresentItem",\
  :item, :name )

$_npc_trade_item = Struct.new("NpcTradeItem",\
  :item, :name )

$_npc_shopping_item = Struct.new("NpcShoppingItem",\
  :item, :price, :name, :per )

$_npc_pcwoman_item = Struct.new("NpcPCWomanItem",\
  :item, :per, :name )

$_npc_trainer_data = Struct.new("NpcTrainerData",\
  :list )

#====================================================
# 主処理 開始
#====================================================
class CItemTbl

  def initialize
  end

  def line_get fp
    line = fp.gets
#    print(line)
    if line == nil then
      return nil
    end
    line.chomp!
    return line.split(",")
  end
  
  def print_err err_str, word
    print("Error! #{err_str} -> #{word}\n")
    exit 1
  end

  def make_item_list f_path
    File::open( f_path ,"r"){ |fp|
      while line = fp.gets
        line.chomp!
        print (line+"\n")
        if line.slice(/#END/) != nil then
          break
        end
        work = line.split(",")
        case work[0]
        when "#LIST0"
          get_kira_item( fp, "#LIST0_END" )
        when "#LIST1"
          get_npc_present_item( fp, "#LIST1_END" )
        when "#LIST2"
          get_npc_trade_item( fp )
        when "#LIST3"
          get_npc_shopping_item( fp )
        when "#LIST4"
          get_npc_pcwoman_item( fp, "#LIST4_END")
        when "#LIST5"
          get_npc_trainer_data( fp, "#LIST5_END")
        end
      end
    }
  end
  
  def get_kira_item  fp, end_key
    @kira_item_list = Array.new

    while(1)
      work = line_get( fp )
      if work[0] != nil && work[0] == end_key then
        break
      end
      @kira_item_list << $_kira_item_list.new
      cp = @kira_item_list.last()
      cp.list_name = work[0]
      cp.list = Array.new
      total_per = 0
      for i in 0..(KIRA_ITEM_SET_NUM-1) do
        cp.list << $_kira_item.new
        idx = i*2+1
        item_name = work[idx+0] 
        item = $item_hash.fetch( item_name, nil )
        if item == nil then
          print_err("道具探しアイテム名が不正です", item_name ) 
        end
        prm = cp.list.last()
        prm.item = item
        prm.per = work[idx+1].to_i
        prm.name = item_name

        total_per += prm.per
      end
      if total_per != 100 then
        print_err("道具探しアイテム 確率合計が100にならない", cp.list_name )
      end
    end
  end

  def output_kira_item fp
    fp.puts("static const FES_ITEM_SET DATA_ItemSetTbl[FMISSION_ITEM_SET_MAX] = {")
    for n in @kira_item_list do
      name_buf = "  //%s -> " % [n.list_name]
      item_buf = "  {{ "
      for prm in n.list do
        name_buf += "%s," % [prm.name]
        item_buf += "{ %s, %d }," % [prm.item, prm.per ]
      end
      name_buf += "\n"
      item_buf += " }},\n"
      fp.puts name_buf
      fp.puts item_buf
    end
    fp.puts("};\n\n")
  end

  def get_npc_present_item  fp, end_key
    @npc_present_list = Array.new

    while(1)
      work = line_get( fp )
      if work[0] != nil && work[0] == end_key then
        break
      end
      item = $item_hash.fetch( work[1], nil )
      if item == nil then
        print_err("道具渡しアイテム名が不正です", work[1] ) 
      end
      @npc_present_list << $_npc_present_item.new
      cp = @npc_present_list.last()
      cp.item = item
      cp.name = work[1]
    end
  end
  
  def output_npc_present_item  fp
    fp.printf("%s = {\n", PRESENT_ITEM_DATA_NAME)
    
    for n in @npc_present_list do
      fp.puts("  #{ n.item },  ///<#{n.name}\n")
    end

    fp.puts("};\n\n")
  end

  def get_npc_trade_item fp
    @npc_trade_list = Array.new( TRADE_ITEM_TBL_MAX )

    for i in 0..(TRADE_ITEM_TBL_MAX-1) do
      #二元配列確保
      @npc_trade_list[i] = Array.new( TRADE_ITEM_CATEGORY_MAX ).map!{
        Array.new( TRADE_ITEM_RANK_MAX, 0 )
      }
      cp = @npc_trade_list[i]

      for j in 0..(TRADE_ITEM_CATEGORY_MAX-1) do
        work = line_get( fp )
        for k in 0..(TRADE_ITEM_RANK_MAX-1) do
          name = work[k+1]
          item = $item_hash.fetch( name, nil )
          if item == nil then
            print_err("交換アイテムリスト アイテム名エラー", name )
          end
          cp[j][k] = $_npc_trade_item.new
          cp[j][k].item = item
          cp[j][k].name = name
        end
      end
    end
  end

  def output_npc_trade_item fp
    fp.printf("%s = {\n", TRADE_ITEM_DATA_NAME)
   
    for cp in @npc_trade_list do
      fp.puts("  {{\n")
      for j in 0..(TRADE_ITEM_CATEGORY_MAX-1) do
        item_buf = ""
        name_buf = ""
        for k in 0..(TRADE_ITEM_RANK_MAX-1) do
          item_buf += cp[j][k].item.to_s
          item_buf += ","
          name_buf += cp[j][k].name
          name_buf += ","
        end
        fp.puts("    //#{name_buf}\n")
        fp.puts("    { #{item_buf} },\n")
      end
      fp.puts("  }},\n")
    end
    fp.puts("};\n\n")
  end

  def get_npc_shopping_item fp
    @npc_shopping_list = Array.new( SHOPPING_ITEM_TYPE_MAX ).map!{
      Array.new( SHOPPING_ITEM_LIST_MAX )
    }
    for i in 0..(SHOPPING_ITEM_TYPE_MAX-1) do
      total_per = 0
      for j in 0..(SHOPPING_ITEM_LIST_MAX-1) do
        work = line_get( fp )
        @npc_shopping_list[i][j] = $_npc_shopping_item.new
        name = work[1]
        item = $item_hash.fetch( name, nil )
        if item == nil then
          print_err("販売アイテムリスト アイテム名エラー", name )
        end
        @npc_shopping_list[i][j].item = item
        @npc_shopping_list[i][j].per = work[0].to_i
        @npc_shopping_list[i][j].name = name
        @npc_shopping_list[i][j].price = work[2].to_i

        total_per +=  @npc_shopping_list[i][j].per
      end
      if total_per != 100 then
        print_err("販売アイテム 確率合計が100にならない", i.to_s )
      end
    end
  end

  def output_npc_shopping_item fp

    fp.printf("%s = {\n",SHOPPING_ITEM_DATA_PROB_NAME)
    for i in 0..(SHOPPING_ITEM_LIST_MAX-1)do
      fp.printf(" %d,",@npc_shopping_list[0][i].per )
    end
    fp.puts("\n};\n\n")
    
    fp.printf("%s = {\n",SHOPPING_ITEM_DATA_NAME)
    
    for i in 0..(SHOPPING_ITEM_TYPE_MAX-1) do
      fp.puts("  {{\n")
      for j in 0..(SHOPPING_ITEM_LIST_MAX-1) do
        item = @npc_shopping_list[i][j]
        fp.puts("   { #{item.item}, #{item.price} },  ///<#{item.name}\n")
      end
      fp.puts("  }},\n")
    end
    fp.puts("};\n\n")
  end

  def get_npc_pcwoman_item fp,end_key
    @npc_pcwoman_list = Array.new( GPRESENT_ITEM_TYPE_MAX ).map!{
      Array.new( GPRESENT_ITEM_DATA_MAX )
    }
    for i in 0..( GPRESENT_ITEM_TYPE_MAX-1) do
      total_per = 0
      for j in 0..( GPRESENT_ITEM_DATA_MAX-1) do
        work = line_get( fp )
        @npc_pcwoman_list[i][j] = $_npc_pcwoman_item.new
        name = work[1]
        item = $item_hash.fetch( name, nil )
        if item == nil then
          print_err("販売アイテムリスト アイテム名エラー", name )
        end
        @npc_pcwoman_list[i][j].item = item
        @npc_pcwoman_list[i][j].per = work[2].to_i
        @npc_pcwoman_list[i][j].name = name

        total_per +=  @npc_pcwoman_list[i][j].per
      end
      if total_per != 100 then
        print_err("プレゼントアイテム 確率合計が100にならない", i.to_s )
      end
    end
=begin
    @npc_pcwoman_list = Array.new
    total_per = 0
    while(1)
      work = line_get( fp )
      if work[0] != nil && work[0] == end_key then
        break
      end
      item = $item_hash.fetch( work[1], nil )
      if item == nil then
        print_err("プレゼントNPCアイテム名が不正です", work[1] ) 
      end
      @npc_pcwoman_list << $_npc_pcwoman_item.new
      cp = @npc_pcwoman_list.last()
      cp.item = item
      cp.name = work[1]
      cp.per = work[2].to_i
      total_per += cp.per
    end
    if total_per != 100 then
      print_err("プレゼントNPCアイテム 確率合計が100にならない", total_per )
    end
=end
  end

  def output_npc_pcwoman_item fp
    fp.printf("%s = {\n",GPRESENT_ITEM_DATA_NAME)
    for i in 0..(GPRESENT_ITEM_TYPE_MAX-1) do
      fp.puts("  {{\n")
      for j in 0..(GPRESENT_ITEM_DATA_MAX-1) do
        item = @npc_pcwoman_list[i][j]
        fp.puts("   { #{item.item}, #{item.per} },  ///<#{item.name}\n")
      end
      fp.puts("  }},\n")
    end
    fp.puts("};\n\n")

=begin
    for n in @npc_pcwoman_list do
      fp.puts("  { #{n.item}, #{n.per} },  ///<#{n.name}\n")
    end

    fp.puts("};\n\n")
=end

  end

  def get_npc_trainer_data fp, end_key
    @npc_trainer_list = Array.new
    
    while(1)
      work = line_get( fp )
      if work[0] != nil && work[0] == end_key then
        break
      end
      @npc_trainer_list << $_npc_trainer_data.new
      cp = @npc_trainer_list.last()
      cp.list = Array.new(TRAINER_SET_NUM)

      for i in 0..(TRAINER_SET_NUM-1) do
        cp.list[i] = work[i+1]
      end
    end
  end

  def output_npc_trainer_data fp

    fp.printf("%s = {\n",TRAINER_DATA_NAME)

    for n in @npc_trainer_list do
      fp.print("  { ")
      for i in 0..(TRAINER_SET_NUM-1) do
        fp.printf("%s, ", n.list[i])
      end
      fp.print("},\n")
    end
    fp.puts("};\n\n")
  end

  def output_item_dat f_path
    File::open( f_path, "w" ){  |file|
      
      file.puts("//=========================================================================");
      file.puts("/**");
      file.puts(" * @file\t#{File.basename(f_path)}");
      file.puts(" * @brief\t" + DST_BRIEF_ITEM_DAT);
      file.puts(" * @autor\t" + DST_AUTOR);
      file.puts(" * @note\t" + DST_NOTE);
      file.puts(" */");
      file.puts("//=========================================================================\n");
      file.puts("\n\n");
      
      output_kira_item( file )
    }
  end
  
  def output_item_hdat f_path
    File::open( f_path, "w" ){  |file|
      
      file.puts("//=========================================================================");
      file.puts("/**");
      file.puts(" * @file\t#{File.basename(f_path)}");
      file.puts(" * @brief\t" + DST_BRIEF_ITEM_HDAT);
      file.puts(" * @autor\t" + DST_AUTOR);
      file.puts(" * @note\t" + DST_NOTE);
      file.puts(" */");
      file.puts("//=========================================================================\n");
      file.puts("\n\n");
  
      code = "__H_%s_H__" % [File.basename(f_path,".h").upcase()]
      file.puts("#ifndef #{code}")
      file.puts("#define #{code}\n\n")

      file.puts("#ifndef __ASM_NO_DEF_")
      file.puts("#endif\n\n")
      
      file.printf("#define FES_KIRA_ITEM_PARAM_NUM   ( %s )\n\n",KIRA_ITEM_SET_NUM)
      
      file.puts("\n\n#endif //#{code}\n\n")
    }
  end

  def output_npc_dat f_path
    File::open( f_path, "w" ){  |file|
      
      file.puts("//=========================================================================");
      file.puts("/**");
      file.puts(" * @file\t#{File.basename(f_path)}");
      file.puts(" * @brief\t" + DST_BRIEF_NPC_DAT);
      file.puts(" * @autor\t" + DST_AUTOR);
      file.puts(" * @note\t" + DST_NOTE);
      file.puts(" */");
      file.puts("//=========================================================================\n");
      file.puts("\n\n");
    
      output_npc_present_item( file )
      output_npc_trade_item( file )
      output_npc_shopping_item( file )
      output_npc_pcwoman_item( file )
      output_npc_trainer_data( file )
    }
  end
  
  def output_npc_hdat f_path
    File::open( f_path, "w" ){  |file|
      
      file.puts("//=========================================================================");
      file.puts("/**");
      file.puts(" * @file\t#{File.basename(f_path)}");
      file.puts(" * @brief\t" + DST_BRIEF_NPC_HDAT);
      file.puts(" * @autor\t" + DST_AUTOR);
      file.puts(" * @note\t" + DST_NOTE);
      file.puts(" */");
      file.puts("//=========================================================================\n");
      file.puts("\n\n");
  
      code = "__H_%s_H__" % [File.basename(f_path,".h").upcase()]
      file.puts("#ifndef #{code}")
      file.puts("#define #{code}\n\n")
      
      file.printf("#define FES_NITEM_PRESENT_DATA_MAX    ( %d )  ///<プレゼントアイテムデータ要素数\n\n",@npc_present_list.length())
      
      file.printf("#define FES_NITEM_TRADE_DATA_MAX      ( %s )\n",TRADE_ITEM_TBL_MAX)
      file.printf("#define FES_NITEM_TRADE_CATEGORY_MAX  ( %s )\n",TRADE_ITEM_CATEGORY_MAX)
      file.printf("#define FES_NITEM_TRADE_RANK_MAX      ( %s )\n\n",TRADE_ITEM_RANK_MAX)

      file.printf("#define FES_NITEM_SHOPPING_DATA_MAX   ( %s )\n",SHOPPING_ITEM_TYPE_MAX)
      file.printf("#define FES_NITEM_SHOPPING_LIST_MAX   ( %s )\n\n",SHOPPING_ITEM_LIST_MAX)

      file.printf("#define FES_NITEM_GET_A_PRESENT_DATA_MAX  ( %d )\n",GPRESENT_ITEM_TYPE_MAX )
      file.printf("#define FES_NITEM_GET_A_PRESENT_LIST_MAX  ( %d )\n\n",GPRESENT_ITEM_DATA_MAX )
      
      file.printf("#define FES_NPC_TRAINER_SET_NUM   ( %d )\n", TRAINER_SET_NUM )
      file.printf("#define FES_NPC_TRAINER_DATA_MAX  ( %d )\n", @npc_trainer_list.length())

      file.puts("\n\n#ifndef __ASM_NO_DEF_\n\n")
      file.puts("typedef struct _NITEM_TRADE_DATA{")
      file.puts("  u16 item[FES_NITEM_TRADE_CATEGORY_MAX][FES_NITEM_TRADE_RANK_MAX];")
      file.puts("}NITEM_TRADE_DATA;")
      file.puts("\n\n")
      file.puts("typedef struct _NITEM_SHOPPING_PARAM{")
      file.puts("  u16 item;")
      file.puts("  u16 price;")
      file.puts("}NITEM_SHOPPING_PARAM;")
      file.puts("\n\n")
      file.puts("typedef struct _NITEM_SHOPPING_DATA{")
      file.puts("  NITEM_SHOPPING_PARAM  list[FES_NITEM_SHOPPING_LIST_MAX];")
      file.puts("}NITEM_SHOPPING_DATA;")
      file.puts("\n\n")
      file.puts("typedef struct _NITEM_GET_A_PRESENT_PARAM{")
      file.puts("  u16 item;")
      file.puts("  u16 prob;      ///<抽選確率")
      file.puts("}NITEM_GET_A_PRESENT_PARAM;")
      file.puts("\n\n")
      file.puts("typedef struct _NITEM_GET_A_PRESENT_DATA{")
      file.puts("  NITEM_GET_A_PRESENT_PARAM  list[FES_NITEM_GET_A_PRESENT_LIST_MAX];")
      file.puts("}NITEM_GET_A_PRESENT_DATA;")
      file.puts("\n\n")
      file.puts("extern "+PRESENT_ITEM_DATA_NAME+";\n")
      file.puts("extern "+TRADE_ITEM_DATA_NAME+";\n")
      file.puts("extern "+SHOPPING_ITEM_DATA_NAME+";\n")
      file.puts("extern "+GPRESENT_ITEM_DATA_NAME+";\n")
      file.puts("#endif //__ASM_NO_DEF_\n\n")
      
      
      file.puts("\n\n#endif //#{code}\n\n")
    }
  end
 
end #end of CItemTbl

def convert
  #アイテムデータ取得 
  c_item = CItemTbl.new()
  c_item.make_item_list( $fpath_in_csv )

  #出力
  c_item.output_item_dat( $fpath_out_item_cdat )
  c_item.output_item_hdat( $fpath_out_item_hdat )
  c_item.output_npc_dat( $fpath_out_npc_cdat )
  c_item.output_npc_hdat( $fpath_out_npc_hdat )
end

convert()
