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
--						o	simple enum declrations
--						o	simplified switch statements
--
-----------------------------------------------------------------------------
--
-- 10-27-2003		Hans-Martin Will	initial version
--
--===========================================================================


'token' IDENTIFIER
'token' TYPE_NAME
'token' FLOATCONSTANT
'token' INTCONSTANT
'token' BOOLCONSTANT
'token' FIELD_SELECTION

'root' translation_unit

'nonterm' variable_identifier
	'rule' variable_identifier: IDENTIFIER

'nonterm' primary_expression
	'rule' primary_expression: variable_identifier
	'rule' primary_expression: INTCONSTANT
	'rule' primary_expression: FLOATCONSTANT
	'rule' primary_expression: BOOLCONSTANT
	'rule' primary_expression: "(" expression ")"

'nonterm' postfix_expression
	'rule' postfix_expression: primary_expression
	'rule' postfix_expression: postfix_expression "[" integer_expression "]"
	'rule' postfix_expression: function_call
	'rule' postfix_expression: postfix_expression "." IDENTIFIER -- was: FIELD_SELECTION
	'rule' postfix_expression: postfix_expression "++"
	'rule' postfix_expression: postfix_expression "--"
	
'nonterm' integer_expression
	'rule' integer_expression: expression

'nonterm' function_call
	'rule' function_call: function_call_generic
	
'nonterm' function_call_generic
	'rule' function_call_generic: function_call_header_with_parameters ")"
	'rule' function_call_generic: function_call_header_no_parameters ")"

'nonterm' function_call_header_no_parameters
	'rule' function_call_header_no_parameters: function_call_header "void"
	'rule' function_call_header_no_parameters: function_call_header

'nonterm' function_call_header_with_parameters
	'rule' function_call_header_with_parameters: function_call_header assignment_expression
	'rule' function_call_header_with_parameters: function_call_header_with_parameters "," assignment_expression

'nonterm' function_call_header
	'rule' function_call_header: function_identifier "("

'nonterm' function_identifier
	'rule' function_identifier: constructor_identifier
	'rule' function_identifier: IDENTIFIER

'nonterm' constructor_identifier
	'rule' constructor_identifier: "float"
	'rule' constructor_identifier: "fixed"
	'rule' constructor_identifier: "ubyte"
	'rule' constructor_identifier: "ushort"
	'rule' constructor_identifier: "int"
	'rule' constructor_identifier: "bool"
	'rule' constructor_identifier: "vec2"
	'rule' constructor_identifier: "vec3"
	'rule' constructor_identifier: "vec4"
	'rule' constructor_identifier: "uvec2"
	'rule' constructor_identifier: "uvec3"
	'rule' constructor_identifier: "uvec4"
	'rule' constructor_identifier: "svec2"
	'rule' constructor_identifier: "svec3"
	'rule' constructor_identifier: "svec4"
	'rule' constructor_identifier: "xvec2"
	'rule' constructor_identifier: "xvec3"
	'rule' constructor_identifier: "xvec4"
	'rule' constructor_identifier: "bvec2"
	'rule' constructor_identifier: "bvec3"
	'rule' constructor_identifier: "bvec4"
	'rule' constructor_identifier: "ivec2"
	'rule' constructor_identifier: "ivec3"
	'rule' constructor_identifier: "ivec4"
	'rule' constructor_identifier: "mat2"
	'rule' constructor_identifier: "mat3"
	'rule' constructor_identifier: "mat4"

'nonterm' unary_expression
	'rule' unary_expression: postfix_expression
	'rule' unary_expression: "++" unary_expression
	'rule' unary_expression: "--" unary_expression
	'rule' unary_expression: unary_operator unary_expression

'nonterm' unary_operator
	'rule' unary_operator: "+"
	'rule' unary_operator: "-"
	'rule' unary_operator: "!"
	'rule' unary_operator: "~" -- reserved

'nonterm' multiplicative_expression
	'rule' multiplicative_expression: unary_expression
	'rule' multiplicative_expression: multiplicative_expression "*" unary_expression
	'rule' multiplicative_expression: multiplicative_expression "/" unary_expression
	'rule' multiplicative_expression: multiplicative_expression "%" unary_expression -- reserved

'nonterm' additive_expression
	'rule' additive_expression: multiplicative_expression
	'rule' additive_expression: additive_expression "+" multiplicative_expression
	'rule' additive_expression: additive_expression "-" multiplicative_expression

'nonterm' shift_expression
	'rule' shift_expression: additive_expression
	'rule' shift_expression: shift_expression "<<" additive_expression -- reserved
	'rule' shift_expression: shift_expression ">>" additive_expression -- reserved

'nonterm' relational_expression
	'rule' relational_expression: shift_expression
	'rule' relational_expression: relational_expression "<" shift_expression
	'rule' relational_expression: relational_expression ">" shift_expression
	'rule' relational_expression: relational_expression "<=" shift_expression
	'rule' relational_expression: relational_expression ">=" shift_expression

'nonterm' equality_expression
	'rule' equality_expression: relational_expression
	'rule' equality_expression: equality_expression "==" relational_expression
	'rule' equality_expression: equality_expression "!=" relational_expression

'nonterm' and_expression
	'rule' and_expression: equality_expression
	'rule' and_expression: and_expression "&" equality_expression -- reserved

'nonterm' exclusive_or_expression
	'rule' exclusive_or_expression: and_expression
	'rule' exclusive_or_expression: exclusive_or_expression "^" and_expression -- reserved

'nonterm' inclusive_or_expression
	'rule' inclusive_or_expression: exclusive_or_expression
	'rule' inclusive_or_expression: inclusive_or_expression "|" exclusive_or_expression -- reserved

'nonterm' logical_and_expression
	'rule' logical_and_expression: inclusive_or_expression
	'rule' logical_and_expression: logical_and_expression "&&" inclusive_or_expression

'nonterm' logical_xor_expression
	'rule' logical_xor_expression: logical_and_expression
	'rule' logical_xor_expression: logical_xor_expression "^^" logical_and_expression

'nonterm' logical_or_expression
	'rule' logical_or_expression: logical_xor_expression
	'rule' logical_or_expression: logical_or_expression "||" logical_xor_expression

'nonterm' conditional_expression
	'rule' conditional_expression: logical_or_expression
	'rule' conditional_expression: logical_or_expression "?" expression ":" conditional_expression

'nonterm' assignment_expression
	'rule' assignment_expression: conditional_expression
	'rule' assignment_expression: unary_expression assignment_operator assignment_expression

'nonterm' assignment_operator
	'rule' assignment_operator: "="
	'rule' assignment_operator: "*="
	'rule' assignment_operator: "/="
	'rule' assignment_operator: "%="
	'rule' assignment_operator: "+="
	'rule' assignment_operator: "-="
	'rule' assignment_operator: "<<=" -- reserved
	'rule' assignment_operator: ">>=" -- reserved
	'rule' assignment_operator: "&=" -- reserved
	'rule' assignment_operator: "^=" -- reserved
	'rule' assignment_operator: "|=" -- reserved

'nonterm' expression
	'rule' expression: assignment_expression
	'rule' expression: expression "," assignment_expression

'nonterm' constant_expression
	'rule' constant_expression: conditional_expression

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
	'rule' function_header: fully_specified_type IDENTIFIER "("

'nonterm' parameter_declarator
	'rule' parameter_declarator: type_specifier IDENTIFIER
	'rule' parameter_declarator: type_specifier IDENTIFIER "[" "]"

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
	'rule' parameter_type_specifier: type_specifier
	'rule' parameter_type_specifier: type_specifier "[" "]"

'nonterm' init_declarator_list
	'rule' init_declarator_list: single_declaration
	'rule' init_declarator_list: init_declarator_list "," IDENTIFIER
	'rule' init_declarator_list: init_declarator_list "," IDENTIFIER "[" "]"
	'rule' init_declarator_list: init_declarator_list "," IDENTIFIER "[" constant_expression
	'rule' init_declarator_list: "]"
	'rule' init_declarator_list: init_declarator_list "," IDENTIFIER "=" initializer

'nonterm' single_declaration
	'rule' single_declaration: fully_specified_type
	'rule' single_declaration: fully_specified_type IDENTIFIER
	'rule' single_declaration: fully_specified_type IDENTIFIER "[" "]"
	'rule' single_declaration: fully_specified_type IDENTIFIER "[" constant_expression "]"
	'rule' single_declaration: fully_specified_type IDENTIFIER "=" initializer

'nonterm' fully_specified_type
	'rule' fully_specified_type: type_specifier
	'rule' fully_specified_type: type_qualifier type_specifier

'nonterm' type_qualifier
	'rule' type_qualifier: "const"
	'rule' type_qualifier: "attribute" -- Vertex only.
	'rule' type_qualifier: "varying"
	'rule' type_qualifier: "uniform"

'nonterm' type_specifier
	'rule' type_specifier: "void"
	'rule' type_specifier: "float"
	'rule' type_specifier: "fixed"
	'rule' type_specifier: "ubyte"
	'rule' type_specifier: "ushort"
	'rule' type_specifier: "int"
	'rule' type_specifier: "bool"
	'rule' type_specifier: "vec2"
	'rule' type_specifier: "vec3"
	'rule' type_specifier: "vec4"
	'rule' type_specifier: "uvec2"
	'rule' type_specifier: "uvec3"
	'rule' type_specifier: "uvec4"
	'rule' type_specifier: "svec2"
	'rule' type_specifier: "svec3"
	'rule' type_specifier: "svec4"
	'rule' type_specifier: "xvec2"
	'rule' type_specifier: "xvec3"
	'rule' type_specifier: "xvec4"
	'rule' type_specifier: "bvec2"
	'rule' type_specifier: "bvec3"
	'rule' type_specifier: "bvec4"
	'rule' type_specifier: "ivec2"
	'rule' type_specifier: "ivec3"
	'rule' type_specifier: "ivec4"
	'rule' type_specifier: "mat2"
	'rule' type_specifier: "mat3"
	'rule' type_specifier: "mat4"
	'rule' type_specifier: "sampler1d"
	'rule' type_specifier: "sampler2d"
	'rule' type_specifier: "sampler3d"
	'rule' type_specifier: "samplercube"
	'rule' type_specifier: "sampler1dshadow"
	'rule' type_specifier: "sampler2dshadow"
	'rule' type_specifier: struct_specifier
	'rule' type_specifier: enum_specifier
	'rule' type_specifier: TYPE_NAME

'nonterm' struct_specifier
	'rule' struct_specifier: "struct" IDENTIFIER "{" struct_declaration_list "}"
	'rule' struct_specifier: "struct" "{" struct_declaration_list "}"

'nonterm' enum_specifier
	'rule' enum_specifier: "enum" IDENTIFIER "{" enum_declaration_list "}"
	
'nonterm' enum_declaration_list
	'rule' enum_declaration_list: enum_declaration
	'rule' enum_declaration_list: enum_declaration_list "," enum_declaration
	
'nonterm' enum_declaration
	'rule' enum_declaration: IDENTIFIER
	'rule' enum_declaration: IDENTIFIER "=" constant_expression
	
'nonterm' struct_declaration_list
	'rule' struct_declaration_list: struct_declaration
	'rule' struct_declaration_list: struct_declaration_list struct_declaration

'nonterm' struct_declaration
	'rule' struct_declaration: type_specifier struct_declarator_list ";"

'nonterm' struct_declarator_list
	'rule' struct_declarator_list: struct_declarator
	'rule' struct_declarator_list: struct_declarator_list "," struct_declarator

'nonterm' struct_declarator
	'rule' struct_declarator: IDENTIFIER
	'rule' struct_declarator: IDENTIFIER "[" constant_expression "]"

'nonterm' initializer
	'rule' initializer: assignment_expression

'nonterm' declaration_statement
	'rule' declaration_statement: declaration

'nonterm' statement
	'rule' statement: compound_statement
	'rule' statement: simple_statement

'nonterm' simple_statement
	'rule' simple_statement: declaration_statement
	'rule' simple_statement: expression_statement
	'rule' simple_statement: selection_statement
	'rule' simple_statement: iteration_statement
	'rule' simple_statement: jump_statement

'nonterm' compound_statement
	'rule' compound_statement: "{" "}"
	'rule' compound_statement: "{" statement_list "}"

'nonterm' statement_no_new_scope
	'rule' statement_no_new_scope: compound_statement_no_new_scope
	'rule' statement_no_new_scope: simple_statement

'nonterm' compound_statement_no_new_scope
	'rule' compound_statement_no_new_scope: "{" "}"
	'rule' compound_statement_no_new_scope: "{" statement_list "}"

'nonterm' statement_list
	'rule' statement_list: statement
	'rule' statement_list: statement_list statement

'nonterm' expression_statement
	'rule' expression_statement: ";"
	'rule' expression_statement: expression ";"

'nonterm' selection_statement
	'rule' selection_statement: "if" "(" expression ")" selection_rest_statement
	'rule' selection_statement: "switch" "(" expression ")" "{" case_block_list "}"
	
'nonterm' case_block_list
	'rule' case_block_list: case_label_list case_statement_list
	'rule' case_block_list: case_block_list "break" case_label_list case_statement_list opt_break

'nonterm' opt_break
	'rule' opt_break: /* empty */
	'rule' opt_break: "break"

'nonterm' case_label_list
	'rule' case_label_list: "case" constant_expression ":"
	'rule' case_label_list: "case" "default" ":"
	'rule' case_label_list: case_label_list "case" constant_expression ":"
	'rule' case_label_list: case_label_list "case" "default" ":"
	
'nonterm' case_statement_list
	'rule' case_statement_list: statement_no_break_decl
	'rule' case_statement_list: case_statement_list statement_no_break_decl
	
'nonterm' statement_no_break_decl
	'rule' statement_no_break_decl: compound_statement
	'rule' statement_no_break_decl: expression_statement
	'rule' statement_no_break_decl: selection_statement
	'rule' statement_no_break_decl: iteration_statement
	'rule' statement_no_break_decl: jump_statement_no_break

'nonterm' selection_rest_statement
	'rule' selection_rest_statement: statement "else" statement
	'rule' selection_rest_statement: statement

'nonterm' condition
	'rule' condition: expression
	'rule' condition: fully_specified_type IDENTIFIER "=" initializer

'nonterm' iteration_statement
	'rule' iteration_statement: "while" "(" condition ")" statement_no_new_scope
	'rule' iteration_statement: "do" statement "while" "(" expression ")" ";"
	'rule' iteration_statement: "for" "(" for_init_statement for_rest_statement ")" statement_no_new_scope

'nonterm' for_init_statement
	'rule' for_init_statement: expression_statement
	'rule' for_init_statement: declaration_statement

'nonterm' conditionopt
	'rule' conditionopt: condition
	'rule' conditionopt: /* empty */

'nonterm' for_rest_statement
	'rule' for_rest_statement: conditionopt ";"
	'rule' for_rest_statement: conditionopt ";" expression

'nonterm' jump_statement
	'rule' jump_statement: "break" ";"
	'rule' jump_statement: jump_statement_no_break
	
'nonterm' jump_statement_no_break
	'rule' jump_statement_no_break: "continue" ";"
	'rule' jump_statement_no_break: "return" ";"
	'rule' jump_statement_no_break: "return" expression ";"
	'rule' jump_statement_no_break: "discard" ";" -- Fragment shader only.

'nonterm' translation_unit
	'rule' translation_unit: translation_unit external_declaration
	'rule' translation_unit: external_declaration

'nonterm' external_declaration
	'rule' external_declaration: function_definition
	'rule' external_declaration: declaration

'nonterm' function_definition
	'rule' function_definition: function_prototype compound_statement_no_new_scope
	