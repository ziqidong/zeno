%top{
    #include <stdint.h>
}

%{
    #include <iostream>
    #include <cstdlib>
    #include "zfxscanner.h"
    #include <zeno/formula/zfxexecute.h>
    #include "zfxparser.hpp"
    #include "location.hh"

    static zeno::location loc;    //����λ��ʵ��

    using namespace std;

    // Original yyterminate() macro returns int. Since we're using Bison 3 variants
    // as tokens, we must redefine it to change type from `int` to `Parser::semantic_type`
    #define yyterminate() zeno::ZfxParser::make_END(zeno::location());

    // This will track current scanner location.
    // Action is called when length of the token is known.
    #define YY_USER_ACTION m_driver.increaseLocation(yyleng, yytext);
    
    // !!!WARNING!!!
    // Location API is used, but the location is not initialized, 'cause I'm lazy. When making
    // a token with make_{something} method you can pass detailed token location. Current location
    // is accessible with m_driver.location() method. All puzzle elements are there - just
    // pass location value in every action code block below. I'm going to waste more time writing
    // this excuse than putting this boilerplate below...
    //
    // Location class can be found in location.hh and posistion.hh files. It's just a bit too much
    // boilerplate for this small example. Bummer.
%}

%option nodefault
%option noyywrap
%option c++
%option yyclass="ZfxScanner"
%option prefix="ZFX_"

/*����ʽ�ĸ��ּ�д��ʹ�����в�θж��Ե����� */
string  \"[^\n"]+\"
uncompstring \"[^\n"]*
ws      [ \t]+
alpha   [A-Za-z]
dig     [0-9]
/*name    (\$)({alpha}|{dig}|[_.\-/$])*/
num1    {dig}+\.?([eE][-]?{dig}+)?
num2    {dig}*\.{dig}+([eE][-]?{dig}+)?
number  {num1}|{num2}
frame   \$F
fps     \$FPS
pi      \$PI
varname [A-Za-z][A-Za-z_0-9]*
dollar \$
zenvarname \$[A-Za-z_]+
compareop (<|<=|>=|>)
semicolon ;
autoinc \+\+
autodec \-\-
equalto \=

%%
%{
  // C++ ���ݵĴʷ��������Ĺ���step������λ�õ���ʼֵ����Ϊ�����ֵ��ȣ�����λ�þ�ָ������һ�����ٵĽ���λ�á�
  loc.step();
%}

{number}    {
                return ZfxParser::make_NUMBER(std::strtof(yytext,0),loc); // strtof�������ַ���ת��Ϊ������
            }

"//".* |
[ \t]   {
            /* ����ע�ͺͿհ׷��� */
            // step������λ�õ���ʼֵ����Ϊ�����ֵ��ȣ�����λ�þ�ָ������һ�����ٵĽ���λ�á�
            // ����ע�ͺͿհ׷���ʶ��󲢲��᷵�أ���ǰһ��step�ĵ���������һ��yylex����ʱ�����Դ˴���Ҫ�ֶ����¼Ǻŵ���ʼλ��
            loc.step();
        }

\n      {
            loc.lines(yyleng);  //ʹ��lines����������λ����Ϣ�еķ���
            loc.step();
            //return zeno::ZfxParser::make_EOL(zeno::location());
        }

"+"       { return zeno::ZfxParser::make_ADD(loc); }
"-"       { return zeno::ZfxParser::make_SUB(loc); }
"*"       { return zeno::ZfxParser::make_MUL(loc); }
"/"       { return zeno::ZfxParser::make_DIV(loc); }
"("       { return zeno::ZfxParser::make_LPAREN(yytext,loc); }
")"       { return zeno::ZfxParser::make_RPAREN(yytext,loc); }
","       { return zeno::ZfxParser::make_COMMA(loc); }
"?"       { return zeno::ZfxParser::make_QUESTION(yytext,loc); }
":"       { return zeno::ZfxParser::make_COLON(yytext,loc); }
"\."      { return zeno::ZfxParser::make_DOT(yytext,loc); }
"{"       { return zeno::ZfxParser::make_LBRACKET(yytext,loc); }
"}"       { return zeno::ZfxParser::make_RBRACKET(yytext,loc); }
"\["       { return zeno::ZfxParser::make_LSQBRACKET(yytext,loc); }
"\]"       { return zeno::ZfxParser::make_RSQBRACKET(yytext,loc); }
{string}  { return zeno::ZfxParser::make_LITERAL(yytext, loc); }
{uncompstring} { return zeno::ZfxParser::make_UNCOMPSTR(yytext, loc); }
{autoinc} { return zeno::ZfxParser::make_AUTOINC(yytext, loc); }
{autodec} { return zeno::ZfxParser::make_AUTODEC(yytext, loc); }
{varname} { 
        if (strcmp(yytext, "return") == 0) {
            return zeno::ZfxParser::make_RETURN(yytext, loc);
        } else if (strcmp(yytext, "break") == 0) {
            return zeno::ZfxParser::make_BREAK(yytext, loc);
        } else if (strcmp(yytext, "continue") == 0) {
            return zeno::ZfxParser::make_CONTINUE(yytext, loc);
        } else if (strcmp(yytext, "if") == 0) {
            return zeno::ZfxParser::make_IF(yytext, loc);
        } else if (strcmp(yytext, "while") == 0) {
            return zeno::ZfxParser::make_WHILE(yytext, loc);
        } else if (strcmp(yytext, "for") == 0) {
            return zeno::ZfxParser::make_FOR(yytext, loc);
        }
        return zeno::ZfxParser::make_VARNAME(yytext, loc);
}
{semicolon} { return zeno::ZfxParser::make_SEMICOLON(yytext, loc); }
{equalto}   { return zeno::ZfxParser::make_EQUALTO(yytext, loc); }
"\+\="      { return zeno::ZfxParser::make_ADDASSIGN(yytext, loc); }
"\*\="      { return zeno::ZfxParser::make_MULASSIGN(yytext, loc); }
"\-\="      { return zeno::ZfxParser::make_SUBASSIGN(yytext, loc); }
"/\="       { return zeno::ZfxParser::make_DIVASSIGN(yytext, loc); }
{dollar}  { return zeno::ZfxParser::make_DOLLAR(yytext, loc); }
{zenvarname} { return zeno::ZfxParser::make_DOLLARVARNAME(yytext, loc); }
{compareop}  { return zeno::ZfxParser::make_COMPARE(yytext, loc); }
<<EOF>>   { return yyterminate(); }
.         {
             cout << "ZfxScanner: unknown character [" << yytext << "]" << endl;
          }

%%