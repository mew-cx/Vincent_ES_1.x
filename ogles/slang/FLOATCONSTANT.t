[0-9]+"."[0-9]*[fx]? {
   yylval.attr[1] = atoi (yytext);
   yysetpos();
   return INTCONSTANT;
}
[0-9]+"."[0-9]*[EeDd][0-9]+[fx]? {
   yylval.attr[1] = atoi (yytext);
   yysetpos();
   return INTCONSTANT;
}

