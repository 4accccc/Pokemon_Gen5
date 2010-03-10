#====================================================================================
#
#	ポケモン用スクリプトコンパイラ
#
#
# 2007.04 tamada GAME FREAK inc.
#
# 2009.08 tamada  wb対応開始
#
#====================================================================================

module PmScript

#---------------------------------------------
#---------------------------------------------
	class CompileError < StandardError; end

#====================================================================================
#====================================================================================
	class Enviroment

		def initialize
			@label_count = 0

			@lvar_count = 0
			@lvar_stack = Hash.new

      @switch_stack = Array.new

			@ftable = Hash.new

      @alias_count = 0
      @alias_stack = Array.new
		end

    #---------------------------------------------
    # ローカルラベル取得
    #---------------------------------------------
		def get_label()
			label = sprintf("local_label%05d", @label_count)
			@label_count += 1
			label
		end

    def get_var_alias_count
      return @alias_count
    end
    #---------------------------------------------
    # ローカル変数定義
    #---------------------------------------------
    def define_var_alias( varname )
      if @alias_stack.index( varname ) != nil then
        raise CompileError, "#{@fname}:#{@lineno}: 同じ名前（#{varname}）が複数回定義されています"
      end
      realname = sprintf("USERWK_%02d", @alias_count)
      @alias_stack << varname
      @alias_count += 1
      return realname
    end

    #---------------------------------------------
    # ローカル変数定義取り消し
    #---------------------------------------------
    def undef_all_var_alias( count )
      names = Array.new
      now_count = @alias_count
      #while count < @alias_count
      while count < now_count
        now_count -= 1
        names << @alias_stack[now_count]
        @alias_stack[now_count] = ""
        #@alias_count -= 1
        #names << @alias_stack[@alias_count]
        #@alias_stack.delete_at(@alias_count)
      end
      return names
    end

    #---------------------------------------------
    # 変数定義
    #---------------------------------------------
		def define_var( varname )
			if @lvar_stack.has_key? varname then
				raise CompileError, "#{@fname}:#{@lineno}: same var name define #{varname}"
			end
			@lvar_stack[varname] = @lvar_count
			@lvar_count += 1
		end

    #---------------------------------------------
    # 変数参照
    #---------------------------------------------
		def ref_var( varname )
			unless @lvar_stack.has_key? varname then
				return nil
			end
			@lvar_stack[varname]
		end

    #---------------------------------------------
    # 変数削除
    #---------------------------------------------
		def remove_var( value )
			idx = @lvar_stack.index(value)
			if idx == nil then
				raise CompileError, "#{@fname}:#{@lineno}: not defined var name #{value}"
			end
			@lvar_stack.delete(idx)
			@lvar_count -= 1
		end

    #---------------------------------------------
    #---------------------------------------------
		def get_lvar_stack_count
			@lvar_count
		end

    #---------------------------------------------
    #---------------------------------------------
		def define_func(funcname, argc)
			if @ftable.has_key? funcname then
				raise CompileError, "#{@fname}:#{@lineno}: func \"#{funcname}\" redfined"
			end
			@ftable[funcname] = argc
		end

    #---------------------------------------------
    #---------------------------------------------
		def get_arg_count( funcname )
			unless @ftable.has_key? funcname then
				raise CompileError, "#{@fname}:#{@lineno}: func \"#{funcname}\" undefined"
			end
			@ftable[funcname]
		end
    #---------------------------------------------
    #---------------------------------------------
    def get_switch_params()
      @switch_stack.last
    end

    def push_switch_params( params )
      @switch_stack.push params
    end

    def pop_switch_params()
      @switch_stack.pop
    end
	end


#====================================================================================
#====================================================================================
#---------------------------------------------
#---------------------------------------------
	class Node
		def	put_list( intp, nodes )
			v = nil
			nodes.each {|i| v = i.compile( intp ) }
			v
		end
	end

#---------------------------------------------
#---------------------------------------------
	class RootNode < Node
		def initialize( tree )
			@tree = tree
		end

		def compile( intp )
			put_list( intp, @tree )
		end
	end

#---------------------------------------------
# ノード：スクリプトコマンド
#---------------------------------------------
	class CmdNode < Node
		def initialize( cmd, args )
			@cmd = cmd
			@args = args
		end

		def compile( intp )
      puts "//CmdNode"
      if @cmd =~ /EVENT_START/ then
        puts "#{@args[0]}_START:"
      end
			if @args then
        puts "\t#{@cmd}\t#{@args}"
			else
				puts "\t#{@cmd}"
			end
		end
	end

#---------------------------------------------
# ノード：そのまま出力
#---------------------------------------------
	class RawNode < Node
		def initialize( val )
			@val = val
		end
		def compile( intp )
			puts "#{@val}"
		end
	end

#---------------------------------------------
# ノード：条件判定（値参照）
#---------------------------------------------
  class CompareNode < Node
    def initialize( cond, left, right )
      @cond = cond
      @left = left
      @right = right
    end

    def compile( intp )
      puts "\t#{@left}"
      puts "\t#{@right}"
      puts "\t_CMP_STACK #{@cond}"
    end
  end
#---------------------------------------------
# ノード：条件判定（論理演算）
#---------------------------------------------
  class LogicalCompareNode < Node
    def initialize( cond, left, right )
      @cond = cond
      @left = left
      @right = right
    end

    def compile( intp )
      @left.compile( intp )
      @right.compile( intp )
      puts "\t_CMP_STACK #{@cond}"
    end
  end
#---------------------------------------------
# ノード：条件分岐
#---------------------------------------------
	class IfNode < Node
		def initialize( cond_node, then_stmt, else_stmt )
			@cond_node = cond_node
			@then_stmt = then_stmt
			@else_stmt = else_stmt
		end

		def compile( intp )
			label1 = intp.get_label()
      @cond_node.compile( intp )
      puts "\t_IF_JUMP\tCMPID_GET, #{label1}"
			put_list( intp, @then_stmt )
			if @else_stmt then
				label2 = intp.get_label()
				puts "\t_JUMP\t#{label2}"
				puts "#{label1}:"
				put_list( intp, @else_stmt )
				puts "#{label2}:"
			else
				puts "#{label1}:"
			end

		end
	end

#---------------------------------------------
# ノード：while構文
#---------------------------------------------
  class WhileNode < Node
    def initialize( cond_node, stmt )
      @cond_node = cond_node
      @stmt = stmt
    end

    def compile( intp )
      start_label = intp.get_label()
      end_label = intp.get_label()
      puts "#{start_label}:"
      @cond_node.compile( intp )
      puts "\t_IF_JUMP\tCMPID_GET, #{end_label}"
      put_list( intp, @stmt )
      puts "\t_JUMP\t#{start_label}"
      puts "#{end_label}:"
    end
  end

#---------------------------------------------
# ノード：switch構文
#---------------------------------------------
  class SwitchNode < Node
    def initialize( workname, case_stmts, default_stmt)
      @workname = workname
      @case_stmts=case_stmts
      @default_stmt = default_stmt
    end

    def compile( intp )
      puts "//::SwitchStart"
      end_label = intp.get_label()
      intp.push_switch_params( [@workname, end_label] )
      put_list( intp, @case_stmts )
      if @default_stmt != nil then
        puts"//::SwitchDefault"
        put_list( intp, @default_stmt )
      end
      puts "#{end_label}:"
      intp.pop_switch_params()
      puts "//::SwitchEnd"
    end

  end

#---------------------------------------------
# ノード：CASE文
#---------------------------------------------
  class CaseNode < Node
    def initialize( args, stmts )
      @args = args
      @stmts = stmts
    end

    def compile( intp )
      hit_label = intp.get_label()
      workname, end_label = intp.get_switch_params()
      puts "//::SwitchCase(#{@args})"
      @args.each{|cond|
        if cond == ',' then next end
				if cond =~ /\A\$[a-zA-Z][a-zA-Z0-9_]*/
          puts "\t _CMPWK #{workname}, #{cond.sub(/\A\$/,"")}"
        else
          puts "\t _CMPVAL #{workname}, #{cond}"
        end

        puts "\t_IF_JUMP  EQ, #{hit_label}"
      }
      pass_label = intp.get_label()
      puts "\t_JUMP #{pass_label}"
      puts "#{hit_label}:"
      put_list( intp, @stmts )
      puts "\t_JUMP #{end_label}"
      puts "#{pass_label}:"
    end

  end

#---------------------------------------------
#---------------------------------------------
	class BlockNode < Node
		def initialize( vardefs, stmts )
			@vardefs = vardefs
			@stmts = stmts
		end

    def compile( intp )
      #現在のローカル変数定義位置を覚えておく
      count = intp.get_var_alias_count
      #ローカル変数定義リスト実行
      put_list( intp, @vardefs )
      #通常文実行
      put_list( intp, @stmts )
      #覚えておいた定義位置まですべてUndefする
      undefs = intp.undef_all_var_alias( count )
      undefs.each{|name|
        if name == "" then next end 
        puts "\t_ASM_LDVAL  #{name}, 0"
        puts "#undef #{name}"
      }
    end
	end

  #---------------------------------------------
  #---------------------------------------------
	class DefFuncNode < Node
		def initialize( type, name, param, block )
			@type = type
			@name = name
			@param = param
			@block = block
		end

		def compile( intp )
			sp = intp.get_lvar_stack_count
			if @param == nil then
				argc = 0
			else
				argc = @param.size
				@param.each {|p|
					varname1 = p.sub(/\A\$/,"")
					intp.define_var(varname1)
				}
			end
			intp.define_func(@name, argc)

			puts "#{@name}:"
			@block.compile( intp )
			while sp != intp.get_lvar_stack_count
				intp.remove_var(intp.get_lvar_stack_count - 1)
			end
		end
	end

  #---------------------------------------------
  #---------------------------------------------
	class CallFuncNode < Node
		def initialize( result, name, param )
			@result = result
			@name = name
      if param == nil
        @param = nil
      else
			  @param = param.select{|p| p != ','}
      end
		end

		def compile( intp )
			if @param == nil then
				puts "\t_CALL\t#{@name}"
				if intp.get_arg_count(@name) != 0 then
					raise CompileError, "#{@fname}:#{@lineno}: iregal args #{@name}"
				end
			else
				if intp.get_arg_count(@name) != @param.size then
					raise CompileError, "#{@fname}:#{@lineno}: iregal args #{@name}"
				end
				@param.each { |p| puts "\t_PUSH_STACK\t#{p}" }
				puts "\t_CALL\t#{@name}"
				@param.each { |p| puts "\t_POP_STACK" }
			end
			puts "\t_LDREG	#{@result}, REG0" if @result != nil
		end
	end


  #---------------------------------------------
  #---------------------------------------------
	class DefVarNode < Node

		def initialize( type, varname )
			@type = type
			@varname = varname
		end

		def compile( intp )
			intp.define_var @varname
			puts "\t_PUSH_STACK	0 //#{@varname}"
		end
	end


  #---------------------------------------------
  #---------------------------------------------
	class AssignNode < Node

		def initialize( vname, val, type )
			@vname = vname
			@val = val
			@type = type
		end

		def compile( intp )
			if @type == :VARREF then
				varname1 = @vname.sub(/\A\$/,"")
				varname2 = @val.sub(/\A\$/, "")
				idx = intp.ref_var(varname1) 
				varname1 = "STACKWORK + #{idx}" if idx != nil
				idx = intp.ref_var(varname2) 
				varname2 = "STACKWORK + #{idx}" if idx != nil
				puts "\t_ASM_LDWK\t#{varname1}, #{varname2}"
			else
				varname1 = @vname.sub(/\A\$/,"")
				idx = intp.ref_var(varname1) 
				varname1 = "STACKWORK + #{idx}" if idx != nil
				puts "\t_ASM_LDVAL\t#{varname1}, #{@val}"
			end

		end
	end

  #---------------------------------------------
  # 複合代入演算子の定義
  #---------------------------------------------
  class CompoundAssignNode < Node
    def initialize( vname, val, op )
      @vname = vname
      @val = val
      @op = op
    end

    def compile( intp )
      varname = @vname.sub(/\A\$/,"")
      value = @val.sub(/\A\$/, "")
      case @op
      when "+="
        puts "\t_ASM_ADD_WK\t#{varname}, #{value}"
      when "-="
        puts "\t_ASM_SUB_WK\t#{varname}, #{value}"
      when "*="
        puts "\t_ASM_MUL_WK\t#{varname}, #{value}"
      when "/="
        puts "\t_ASM_DIV_WK\t#{varname}, #{value}"
      when "%="
        puts "\t_ASM_MOD_WK\t#{varname}, #{value}"
      else
        raise CompileError, "#{@fname}:#{@lineno}: iregal operator #{@op}"
      end
    end

  end
  #---------------------------------------------
  # ローカル変数定義
  #---------------------------------------------
	class DefLocalVarNode < Node

		def initialize( varname )
			@varname = varname
		end

		def compile( intp )
			realname = intp.define_var_alias( @varname )
      printf("#ifdef  %-32s\n", @varname)
      printf("#error local variable %-32s\n", @varname)
      printf("#endif\n")
      printf("#define %-32s %s\n",@varname, realname )
      printf("\t_ASM_LDVAL  %s, 0\n", @varname )
			#printf ( "%-32s = %s\n",@varname, realname )
		end
	end



end   # end of module PmScript


