[A-Za-z_][_A-Za-z0-9]* {  
	extern int TypeNameAllowed;
	long id;
	string_to_id (yytext, &id);
	yylval.attr[1] = id;
	yysetpos();

	// check if the identifier is bound to a type name; if so and if we are not redeclaring, return TYPE_NAME
	if (TypeNameAllowed) {
		TypeNameAllowed = 0;
		
		if (1 /* this is not a type name */ ) {
			return IDENTIFIER;
		} else {
			return TYPE_NAME;
		}
	}
	
	return IDENTIFIER;
}
