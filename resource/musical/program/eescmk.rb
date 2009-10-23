#========================================================================
#
#	エフェクトエディタで生成したデータからスクリプトファイルを生成
#
#========================================================================

	$KCODE = "Shift-JIS"

#	コマンドリストクラス
class	CommandList
	def initialize
		@com = []
	end

	def	add_com
		@com << Command.new
	end

	def get_com_str( str )
		for i in 0..( @com.size - 1 )
			if @com[ i ].get_com_str == str
				break
			end
		end
		@com[ i ]
	end

	def get_com_num( num )
		@com[num]
	end
end

#	コマンドクラス
class	Command
	def	initialize
		@com_str				      #コマンド名（全角文字で書かれたもの）
		@com_label				    #コマンド名（.sファイルに吐き出す#defineで定義されたもの）
		@param = []				    #パラメータ
		@combobox_text = []		#コンボボックスで表示されていたリスト
		@combobox_value = []	#コンボボックスで表示されていたリストに対応するラベル定義
		@fd_ext_num = []		  #FILE_DIALOGの拡張子指定をしたパラメータ番号
		@fd_ext = []			    #FILE_DIALOGの拡張子
		@fdc_ext_num = []		  #FILE_DIALOG_COMBOBOXの拡張子指定をしたパラメータ番号
		@fdc_ext = []			    #FILE_DIALOG_COMBOBOXの拡張子
	end

	def	set_com_str( str )
		@com_str = str
	end

	def set_com_label( label )
		@com_label = label
	end

	def	get_com_str
		@com_str
	end

	def get_com_label
		@com_label
	end

	def	add_param( param )
		@param << param
	end

	def	add_combobox_text( text )
		@combobox_text << text
	end

	def	add_combobox_value( value )
		@combobox_value << value
	end

	def add_fd_ext( num, ext )
		@fd_ext_num << num
		@fd_ext << ext
	end

	def add_fdc_ext( num, ext )
		@fdc_ext_num << num
		@fdc_ext << ext
	end

	def get_param
		@param
	end

	def get_combobox_text( num )
		if num > @combobox_text.size
			print "combobox_text配列サイズより大きい値が設定されています\n"
			exit( 1 )
		end
		if @combobox_text.size != @combobox_value.size
			print @com_str + "のCOMBOBOX_TEXTとCOMBOBOX_VALUEの数が一致しません\n"
			print "区切り文字に全角スペースが含まれている可能性があります\n"
			exit( 1 )
		end
		@combobox_text[ num ]
	end

	def get_combobox_value( num )
		if num > @combobox_value.size
			print "combobox_value配列サイズより大きい値が設定されています\n"
			exit( 1 )
		end
		if @combobox_text.size != @combobox_value.size
			print @com_str + "のCOMBOBOX_TEXTとCOMBOBOX_VALUEの数が一致しません\n"
			print "区切り文字に全角スペースが含まれている可能性があります\n"
			exit( 1 )
		end
		@combobox_value[ num ]
	end

	def get_fd_ext( num )
		find = -1
		@fd_ext_num.each { |no|
			if @fd_ext_num[ no ] == num
				find = no
				break
			end
		}
		if find == -1
			print "設定されていない拡張子が指定されています\n"
		end
		@fd_ext[ find ]
	end

	def get_fdc_ext( num )
		find = -1
		@fdc_ext_num.each { |no|
			if @fdc_ext_num[ no ] == num
				find = no
				break
			end
		}
		if find == -1
			print "設定されていない拡張子が指定されています\n"
		end
		@fdc_ext[ find ]
	end

	def search_param( param )
		for num in 0..@param.size
			if @param[ num ] == param
				break
			end
		end
		num
	end

	def search_combobox_text( text )
		for num in 0..@combobox_text.size
			if @combobox_text[ num ] == text
				break
			end
		end
		num
	end

	def search_combobox_value( value )
		for num in 0..@combobox_value.size
			if @combobox_value[ num ] == value
				break
			end
		end
		num
	end

end

#	メインルーチン
	if ARGV.size < 3
		print "error: ruby eescmk.rb def_file esf_file inc_dir sw\n";
		print "def_file: スクリプト命令マクロが記述されたヘッダーファイル\n";
		print "esf_file: エフェクトエディタで作成されたesfファイル\n";
		print "inc_dir: インクルードするヘッダーファイルのディレクトリ\n";
		print "sw: MCS用のバイナリデータを生成する（onで生成）\n";
		exit( 1 )
	end

	ARGV_DEF_FILE = 0
	ARGV_ESF_FILE = 1
	ARGV_INC_DIR = 2
	ARGV_SW = 3

	com_list = CommandList.new
	com_num = 0
	param_num = 0
	param_num_max = 0
	combobox_text = 0

	fp_r = open( ARGV[ ARGV_DEF_FILE ] )
	data = fp_r.readlines
	fp_r.close

	seq_no = 0

	#コマンドクラスを生成
	SEQ_INIT_CMD_SEARCH = 0
	SEQ_BRIEF_SEARCH = 1
	SEQ_PARAM_NUM_SEARCH = 2
	SEQ_PARAM_SEARCH = 3
	SEQ_MACRO_SEARCH = 4

	#SEQ_INIT_CMD_SEARCH
	INIT_CMD_POS = 0

	#SEQ_BRIEF_SEARCH
	BRIEF_POS = 2
	COM_STR_POS = 3

	#SEQ_PARAM_NUM_SEARCH
	PARAM_NUM_POS = 2
	PARAM_NUM_VALUE = 3

	#SEQ_PARAM_SEARCH
	PARAM_POS = 2
	PARAM_VALUE = 3

	#SEQ_MACRO_SEARCH
	MACRO_POS = 1
	COM_LABEL_POS = 2

	data.size.times { |i|
		split_data = data[i].split(/\s+/)
		case seq_no
		when SEQ_INIT_CMD_SEARCH
			if split_data[ INIT_CMD_POS ] == "//COMMAND_START"
				seq_no = SEQ_BRIEF_SEARCH
			end
		when SEQ_BRIEF_SEARCH
			if split_data[ BRIEF_POS ] == "@brief"
				com_list.add_com
				com_list.get_com_num( com_num ).set_com_str( split_data[ COM_STR_POS ] );
				seq_no = SEQ_PARAM_NUM_SEARCH
			end
		when SEQ_PARAM_NUM_SEARCH
			if split_data[ PARAM_NUM_POS ] == "#param_num"
				param_num_max = split_data[ PARAM_NUM_VALUE ].to_i;
				param_num = 0
				seq_no = SEQ_PARAM_SEARCH
			end
		when SEQ_PARAM_SEARCH
			if split_data[ PARAM_POS ] == "#param"
				case split_data[ PARAM_VALUE ]
				when "COMBOBOX_TEXT"
					combobox_text = 1
					com_list.get_com_num( com_num ).add_param( split_data[ PARAM_VALUE ] )
					for i in (PARAM_VALUE + 1)..(split_data.size - 1)
						com_list.get_com_num( com_num ).add_combobox_text( split_data[ i ] )
					end
				when "COMBOBOX_VALUE"
					if combobox_text == 0
						print "syntax error command:" + com_list.get_com_num( com_num ).get_com_str
						print "\nCOBOBOX_VALUEを使用するときは、COMBOBOX_TEXTと併記してください\n"
						exit( 1 )
					end
					for i in (PARAM_VALUE + 1)..(split_data.size - 1)
						com_list.get_com_num( com_num ).add_combobox_value( split_data[ i ] )
					end
					combobox_text = 0
					param_num = param_num + 1
				when "FILE_DIALOG"
					combobox_text = 0
					com_list.get_com_num( com_num ).add_param( split_data[ PARAM_VALUE ] )
					com_list.get_com_num( com_num ).add_fd_ext( param_num, split_data[ PARAM_VALUE + 1 ] )
					param_num = param_num + 1
				when "FILE_DIALOG_WITH_ADD"
					combobox_text = 0
					com_list.get_com_num( com_num ).add_param( split_data[ PARAM_VALUE ] )
					com_list.get_com_num( com_num ).add_fd_ext( param_num, split_data[ PARAM_VALUE + 1 ] )
					param_num = param_num + 1
				when "FILE_DIALOG_COMBOBOX"
					combobox_text = 0
					com_list.get_com_num( com_num ).add_param( split_data[ PARAM_VALUE ] )
					com_list.get_com_num( com_num ).add_fdc_ext( param_num, split_data[ PARAM_VALUE + 1 ] )
					param_num = param_num + 1
        when "VALUE_INIT"
				else
					combobox_text = 0
					com_list.get_com_num( com_num ).add_param( split_data[ PARAM_VALUE ] )
					param_num = param_num + 1
				end
			end
			if param_num == param_num_max
				seq_no = SEQ_MACRO_SEARCH
			end
		when SEQ_MACRO_SEARCH
			if split_data[ MACRO_POS ] == ".macro"
				com_list.get_com_num( com_num ).set_com_label( split_data[ COM_LABEL_POS ] )
				com_num = com_num + 1
				seq_no = SEQ_BRIEF_SEARCH
			end
		end
	}

	#esfファイルから.sファイルを生成
	fp_r = open( ARGV[ ARGV_ESF_FILE ] )
	data = fp_r.readlines
	fp_r.close

	seq_no = 0
	seq_table = []
	sequence = []
	inc_header = []
	dir_table = [ "AA2BB", "BB2AA", "A2B", "A2C", "A2D", "B2A", "B2C", "B2D", "C2A", "C2B", "C2D", "D2A", "D2B", "D2C" ]
	ARGV[ ARGV_ESF_FILE ] =~ /\D+(\d+)\.\D+/
	num_str = $1
  file_name = File::basename( ARGV[ ARGV_ESF_FILE ] )
  write_file = file_name.sub("esf","s")
	file_list = []
	bin_list = []
	bin_list_tmp = []

	data_pos = 0

	#シーケンス
	SEQ_SIGNATURE_SEARCH = 0
	SEQ_FILE_LIST_SEARCH = 1
	SEQ_EFFNO_SEARCH = 2
	SEQ_MAKE_DATA = 3

	EFFNO_POS = 0
	ESF_COM_STR_POS = 0

	while data_pos < data.size
		split_data = data[ data_pos ].split(/\s+/)
		case seq_no
		when SEQ_SIGNATURE_SEARCH
			cnt = 0
			"EFFSEQ".each_byte { |c|
				if split_data[ 0 ][ cnt ] != c
					print "このスクリプトで解釈できないファイルです\n"
					exit( 1 )
				end
				cnt += 1
			}
			seq_no = SEQ_FILE_LIST_SEARCH
		when SEQ_FILE_LIST_SEARCH
			if split_data[ 0 ][ 0 ].chr == "$"
				file_cnt = split_data[ 0 ][ 1 ].chr + split_data[ 0 ][ 2 ].chr + split_data[ 0 ][ 3 ].chr + split_data[ 0 ][ 4 ].chr
				file_cnt.to_i.times{
					data_pos += 1
					file_list << data[ data_pos ].strip
				}
			else
				data_pos -= 1
			end
			seq_no = SEQ_EFFNO_SEARCH
		when SEQ_EFFNO_SEARCH
			if split_data[ EFFNO_POS ][ 0 ].chr == "#"
        if num_str == nil
				  num_str = split_data[ EFFNO_POS ][ 1 ].chr + split_data[ EFFNO_POS ][ 2 ].chr + split_data[ EFFNO_POS ][ 3 ].chr
          write_file = "we_" + num_str + ".s"
        end
				seq_table.clear
				sequence.clear
				inc_header.clear
				dir_table.size.times { |dir|
					seq_table << "\t.long\t" + "WE_" + num_str + "_00 - WE_" + num_str +"\t//" + dir_table[ dir ] + "\n"
				}
				seq_no = SEQ_MAKE_DATA
			end
		when SEQ_MAKE_DATA
			if split_data[ EFFNO_POS ][ 0 ].chr == "#"
				data_pos -= 1
				seq_no = SEQ_EFFNO_SEARCH
			elsif split_data[ EFFNO_POS ][ 0 ].chr == "&"
				fp_w = open( write_file, "w" )
				fp_w.print("//===================================================\n")
				fp_w.print("//\n")
				fp_w.print("//	エフェクトシーケンス\n")
				fp_w.print("//\n")
				fp_w.print("//	コンバータから吐き出されたファイルです\n")
				fp_w.print("//\n")
				fp_w.print("//===================================================\n")
				fp_w.print("\n")
				fp_w.print("\t.text\n")
				fp_w.print("\n")
				fp_w.print("#define	__ASM_NO_DEF_\n")
#				fp_w.print("\t.include	" + ARGV[ ARGV_INC_DIR ] + "prog/src/battle/btlv/btlv_efftool.h\n")
#				fp_w.print("\t.include	" + ARGV[ ARGV_INC_DIR ] + "prog/src/battle/btlv/btlv_effvm_def.h\n")
#				fp_w.print("\t.include	" + ARGV[ ARGV_INC_DIR ] + "prog/include/system/mcss.h\n")
#				fp_w.print("\t.include	" + ARGV[ ARGV_INC_DIR ] + "prog/include/sound/wb_sound_data.sadl\n")
#				fp_w.print("\t.include	" + ARGV[ ARGV_INC_DIR ] + "prog/arc/particle/wazaeffect/spa_def.h\n")
				fp_w.print("\t.include	" + ARGV[ ARGV_INC_DIR ] + "prog/src/musical/stage/script/musical_script_command.h\n")
				inc_header.size.times { |inc|
					fp_w.print("\t.include	" + ARGV[ ARGV_INC_DIR ] + "resource/particle/wazaeffect/" + inc_header[ inc ] + "\n")
				}
				fp_w.print("\n")
				fp_w.print( "WE_" + num_str + ":\n" )
				seq_table.size.times { |seq|
					fp_w.print seq_table[ seq ]
				}
				sequence.size.times { |seq|
					fp_w.print sequence[ seq ]
				}
				fp_w.close
			elsif split_data[ EFFNO_POS ][ 0 ].chr == "%"
				dir_str = split_data[ EFFNO_POS ][ 1 ].chr + split_data[ EFFNO_POS ][ 2 ].chr
				seq_str = "\nWE_" + num_str + "_" + dir_str + ":\n"
				sequence << seq_str
				seq_table[ dir_str.to_i ] = "\t.long\t" + "WE_" + num_str + "_" + dir_str + " - WE_" + num_str + "\t//" + dir_table[ dir_str.to_i ] + "\n"
			else
				str = ""
				str += "\t" + com_list.get_com_str( split_data[ ESF_COM_STR_POS ] ).get_com_label + "\t"
				param_num = 1
				com_list.get_com_str( split_data[ ESF_COM_STR_POS ] ).get_param.each {|param|
					if param_num != 1
						str += ",\t"
					end
					case param
					when "COMBOBOX_TEXT"
						num = com_list.get_com_str( split_data[ ESF_COM_STR_POS ] ).search_combobox_text( split_data[ param_num ] )
						str += com_list.get_com_str( split_data[ ESF_COM_STR_POS ] ).get_combobox_value( num )
					when "VALUE_FX32"
						f = split_data[ param_num ].to_f
						if f > 0
							f = f * ( 1 << 12 ) + 0.5
						else
							f = f * ( 1 << 12 ) - 0.5
						end
						str += format("0x%08x",f)
					when "VALUE_INT"
						str += split_data[ param_num ]
					when "VALUE_VECFX32"
						vecfx32 = split_data[ param_num ].split(/:/)
						for i in 0..(vecfx32.size-1)
							if i != 0
								str += ",\t"
							end
							f = vecfx32[ i ].to_f
							if f > 0
								f = f * ( 1 << 12 ) + 0.5
							else
								f = f * ( 1 << 12 ) - 0.5
							end
							str += format("0x%08x",f)
						end
					when "VALUE_VECINT"
						vecint = split_data[ param_num ].split(/:/)
						for i in 0..(vecint.size-1)
							if i != 0
								str += ",\t"
							end
							str += vecint[ i ]
						end
					when "VALUE_ANGLE"
            angle = split_data[ param_num ].to_f;
            angle = 65535 * angle / 360
						if angle > 0
							angle = angle * ( 1 << 12 ) + 0.5
						else
							angle = angle * ( 1 << 12 ) - 0.5
						end
						str += format("0x%08x",angle)
					when "FILE_DIALOG"
						file_dialog = split_data[ param_num ] + com_list.get_com_str( split_data[ ESF_COM_STR_POS ] ).get_fd_ext( param_num -1 ) 
            if( File::extname( file_dialog ) == ".spa" )
						  inc_header << file_dialog.sub( com_list.get_com_str( split_data[ ESF_COM_STR_POS ] ).get_fd_ext( param_num -1 ), ".h" )
            end
						file_list.size.times {|num|
							file_name = File::basename( file_list[ num ] )
							if file_name[ 0..file_dialog.length ] == file_dialog
								bin_list_tmp << file_list[ num ]
							end
						}

						file_dialog = file_dialog.sub( ".", "_" ).upcase
						str += file_dialog
					when "FILE_DIALOG_WITH_ADD"
						file_dialog = split_data[ param_num ] + com_list.get_com_str( split_data[ ESF_COM_STR_POS ] ).get_fd_ext( param_num -1 ) 
            if( File::extname( file_dialog ) == ".spa" )
						  inc_header << file_dialog.sub( com_list.get_com_str( split_data[ ESF_COM_STR_POS ] ).get_fd_ext( param_num -1 ), ".h" )
            end
						file_list.size.times {|num|
							file_name = File::basename( file_list[ num ] )
							if file_name[ 0..file_dialog.length ] == file_dialog
								bin_list_tmp << file_list[ num ]
							end
						}

						file_dialog = file_dialog.sub( ".", "_" ).upcase
						str += file_dialog
					when "FILE_DIALOG_COMBOBOX"
						file_dialog = split_data[ param_num ] + com_list.get_com_str( split_data[ ESF_COM_STR_POS ] ).get_fdc_ext( param_num -1 ) 
            if( File::extname( file_dialog ) == ".spa" )
						  inc_header << file_dialog.sub( com_list.get_com_str( split_data[ ESF_COM_STR_POS ] ).get_fdc_ext( param_num -1 ), ".h" )
            end
						file_list.size.times {|num|
							file_name = File::basename( file_list[ num ] )
							if file_name[ 0..file_dialog.length ] == file_dialog
								bin_list_tmp << file_list[ num ]
							end
						}
						file_dialog = file_dialog.sub( ".", "_" ).upcase
						str += file_dialog
					when "COMBOBOX_HEADER"
						str += split_data[ param_num ]
					end
					param_num = param_num + 1
				}
				str += "\n"
				sequence << str
			end
		end
		data_pos += 1
	end

	#bin_list_tmpのダブりをチェック
	bin_list_tmp.size.times {|tmp_num|
		count = 0
		bin_list.size.times {|list_num|
			if bin_list[ list_num ] == bin_list_tmp[ tmp_num ]
				break;
			end
			count += 1
		}
		if bin_list.size == count
			bin_list << bin_list_tmp[ tmp_num ]
      if( File::extname( bin_list_tmp[ tmp_num ] ) == ".NSCR" )
			  bin_list << File::dirname( bin_list_tmp[ tmp_num ] ) + "\\" + File::basename( bin_list_tmp[ tmp_num ], ".NSCR" ) + ".NCGR"
			  bin_list << File::dirname( bin_list_tmp[ tmp_num ] ) + "\\" + File::basename( bin_list_tmp[ tmp_num ], ".NSCR" ) + ".NCLR"
      end
		end
	}

	open( "eebinary.bin", "wb" ) {|file|
		padding = []
		offset = bin_list.size * 4
		#ファイル数書き出し
		file.write [bin_list.size].pack("L")
		#ファイルサイズ書き出し
		bin_list.size.times {|num|
			fp_r = File::stat( bin_list[ num ] )
			padding << 4 - ( fp_r.size % 4 )
			size = fp_r.size + padding[ num ]
			file.write [offset].pack("L")
			offset += size
		}
		#ファイルデータ書き出し
		bin_list.size.times {|num|
			fp_r = open( bin_list[ num ], "rb" )
			file.write fp_r.read
			padding[ num ].times do
				file.putc( 0 )
			end
		}
	}

