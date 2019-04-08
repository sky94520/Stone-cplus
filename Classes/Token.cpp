#include "Token.h"
Token* const Token::TOKEN_EOF = new Token(-1);
const std::string Token::TOKEN_EOL = "\\n";

Token::Token(int line)
	:_line(line)
	,_type(Type::None)
{
}

Token::~Token()
{
}

//-------------------------NumToken-----------------------------
NumToken::NumToken(int line, int value)
	:Token(line)
	,_value(value)
{
	_type = Type::Number;
}

std::string NumToken::getText() const
{
	return std::to_string(_value);
}

int NumToken::getNumber() const {
	return _value;
}

//-------------------------IdToken-----------------------------
IdToken::IdToken(int line,const std::string& id)
	:Token(line)
	,_text(id)
{
	_type = Type::Identifier;
}

std::string IdToken::getText() const
{
	return _text;
}

//-------------------------StrToken-----------------------------
StrToken::StrToken(int line, const std::string& str)
	:Token(line)
	,_literal(str)
{
	_type = Type::String;
}

std::string StrToken::getText() const
{
	return _literal;
}