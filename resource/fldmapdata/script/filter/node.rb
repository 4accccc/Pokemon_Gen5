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

			@ftable = Hash.new
		end

		def get_label
			label = sprintf("LABEL%05d", @label_count)
			@label_count += 1
			label
		end

		def define_var( varname )
			if @lvar_stack.has_key? varname then
				raise CompileError, "#{@fname}:#{@lineno}: same var name define #{varname}"
			end
			@lvar_stack[varname] = @lvar_count
			@lvar_count += 1
		end

		def ref_var( varname )
			unless @lvar_stack.has_key? varname then
				return nil
			end
			@lvar_stack[varname]
		end

		def remove_var( value )
			idx = @lvar_stack.index(value)
			if idx == nil then
				raise CompileError, "#{@fname}:#{@lineno}: not defined var name #{value}"
			end
			@lvar_stack.delete(idx)
			@lvar_count -= 1
		end

		def get_lvar_stack_count
			@lvar_count
		end

		def define_func(funcname, argc)
			if @ftable.has_key? funcname then
				raise CompileError, "#{@fname}:#{@lineno}: func \"#{funcname}\" redfined"
			end
			@ftable[funcname] = argc
		end

		def get_arg_count( funcname )
			unless @ftable.has_key? funcname then
				raise CompileError, "#{@fname}:#{@lineno}: func \"#{funcname}\" undefined"
			end
			@ftable[funcname]
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
# ノード：条件分岐
#---------------------------------------------
	class IfNode < Node
		def initialize( cond_node, then_stmt, else_stmt )
			@cond_node = cond_node
			@then_stmt = then_stmt
			@else_stmt = else_stmt
		end

		def compile( intp )
			#put compare command
			puts "\t#{@cond_node[0]}"

			#put if_stmt condition EOL
			label1 = intp.get_label
			puts "\t_IF_JUMP\t#{@cond_node[1]},#{label1}"
			put_list( intp, @then_stmt )
			if @else_stmt then
				label2 = intp.get_label
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
#---------------------------------------------
	class BlockNode < Node
		def initialize( vardefs, stmts )
			@vardefs = vardefs
			@stmts = stmts
		end

		def compile( intp )
			sp = intp.get_lvar_stack_count
			put_list( intp, @vardefs )
			put_list( intp, @stmts )
			while sp != intp.get_lvar_stack_count
				intp.remove_var(intp.get_lvar_stack_count - 1)
				puts "\t_POP_STACK"
			end
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
				puts "\t_LDWK\t#{varname1}, #{varname2}"
			else
				varname1 = @vname.sub(/\A\$/,"")
				idx = intp.ref_var(varname1) 
				varname1 = "STACKWORK + #{idx}" if idx != nil
				puts "\t_LDVAL\t#{varname1}, #{@val}"
			end

		end
	end


end


