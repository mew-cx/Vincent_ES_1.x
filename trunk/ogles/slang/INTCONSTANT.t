[1-9][0-9]*[us]? {
   yylval.attr[1] = atoi (yytext);
   yysetpos();
   return INTCONSTANT;
}
0[0-7]*[us]? {
   yylval.attr[1] = atoi (yytext);
   yysetpos();
   return INTCONSTANT;
}
0x[0-9A-Fa-f]+[us]? {
   yylval.attr[1] = atoi (yytext);
   yysetpos();
   return INTCONSTANT;
}
