--===========================================================================
--
-- slang.g			Shader Language for Embedded OpenGL Implementation
--
--					This grammar is based on the OpenGL Shader Language
--					Specification Version 1.051
--
--					We added a couple of additions to this grammar to make it 
--					usable for runtime generation of shaders for the 
--					OpenGL|ES implementation.
--
--					Additions to the basic grammar are:
--						o	new basic types ubyte, ushort and fixed
--						o	corresponding vector types uvec?, svec?, xvec?
--						o	simplified switch statements
--
-----------------------------------------------------------------------------
--
-- 10-27-2003		Hans-Martin Will	initial version
--
--===========================================================================


-----------------------------------------------------------------------------
-- Primitive Types implemented in C
-----------------------------------------------------------------------------


'type' FLOAT							-- floating point constant
'type' FIXED							-- fixed point constant
'type' IDENT							-- identifier (unique string)


-----------------------------------------------------------------------------
-- Primitive Functions implemented in C
-----------------------------------------------------------------------------


'action' Error(STRING, POS)


-----------------------------------------------------------------------------
-- Types
-----------------------------------------------------------------------------


'type' SIMPLETYPE
	bool
	ubyte
	ushort
	int
	fixed
	float

'type' TYPE
	void
	simple(SIMPLETYPE)
	vec(SIMPLETYPE, INT)
	mat(SIMPLETYPE, INT)
	array(TYPE, INT)
	struct
	sampler2d
	

-----------------------------------------------------------------------------
-- Expressions
-----------------------------------------------------------------------------


'type' LITERAL
	nil
	bool(INT)
	integer(INT)
	fixed(FIXED)
	float(FLOAT)

	
'type' UNARY_OP
	not				--	!
	cmp				--	~
	minus			--	-
	plus			--	+
	pre_inc			--	++x
	post_inc		--	x++
	pre_dec			--	--x
	post_dec		--	x--

	
'type' BINARY_OP
	add				--	+
	sub				--  -
	mul				--	*
	div				--  /
	mod				--	%
	shl				--	<<
	shr				--	>>
	and				--	&
	or				--	|
	xor				--	^

		
'type' COMPARISON_OP
	equ				--	==
	neq				--	!=
	gtr				--  >
	lss				--	<
	geq				--	>=
	leq				--	<=

	
'type' ASSIGNMENT_OP
	asn				--	=
	asn_add			--	+=
	asn_sub			--	-=
	asn_mul			--	*=
	asn_div			--	/=
	asn_mod			--	%=
	asn_shl			--	<<=
	asn_shr			--	>>=
	asn_and			--	&=
	asn_or			--	|=
	asn_xor			--	^=


'type' LOGICAL_OP
	land
	lor
	lxor
	
	
'type' EXPRESSION_LIST
	nil
	cons(EXPRESSION, EXPRESSION_LIST)


'type' REF
	nil
	global(INT)							-- global variable
	param(INT)							-- parameter 
	local(INT)							-- local variable

	
'type' EXPRESSION
	nil
	literal(TYPE, LITERAL)
	access(TYPE, REF)					--	memory access: type, base+offset
	index(TYPE, REF, EXPRESSION)		--	memory access: type, base+offset, index
	unary(TYPE, UNARY_OP, EXPRESSION)
	binary(TYPE, BINARY_OP, EXPRESSION, EXPRESSION)
	assignment(ASSIGNMENT_OP, EXPRESSION, EXPRESSION)
	logical(LOGICAL_OP, EXPRESSION, EXPRESSION)
	comparison(COMPARISON_OP, EXPRESSION, EXPRESSION)
	comma(EXPRESSION, EXPRESSION)
	cond(TYPE, EXPRESSION, EXPRESSION, EXPRESSION)	
	

--
--	L-Value Rules: When isa n expression valid on the left hand-side of
--	an assignment operator or subject to an increment/decrement operation
--
'condition' L_Value(EXPRESSION)
	'rule' L_Value(access(Type, Ref)):
	'rule' L_Value(index(Type, Ref, Expr)):
	'rule' L_Value(assignment(Op, Lhs, Rhs)):
	'rule' L_Value(unary(T, Op, Expr)): (| eq(Op, pre_inc) || eq (Op, pre_dec) |)

'action' ExtractType(EXPRESSION -> TYPE)
	'rule' ExtractType(literal(Type, _) -> Type):
	'rule' ExtractType(access(Type, _) -> Type):
	'rule' ExtractType(index(Type, _, _) -> Type):
	'rule' ExtractType(unary(Type, _, _) -> Type):
	'rule' ExtractType(binary(Type, _, _, _) -> Type):
	
	'rule' ExtractType(assignment(Op, Left, _) -> Type):
		ExtractType(Left -> Type)
		
	'rule' ExtractType(logical(_, _, _) -> simple(bool)):
	'rule' ExtractType(comparison(_, _, _) -> simple(bool)):
	
	'rule' ExtractType(comma(_, Right) -> Type):
		ExtractType(Right -> Type)
		
	'rule' ExtractType(cond(Type, _, _, _) -> Type):
	'rule' ExtractType(Expr -> TYPE'void):
	
'action' Coerce(EXPRESSION, EXPRESSION -> TYPE) 
	'rule' Coerce(Left, Right -> Type):
		ExtractType(Left -> Type)
		ExtractType(Right -> Type0)
		eq(Type, Type0)
		
-----------------------------------------------------------------------------
-- Statements
-----------------------------------------------------------------------------


'type' STATEMENT_LIST
	nil
	cons(STATEMENT, STATEMENT_LIST)

	
'action' AppendStatement(STATEMENT_LIST, STATEMENT -> STATEMENT_LIST)
	'rule' AppendStatement(nil, Statement -> cons(Statement, nil)):
	'rule' AppendStatement(cons(Head, Tail), Statement -> cons(Head, Tail0)):
		AppendStatement(Tail, Statement -> Tail0)
	
	
'type' STATEMENT
	nil
	expr(EXPRESSION)
	if(EXPRESSION, STATEMENT, STATEMENT)
	switch(EXPRESSION, CASE_LIST)
	while(EXPRESSION, STATEMENT)
	do(EXPRESSION, STATEMENT)
	list(STATEMENT_LIST)
	break
	continue
	return(EXPRESSION)
	discard


'type' CASE_LIST
	nil
	cons(CASE, CASE_LIST)
	

'type' CASE
	case(LITERAL_LIST, STATEMENT_LIST)
	
'action' AppendCase(CASE_LIST, CASE -> CASE_LIST)
	'rule' AppendCase(nil, Case -> cons(Case, nil)):
	'rule' AppendCase(cons(Head, Tail), Case -> cons(Head, Tail0)):
		AppendCase(Tail, Case -> Tail0)
		
'type' LITERAL_LIST
	nil
	default
	cons(LITERAL, LITERAL_LIST)
	
'action' AppendLiteral(LITERAL_LIST, LITERAL -> LITERAL_LIST)
	'rule' AppendLiteral(nil, Lit -> cons(Lit, nil)):
	'rule' AppendLiteral(default, Lit -> cons(Lit, default)):
	'rule' AppendLiteral(cons(Head, Tail), Lit -> cons(Head, Tail0)):
		AppendLiteral(Tail, Lit -> Tail0)
		
'action' AppendDefault(LITERAL_LIST -> LITERAL_LIST)
	'rule' AppendDefault(nil -> default):
	'rule' AppendDefault(default -> default):
	'rule' AppendDefault(cons(Head, Tail) -> cons(Head, Tail0)):
		AppendDefault(Tail -> Tail0)
 	
-----------------------------------------------------------------------------
-- Tokens
-----------------------------------------------------------------------------


'token' IDENTIFIER (-> IDENT)
'token' TYPE_NAME (-> IDENT)
'token' FLOATCONSTANT(-> FLOAT)
'token' INTCONSTANT(-> INT)

'action' AllowTypeName()			-- a call into this functions will
									-- have the lexer check for type_names
									-- for the next token to be returned
									
-----------------------------------------------------------------------------
-- Grammar
-----------------------------------------------------------------------------


'root' translation_unit

'nonterm' variable_identifier
	'rule' variable_identifier: IDENTIFIER (-> Id)

'nonterm' primary_expression(-> EXPRESSION)
	'rule' primary_expression(-> access(void, nil)): 
		variable_identifier
		
	'rule' primary_expression(-> literal(simple(int), integer(Val))): 
		INTCONSTANT(-> Val)
		
	'rule' primary_expression(-> literal(simple(int), float(Val))): 
		FLOATCONSTANT(-> Val)
		
	'rule' primary_expression(-> literal(simple(bool), Lit)): 
		bool_const(-> Lit)
		
	'rule' primary_expression(-> Expr): 
		"(" expression(-> Expr) ")"

'nonterm' bool_const (-> LITERAL)
	'rule' bool_const(-> bool(1)): "true"
	'rule' bool_const(-> bool(0)): "false"
	
'nonterm' postfix_expression (-> EXPRESSION)
	'rule' postfix_expression(-> Expr): 
		primary_expression(-> Expr)
		
	'rule' postfix_expression(-> Result): 
		postfix_expression(-> Expr) "[" @ (-> Pos) integer_expression(-> Index) "]"
		(|
			where(Expr -> access(array(Type, Size), Ref))
			where(index(Type, Ref, Expr) -> Result)
		||
			Error("Invalid application of indexing operator[]", Pos)
			where (EXPRESSION'nil -> Result)
		|)
		
	'rule' postfix_expression(-> Expr): 
		function_call (-> Expr)
		
	'rule' postfix_expression (-> Expr): 
		postfix_expression(-> Expr) "." @ (-> Pos) IDENTIFIER (-> Id) -- was: FIELD_SELECTION
		
	'rule' postfix_expression (-> Result): 
		postfix_expression(-> Expr) @ (-> Pos) "++"
		(| 
			L_Value(Expr)
			ExtractType(Expr -> Type)
			where (unary(Type, post_inc, Expr) -> Result)
		||
			Error("Lvalue required", Pos)
			where (EXPRESSION'nil -> Result)
		|)
		
	'rule' postfix_expression (-> Result): 
		postfix_expression(-> Expr) @ (-> Pos)  "--"
		(| 
			L_Value(Expr)
			ExtractType(Expr -> Type)
			where (unary(Type, post_dec, Expr) -> Result)
		||
			Error("Lvalue required", Pos)
			where (EXPRESSION'nil -> Result)
		|)
	
'nonterm' integer_expression (-> EXPRESSION)
	'rule' integer_expression (-> Expr): 
		expression(-> Expr)
		ExtractType(Expr -> Type)
		(|
			where (Type -> simple(ubyte))
		||
			where (Type -> simple(ushort))
		||
			where (Type -> simple(int))
		|)

'nonterm' function_call (-> EXPRESSION)
	'rule' function_call (-> EXPRESSION'nil): 
		function_call_generic
	
'nonterm' function_call_generic 
	'rule' function_call_generic: function_call_header_with_parameters ")"
	'rule' function_call_generic: function_call_header_no_parameters ")"

'nonterm' function_call_header_no_parameters
	'rule' function_call_header_no_parameters: function_call_header "void"
	'rule' function_call_header_no_parameters: function_call_header

'nonterm' function_call_header_with_parameters
	'rule' function_call_header_with_parameters: function_call_header assignment_expression (-> Expr)
	'rule' function_call_header_with_parameters: function_call_header_with_parameters "," assignment_expression (-> Expr)

'nonterm' function_call_header
	'rule' function_call_header: function_identifier "("

'nonterm' function_identifier
	'rule' function_identifier: constructor_identifier (-> Type)
	'rule' function_identifier: IDENTIFIER (-> Id)

'nonterm' constructor_identifier (-> TYPE)
	'rule' constructor_identifier (-> simple(float))	: "float"		
	'rule' constructor_identifier (-> simple(fixed))	: "fixed"
	'rule' constructor_identifier (-> simple(ubyte))	: "ubyte"
	'rule' constructor_identifier (-> simple(ushort))	: "ushort"
	'rule' constructor_identifier (-> simple(int))		: "int"
	'rule' constructor_identifier (-> simple(bool))		: "bool"
	'rule' constructor_identifier (-> vec(float, 2))	: "vec2"
	'rule' constructor_identifier (-> vec(float, 3))	: "vec3"
	'rule' constructor_identifier (-> vec(float, 4))	: "vec4"
	'rule' constructor_identifier (-> vec(ubyte, 2))	: "uvec2"
	'rule' constructor_identifier (-> vec(ubyte, 3))	: "uvec3"
	'rule' constructor_identifier (-> vec(ubyte, 4))	: "uvec4"
	'rule' constructor_identifier (-> vec(ushort, 2))	: "svec2"
	'rule' constructor_identifier (-> vec(ushort, 3))	: "svec3"
	'rule' constructor_identifier (-> vec(ushort, 4))	: "svec4"
	'rule' constructor_identifier (-> vec(fixed, 2))	: "xvec2"
	'rule' constructor_identifier (-> vec(fixed, 3))	: "xvec3"
	'rule' constructor_identifier (-> vec(fixed, 4))	: "xvec4"
	'rule' constructor_identifier (-> vec(bool, 2))		: "bvec2"
	'rule' constructor_identifier (-> vec(bool, 3))		: "bvec3"
	'rule' constructor_identifier (-> vec(bool, 4))		: "bvec4"
	'rule' constructor_identifier (-> vec(int, 2))		: "ivec2"
	'rule' constructor_identifier (-> vec(int, 3))		: "ivec3"
	'rule' constructor_identifier (-> vec(int, 4))		: "ivec4"
	'rule' constructor_identifier (-> mat(float, 2))	: "mat2"
	'rule' constructor_identifier (-> mat(float, 3))	: "mat3"
	'rule' constructor_identifier (-> mat(float, 4))	: "mat4"
	'rule' constructor_identifier (-> void)				: allow_type_name TYPE_NAME (-> String)

'nonterm' allow_type_name
	'rule' allow_type_name: /* empty */ AllowTypeName() 
		
'nonterm' unary_expression (-> EXPRESSION)
	'rule' unary_expression (-> Expr): 
		postfix_expression (-> Expr)
		
	'rule' unary_expression (-> Result): 
		"++" @ (-> Pos) unary_expression (-> Expr)
		(| 
			L_Value(Expr)
			ExtractType(Expr -> Type)
			where (unary(Type, pre_inc, Expr) -> Result)
		||
			Error("Lvalue required", Pos)
			where (EXPRESSION'nil -> Result)
		|)
		
	'rule' unary_expression (-> Result): 
		"--" @ (-> Pos) unary_expression (-> Expr)
		(| 
			L_Value(Expr)
			ExtractType(Expr -> Type)
			where (unary(Type, pre_dec, Expr) -> Result)
		||
			Error("Lvalue required", Pos)
			where (EXPRESSION'nil -> Result)
		|)
	
	'rule' unary_expression (-> unary(Type, Op, Expr)): 
		unary_operator (-> Op) unary_expression (-> Expr)
		ExtractType(Expr -> Type)

'nonterm' unary_operator (-> UNARY_OP)
	'rule' unary_operator(-> plus)	: "+"
	'rule' unary_operator(-> minus)	: "-"
	'rule' unary_operator(-> not)	: "!"
	'rule' unary_operator(-> cmp)	: "~" 

'nonterm' multiplicative_expression (-> EXPRESSION)
	'rule' multiplicative_expression (-> Expr): 
		unary_expression (-> Expr)
		
	'rule' multiplicative_expression (-> binary(Type, mul, Left, Right)): 
		multiplicative_expression (-> Left) "*" unary_expression (-> Right)
		Coerce(Left, Right -> Type)
		
	'rule' multiplicative_expression (-> binary(Type, div, Left, Right)): 
		multiplicative_expression (-> Left) "/" unary_expression (-> Right)
		Coerce(Left, Right -> Type)
		
	'rule' multiplicative_expression (-> binary(Type, mod, Left, Right)): 
		multiplicative_expression (-> Left) "%" unary_expression (-> Right)
		Coerce(Left, Right -> Type)

'nonterm' additive_expression (-> EXPRESSION)
	'rule' additive_expression (-> Expr): 
		multiplicative_expression (-> Expr)
		
	'rule' additive_expression (-> binary(Type, add, Left, Right)): 
		additive_expression (-> Left) "+" multiplicative_expression (-> Right)
		Coerce(Left, Right -> Type)
		
	'rule' additive_expression (-> binary(Type, sub, Left, Right)): 
		additive_expression (-> Left) "-" multiplicative_expression (-> Right)
		Coerce(Left, Right -> Type)

'nonterm' shift_expression (-> EXPRESSION)
	'rule' shift_expression (-> Expr): 
		additive_expression (-> Expr)
		
	'rule' shift_expression (-> binary(Type, shl, Left, Right)): 
		shift_expression (-> Left) "<<" additive_expression (-> Right) 
		ExtractType(Left -> Type)
		
	'rule' shift_expression (-> binary(Type, shr, Left, Right)): 
		shift_expression (-> Left) ">>" additive_expression (-> Right)
		ExtractType(Left -> Type)

'nonterm' relational_expression (-> EXPRESSION)
	'rule' relational_expression (-> Expr): 
		shift_expression (-> Expr)
		
	'rule' relational_expression (-> comparison(Op, Left, Right)): 
		relational_expression (-> Left) relational_operator (-> Op) shift_expression (-> Right)

'nonterm' relational_operator (-> COMPARISON_OP)
	'rule' relational_operator (-> geq): ">="
	'rule' relational_operator (-> leq): "<="
	'rule' relational_operator (-> gtr): ">"
	'rule' relational_operator (-> lss): "<"
	
'nonterm' equality_expression (-> EXPRESSION)
	'rule' equality_expression (-> Expr): 
		relational_expression (-> Expr)
		
	'rule' equality_expression (-> comparison(Op, Left, Right)):
		equality_expression (-> Left) equality_operator(-> Op) relational_expression (-> Right)

'nonterm' equality_operator (-> COMPARISON_OP)
	'rule' equality_operator (-> equ): "=="
	'rule' equality_operator (-> neq): "!="

'nonterm' and_expression (-> EXPRESSION)
	'rule' and_expression (-> Expr): 
		equality_expression (-> Expr)
		
	'rule' and_expression (-> binary(Type, and, Left, Right)): 
		and_expression (-> Left) "&" equality_expression (-> Right)
		Coerce(Left, Right -> Type)

'nonterm' exclusive_or_expression (-> EXPRESSION)
	'rule' exclusive_or_expression (-> Expr): 
		and_expression (-> Expr)
		
	'rule' exclusive_or_expression (-> binary(Type, xor, Left, Right)):
		exclusive_or_expression (-> Left) "^" and_expression (-> Right)
		Coerce(Left, Right -> Type)

'nonterm' inclusive_or_expression (-> EXPRESSION)
	'rule' inclusive_or_expression (-> Expr): 
		exclusive_or_expression (-> Expr)
		
	'rule' inclusive_or_expression (-> binary(Type, or, Left, Right)):
		inclusive_or_expression (-> Left) "|" exclusive_or_expression (-> Right)
		Coerce(Left, Right -> Type)

'nonterm' logical_and_expression (-> EXPRESSION)
	'rule' logical_and_expression (-> Expr): 
		inclusive_or_expression (-> Expr)
		
	'rule' logical_and_expression (-> logical(land, Left, Right)): 
		logical_and_expression (-> Left) "&&" inclusive_or_expression (-> Right)

'nonterm' logical_xor_expression (-> EXPRESSION)
	'rule' logical_xor_expression (-> Expr): 
		logical_and_expression (-> Expr)
		
	'rule' logical_xor_expression (-> logical(lxor, Left, Right)): 
		logical_xor_expression (-> Left) "^^" logical_and_expression (-> Right)

'nonterm' logical_or_expression (-> EXPRESSION)
	'rule' logical_or_expression (-> Expr): 
		logical_xor_expression (-> Expr)
		
	'rule' logical_or_expression (-> logical(lor, Left, Right)): 
		logical_or_expression (-> Left) "||" logical_xor_expression (-> Right)

'nonterm' conditional_expression (-> EXPRESSION)
	'rule' conditional_expression (-> Expr): 
		logical_or_expression (-> Expr)
		
	'rule' conditional_expression (-> cond(Type, Cond, Left, Right)): 
		logical_or_expression (-> Cond) "?" expression (-> Left) ":" conditional_expression (-> Right)
		Coerce(Left, Right -> Type)

'nonterm' assignment_expression (-> EXPRESSION)
	'rule' assignment_expression (-> Expr): 
		conditional_expression (-> Expr)
		
	'rule' assignment_expression (-> assignment(Op, Left, Right)): 
		unary_expression (-> Left) assignment_operator (-> Op) assignment_expression (-> Right)

'nonterm' assignment_operator (-> ASSIGNMENT_OP)
	'rule' assignment_operator (-> asn)		: "="
	'rule' assignment_operator (-> asn_mul)	: "*="
	'rule' assignment_operator (-> asn_div)	: "/="
	'rule' assignment_operator (-> asn_mod)	: "%="
	'rule' assignment_operator (-> asn_add)	: "+="
	'rule' assignment_operator (-> asn_sub)	: "-="
	'rule' assignment_operator (-> asn_shl)	: "<<=" -- reserved
	'rule' assignment_operator (-> asn_shr)	: ">>=" -- reserved
	'rule' assignment_operator (-> asn_and)	: "&=" -- reserved
	'rule' assignment_operator (-> asn_xor)	: "^=" -- reserved
	'rule' assignment_operator (-> asn_or)	: "|=" -- reserved

'nonterm' expression (-> EXPRESSION)
	'rule' expression (-> Expr): 
		assignment_expression (-> Expr)
		
	'rule' expression (-> comma(Left, Right)): 
		expression (-> Left) "," assignment_expression (-> Right)

'nonterm' constant_expression (-> LITERAL)
	'rule' constant_expression (-> nil): 
		conditional_expression (-> Expr)

'nonterm' declaration
	'rule' declaration: function_prototype ";"
	'rule' declaration: init_declarator_list ";"

'nonterm' function_prototype
	'rule' function_prototype: function_declarator ")"

'nonterm' function_declarator
	'rule' function_declarator: function_header
	'rule' function_declarator: function_header_with_parameters

'nonterm' function_header_with_parameters
	'rule' function_header_with_parameters: function_header parameter_declaration
	'rule' function_header_with_parameters: function_header_with_parameters "," parameter_declaration

'nonterm' function_header
	'rule' function_header: fully_specified_type IDENTIFIER (-> Id) "("

'nonterm' parameter_declarator
	'rule' parameter_declarator: 
		type_specifier (-> Type) IDENTIFIER (-> Id)
		
	'rule' parameter_declarator: 
		type_specifier (-> Type) IDENTIFIER (-> Id) "[" "]"

'nonterm' parameter_declaration
	'rule' parameter_declaration: type_qualifier parameter_qualifier parameter_declarator
	'rule' parameter_declaration: parameter_qualifier parameter_declarator
	'rule' parameter_declaration: type_qualifier parameter_qualifier parameter_type_specifier
	'rule' parameter_declaration: parameter_qualifier parameter_type_specifier

'nonterm' parameter_qualifier
	'rule' parameter_qualifier: /* empty */
	'rule' parameter_qualifier: "in"
	'rule' parameter_qualifier: "out"
	'rule' parameter_qualifier: "inout"

'nonterm' parameter_type_specifier
	'rule' parameter_type_specifier: 
		type_specifier (-> Type)
		
	'rule' parameter_type_specifier: 
		type_specifier (-> Type) "[" "]"

'nonterm' init_declarator_list
	'rule' init_declarator_list: single_declaration
	'rule' init_declarator_list: init_declarator_list "," IDENTIFIER (-> Id)
	'rule' init_declarator_list: init_declarator_list "," IDENTIFIER (-> Id) "[" "]"
	'rule' init_declarator_list: init_declarator_list "," IDENTIFIER (-> Id) "[" constant_expression (-> Lit)
	'rule' init_declarator_list: "]"
	'rule' init_declarator_list: init_declarator_list "," IDENTIFIER (-> Id) "=" initializer

'nonterm' single_declaration
	'rule' single_declaration: fully_specified_type
	'rule' single_declaration: fully_specified_type IDENTIFIER (-> Id)
	'rule' single_declaration: fully_specified_type IDENTIFIER (-> Id) "[" "]"
	'rule' single_declaration: fully_specified_type IDENTIFIER (-> Id) "[" constant_expression (-> Lit) "]"
	'rule' single_declaration: fully_specified_type IDENTIFIER (-> Id) "=" initializer

'nonterm' fully_specified_type
	'rule' fully_specified_type: type_specifier (-> Type)
	'rule' fully_specified_type: type_qualifier type_specifier (-> Type)

'nonterm' type_qualifier
	'rule' type_qualifier: "const"
	'rule' type_qualifier: "attribute" -- Vertex only.
	'rule' type_qualifier: "varying"
	'rule' type_qualifier: "uniform"

'nonterm' type_specifier (-> TYPE)
	'rule' type_specifier (-> Type)				: constructor_identifier(-> Type)
	'rule' type_specifier (-> Type)				: struct_specifier (-> Type)
	'rule' type_specifier (-> void)				: "void"
	-- 'rule' type_specifier: "sampler1d"
	'rule' type_specifier (-> sampler2d)		: "sampler2d"
	-- 'rule' type_specifier: "sampler3d"
	-- 'rule' type_specifier: "samplercube"
	-- 'rule' type_specifier: "sampler1dshadow"
	-- 'rule' type_specifier: "sampler2dshadow"

'nonterm' struct_specifier (-> TYPE)
	'rule' struct_specifier (-> struct): 
		"struct" IDENTIFIER (-> Id) "{" struct_declaration_list "}"
		
	'rule' struct_specifier (-> struct): 
		"struct" "{" struct_declaration_list "}"

'nonterm' struct_declaration_list
	'rule' struct_declaration_list: struct_declaration
	'rule' struct_declaration_list: struct_declaration_list struct_declaration

'nonterm' struct_declaration
	'rule' struct_declaration: 
		type_specifier (-> Type) struct_declarator_list ";"

'nonterm' struct_declarator_list
	'rule' struct_declarator_list: struct_declarator
	'rule' struct_declarator_list: struct_declarator_list "," struct_declarator

'nonterm' struct_declarator
	'rule' struct_declarator: 
		IDENTIFIER (-> Id)
		
	'rule' struct_declarator: 
		IDENTIFIER (-> Id) "[" constant_expression (-> Lit) "]"

'nonterm' initializer
	'rule' initializer: assignment_expression (-> Expr)

'nonterm' declaration_statement (-> STATEMENT)
	'rule' declaration_statement (-> nil): 
		declaration

'nonterm' statement (-> STATEMENT)
	'rule' statement (-> Statement): compound_statement (-> Statement)
	'rule' statement (-> Statement): simple_statement (-> Statement)

'nonterm' simple_statement (-> STATEMENT)
	'rule' simple_statement (-> Statement): declaration_statement (-> Statement)
	'rule' simple_statement (-> Statement): expression_statement (-> Statement)
	'rule' simple_statement (-> Statement): selection_statement (-> Statement)
	'rule' simple_statement (-> Statement): iteration_statement (-> Statement)
	'rule' simple_statement (-> Statement): jump_statement (-> Statement)

'nonterm' compound_statement (-> STATEMENT)
	'rule' compound_statement (-> nil) : "{" "}"
	
	'rule' compound_statement (-> list(List)): 
		"{" statement_list (-> List) "}"

'nonterm' statement_no_new_scope (-> STATEMENT)
	'rule' statement_no_new_scope (-> Statement): compound_statement_no_new_scope (-> Statement)
	'rule' statement_no_new_scope (-> Statement): simple_statement (-> Statement)

'nonterm' compound_statement_no_new_scope (-> STATEMENT)
	'rule' compound_statement_no_new_scope (-> nil): "{" "}"
	
	'rule' compound_statement_no_new_scope (-> list(List)): 
		"{" statement_list (-> List) "}"

'nonterm' statement_list (-> STATEMENT_LIST)
	'rule' statement_list (-> cons(Statement, nil)): 
		statement (-> Statement)
		
	'rule' statement_list (-> List): 
		statement_list (-> Head) statement (-> Statement)
		AppendStatement(Head, Statement -> List)

'nonterm' expression_statement (-> STATEMENT)
	'rule' expression_statement (-> nil): ";"
	
	'rule' expression_statement (-> expr(Expr)): 
		expression (-> Expr) ";"

'nonterm' selection_statement (-> STATEMENT)
	'rule' selection_statement (-> if(Expr, IfPart, ElsePart)): 
		"if" "(" expression (-> Expr) ")" statement (-> IfPart) "else" statement (-> ElsePart)
		
	'rule' selection_statement (-> if(Expr, IfPart, nil)): 
		"if" "(" expression (-> Expr) ")" statement (-> IfPart)
		
	'rule' selection_statement (-> switch(Expr, CaseList)): 
		"switch" "(" expression (-> Expr) ")" "{" case_block_list (-> CaseList) "}"
	
'nonterm' case_block_list (-> CASE_LIST)
	'rule' case_block_list (-> CASE_LIST'nil): 
		
	'rule' case_block_list (-> Result): 
		case_block_list (-> Head) case_label_list (-> Labels) case_statement_list (-> Statements) "break" ";"
		AppendCase(Head, case(Labels, Statements) -> Result)

'nonterm' case_label_list (-> LITERAL_LIST)
	'rule' case_label_list (-> nil): 
	'rule' case_label_list (-> Result): 
		case_label_list (-> Labels) "case" constant_expression (-> Lit) ":"
		AppendLiteral(Labels, Lit -> Result)
		
	'rule' case_label_list (-> Result): 
		case_label_list (-> Labels) "default" ":"
		AppendDefault(Labels -> Result)
	
'nonterm' case_statement_list (-> STATEMENT_LIST)
	'rule' case_statement_list (-> STATEMENT_LIST'cons(Statement, nil)): 
		statement_no_break_decl (-> Statement)
		
	'rule' case_statement_list (-> List): 
		case_statement_list (-> Head) statement_no_break_decl (-> Statement)
		AppendStatement(Head, Statement -> List)
	
'nonterm' statement_no_break_decl (-> STATEMENT)
	'rule' statement_no_break_decl (-> Statement): compound_statement (-> Statement)
	'rule' statement_no_break_decl (-> Statement): expression_statement (-> Statement)
	'rule' statement_no_break_decl (-> Statement): selection_statement (-> Statement)
	'rule' statement_no_break_decl (-> Statement): iteration_statement (-> Statement)
	'rule' statement_no_break_decl (-> Statement): jump_statement_no_break (-> Statement)

'nonterm' condition
	'rule' condition: expression (-> Expr)
	'rule' condition: fully_specified_type IDENTIFIER (-> Id) "=" initializer

'nonterm' iteration_statement (-> STATEMENT)
	'rule' iteration_statement (-> while(nil, Body)): 
		"while" "(" condition ")" statement_no_new_scope (-> Body)
		
	'rule' iteration_statement (-> do(Expr, Body)): 
		"do" statement (-> Body) "while" "(" expression (-> Expr) ")" ";"
		
--	'rule' iteration_statement: 
--		"for" "(" for_init_statement (-> Statement) conditionopt expressionopt (-> Expr) ")" statement_no_new_scope

--'nonterm' for_init_statement (-> STATEMENT)
--	'rule' for_init_statement(-> Stmnt): 
--		expression_statement(-> Stmnt)
		
--	'rule' for_init_statement(-> Stmnt): 
--		declaration_statement(-> Stmnt)

--'nonterm' conditionopt
--	'rule' conditionopt: condition ";"
--	'rule' conditionopt: /* empty */ ";"

--'nonterm' expressionopt (-> EXPRESSION)
--	'rule' expressionopt (-> nil): /* empty */
--	'rule' expressionopt (-> Expr): expression (-> Expr)

'nonterm' jump_statement (-> STATEMENT)
	'rule' jump_statement(-> break): 
		"break" ";"
		
	'rule' jump_statement(-> Stmnt): 
		jump_statement_no_break(-> Stmnt)
	
'nonterm' jump_statement_no_break (-> STATEMENT)
	'rule' jump_statement_no_break(-> continue): 
		"continue" ";"
		
	'rule' jump_statement_no_break(-> return(nil)): 
		"return" ";"
		
	'rule' jump_statement_no_break(-> return(Expr)): 
		"return" expression (-> Expr) ";"
		
	'rule' jump_statement_no_break(-> discard): 
		"discard" ";" -- Fragment shader only.

'nonterm' translation_unit
	'rule' translation_unit: translation_unit external_declaration
	'rule' translation_unit: external_declaration

'nonterm' external_declaration
	'rule' external_declaration: function_definition
	'rule' external_declaration: declaration

'nonterm' function_definition
	'rule' function_definition: function_prototype compound_statement_no_new_scope (-> Statement)
	