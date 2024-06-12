%top{
    #include <stdint.h>
}

%{
    #include <iostream>
    #include <cstdlib>
    #include "scanner.h"
    #include <zeno/formula/formula.h>
    #include "parser.hpp"
    #include "location.hh"

    static zeno::location loc;    //����λ��ʵ��

    using namespace std;

    // Original yyterminate() macro returns int. Since we're using Bison 3 variants
    // as tokens, we must redefine it to change type from `int` to `Parser::semantic_type`
    #define yyterminate() zeno::Parser::make_END(zeno::location());

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
%option yyclass="Scanner"
%option prefix="Zeno_"

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
functoken [A-Za-z][A-Za-z_0-9]*
dollar \$
zenvarname \$[A-Za-z_]+


%%
%{
  // C++ ���ݵĴʷ��������Ĺ���step������λ�õ���ʼֵ����Ϊ�����ֵ��ȣ�����λ�þ�ָ������һ�����ٵĽ���λ�á�
  loc.step();
%}

{number}    {
                return Parser::make_NUMBER(std::strtof(yytext,0),loc); // strtof�������ַ���ת��Ϊ������
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
            return zeno::Parser::make_EOL(zeno::location());
        }

"+"       { return zeno::Parser::make_ADD(loc); }
"-"       { return zeno::Parser::make_SUB(loc); }
"*"       { return zeno::Parser::make_MUL(loc); }
"/"       { return zeno::Parser::make_DIV(loc); }
"("       { return zeno::Parser::make_LPAREN(yytext,loc); }
")"       { return zeno::Parser::make_RPAREN(yytext,loc); }
","       { return zeno::Parser::make_COMMA(loc); }
{string}  { return zeno::Parser::make_LITERAL(yytext, loc); }
{uncompstring} { return zeno::Parser::make_UNCOMPSTR(yytext, loc); }
{functoken} { return zeno::Parser::make_FUNC(yytext, loc); }
{dollar}  { return zeno::Parser::make_DOLLAR(yytext, loc); }
{zenvarname} { return zeno::Parser::make_VARNAME(yytext, loc); }
<<EOF>>   { return yyterminate(); }
.         {
             cout << "Scanner: unknown character [" << yytext << "]" << endl;
          }

%%