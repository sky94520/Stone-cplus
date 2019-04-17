#include "Parser.h"
#include "Lexer.h"
#include "ASTree.h"
#include "ParseException.h"
#include "ASTList.h"
#include "Factory.h"
#include "ASTLeaf.h"
#include "Token.h"
#include "NumberLiteral.h"
#include "Name.h"
#include "StringLiteral.h"

NS_STONE_BEGIN
//----------------------------------Tree--------------------------
Parser::Tree::Tree(Parser* parser)
	:_parser(parser)
{
	_parser->retain();
}

Parser::Tree::~Tree()
{
	_parser->release();
}

void Parser::Tree::parse(Lexer* lexer, std::vector<ASTree*>& result)
{
	result.push_back(_parser->parse(lexer));
}

bool Parser::Tree::match(Lexer* lexer) const
{
	return _parser->match(lexer);
}
//----------------------------------OrTree--------------------------
Parser::OrTree::OrTree(Parser* parsers[], unsigned n)
{
	unsigned i = 0;

	while (i < n)
	{
		Parser* parser = parsers[i];
		parser->retain();
		_parsers.push_back(parser);
	}
}

Parser::OrTree::~OrTree()
{
	auto it = _parsers.begin();

	while (it != _parsers.end())
	{
		Parser* parser = *it;
		parser->release();

		it = _parsers.erase(it);
	}
}

void Parser::OrTree::insert(Parser* parser)
{
	//在数组头部插入一个元素
	auto it = _parsers.begin();

	parser->retain();
	_parsers.insert(it, parser);
}

void Parser::OrTree::parse(Lexer* lexer, std::vector<ASTree*>& result)
{
	//选择一个合适的Parser对象
	Parser* p = this->choose(lexer);

	if (p == nullptr)
		throw ParseException(lexer->peek(0));
	else
		result.push_back(p->parse(lexer));
}

bool Parser::OrTree::match(Lexer* lexer) const
{
	return this->choose(lexer) != nullptr;
}

Parser* Parser::OrTree::choose(Lexer* lexer) const
{
	for (Parser* p : _parsers)
		if (p->match(lexer))
			return p;

	return nullptr;
}
//----------------------------------Repeat--------------------------
Parser::Repeat::Repeat(Parser* parser, bool once)
	:_parser(parser)
	,_onlyOnce(once)
{
	_parser->retain();
}

Parser::Repeat::~Repeat()
{
	_parser->release();
}

void Parser::Repeat::parse(Lexer* lexer, std::vector<ASTree*>& result)
{
	while (_parser->match(lexer))
	{
		ASTree* t = _parser->parse(lexer);
		//TODO:去掉children为0的ASTList
		if (dynamic_cast<ASTList*>(t) == nullptr || t->getNumChildren() > 0)
			result.push_back(t);

		if (_onlyOnce)
			break;
	}
}

bool Parser::Repeat::match(Lexer* lexer) const
{
	return _parser->match(lexer);
}
//----------------------------------AToken--------------------------
Parser::AToken::AToken(const std::string& factoryName)
	:_factoryName(factoryName)
{
}

Parser::AToken::~AToken()
{
}

void Parser::AToken::parse(Lexer* lexer, std::vector<ASTree*>& result)
{
	Token* token = lexer->read();

	if (test(token))
	{
		ASTree* leaf = Factory::make(_factoryName, token);
		result.push_back(leaf);
	}
	else
		throw ParseException(token);
}

bool Parser::AToken::match(Lexer* lexer) const
{
	return test(lexer->peek(0));
}
//----------------------------------IdToken--------------------------
Parser::IdToken::IdToken(const std::string& factoryName, const std::unordered_set<std::string>& set)
	:AToken(factoryName)
	,_reserved(set)
{
}

bool Parser::IdToken::test(Token* token) const
{
	return token->getType() == Token::Type::Identifier &&
		_reserved.find(token->asString()) != _reserved.end();
}

//----------------------------------NumToken--------------------------
Parser::NumToken::NumToken(const std::string& factoryName)
	:AToken(factoryName)
{
}

bool Parser::NumToken::test(Token* token) const
{
	return token->getType() == Token::Type::Number;
}

//----------------------------------StrToken--------------------------
Parser::StrToken::StrToken(const std::string& factoryName)
	:AToken(factoryName)
{
}

bool Parser::StrToken::test(Token* token) const
{
	return token->getType() == Token::Type::String;
}

//----------------------------------Leaf--------------------------
Parser::Leaf::Leaf(std::string tokens[], unsigned n)
{
	unsigned index = 0;
	while (index < n)
	{
		_tokens.push_back(tokens[index]);
	}
}

void Parser::Leaf::parse(Lexer* lexer, std::vector<ASTree*>& result)
{
	Token* t = lexer->read();

	//为标识符
	if (t->getType() == Token::Type::Identifier)
	{
		for (const std::string& token : _tokens)
		{
			if (token == t->asString())
			{
				find(result, t);
				return;
			}
		}
	}
	if (_tokens.size() > 0)
		throw ParseException(_tokens[0] + " expected.", t);
	else
		throw ParseException(t);
}

bool Parser::Leaf::match(Lexer* lexer) const
{
	Token* t = lexer->read();

	//为标识符
	if (t->getType() == Token::Type::Identifier)
	{
		for (const std::string& token : _tokens)
		{
			if (token == t->asString())
			{
				return true;
			}
		}
	}
	return false;
}

void Parser::Leaf::find(std::vector<ASTree*>& result, Token* token)
{
	result.push_back(new ASTLeaf(token));
}

//----------------------------------Skip--------------------------
Parser::Skip::Skip(std::string tokens[], unsigned n)
	:Leaf(tokens, n)
{
}

void Parser::Skip::find(std::vector<ASTree*>& result, Token* token)
{
	//仅消费，不生成
}

//----------------------------------Operators--------------------------
bool Parser::Operators::LEFT = true;
bool Parser::Operators::RIGHT = false;

Parser::Operators::Operators()
{
}

Parser::Operators::~Operators()
{
	auto it = _mapping.begin();
	//删除
	while (it != _mapping.end())
	{
		auto precedence = it->second;
		delete precedence;

		it = _mapping.erase(it);
	}
}

void Parser::Operators::add(const std::string& name, int prec, bool leftAssoc)
{
	auto precedence = new Precedence(prec, leftAssoc);
	_mapping.insert(std::make_pair(name, precedence));
}

Parser::Precedence* Parser::Operators::get(const std::string& name)
{
	auto it = _mapping.find(name);
	if (it == _mapping.end())
		return nullptr;

	return it->second;
}

//----------------------------------Expr--------------------------
Parser::Expr::Expr(const std::string& factoryName, Parser* exp, Operators* map)
	:_factoryName(factoryName)
	,_factor(exp)
	,_operators(map)
{
	_factor->retain();
}

Parser::Expr::~Expr()
{
	_factor->release();
	//TODO
	delete _operators;
}

void Parser::Expr::parse(Lexer* lexer, std::vector<ASTree*>& result)
{
	ASTree* right = _factor->parse(lexer);
	Precedence* prec;

	while ((prec = nextOperator(lexer)) != nullptr)
	{
		right = doShift(lexer, right, prec->value);
	}
	result.push_back(right);
}

bool Parser::Expr::match(Lexer* lexer) const
{
	return _factor->match(lexer);
}

ASTree* Parser::Expr::doShift(Lexer* lexer, ASTree* left, int prec)
{
	std::vector<ASTree*> list;
	list.push_back(left);
	list.push_back(new ASTLeaf(lexer->read()));

	ASTree* right = _factor->parse(lexer);
	Precedence* next;

	while ((next = nextOperator(lexer)) != nullptr
		&& rightIsExpr(prec, next))
		right = doShift(lexer, right, next->value);

	list.push_back(right);

	return Factory::make(_factoryName, list);
}

Parser::Precedence* Parser::Expr::nextOperator(Lexer* lexer)
{
	Token* token = lexer->peek(0);
	//为标识符,则获取名称对应的优先权
	if (token->getType() == Token::Type::Identifier)
		return _operators->get(token->asString());
	return nullptr;
}

bool Parser::Expr::rightIsExpr(int prec, Precedence* nextPrec)
{
	if (nextPrec->leftAssoc)
		return prec < nextPrec->value;
	else
		return prec <= nextPrec->value;
}
//----------------------------------Parser--------------------------
Parser::Parser(const std::string& factoryName)
	:_factoryName(factoryName)
{
}

Parser::Parser(const Parser& parser)
{
	this->_elements = parser._elements;
	this->_factoryName = parser._factoryName;
}

Parser::~Parser()
{
	auto it = _elements.begin();

	while (it != _elements.end())
	{
		Element* e = *it;
		delete e;

		it = _elements.erase(it);
	}
}

ASTree* Parser::parse(Lexer* lexer)
{
	std::vector<ASTree*> results;

	for (Element* e : _elements)
	{
		e->parse(lexer, results);
	}
	//TODO: 使用Factory生成ASTList的子类对象
	return Factory::make(_factoryName, results);
}

bool Parser::match(Lexer* lexer) const
{
	if (_elements.size() == 0)
		return true;
	else
	{
		Element* e = _elements.front();
		return e->match(lexer);
	}
}


void Parser::reset()
{
	//:_factoryName(ASTList::TREE_ID)
	reset(ASTList::TREE_ID);
}

void Parser::reset(const std::string& factoryName)
{
	auto it = _elements.begin();

	while (it != _elements.end())
	{
		Element* e = *it;
		delete e;

		it = _elements.erase(it);
	}
	_factoryName = factoryName;
}

Parser* Parser::rule()
{
	return rule(ASTList::TREE_ID);
}

Parser* Parser::rule(const std::string& factoryName)
{
	return new Parser(factoryName);
}

Parser* Parser::number()
{
	return number(NumberLiteral::TREE_ID);
}

Parser* Parser::number(const std::string& factoryName)
{
	_elements.push_back(new NumToken(factoryName));
	return this;
}

Parser* Parser::identifier(const std::unordered_set<std::string>& reserved)
{
	return identifier(Name::TREE_ID, reserved);
}

Parser* Parser::identifier(const std::string& factoryName, const std::unordered_set<std::string>& reserved)
{
	_elements.push_back(new IdToken(factoryName, reserved));
	return this;
}

Parser* Parser::string()
{
	return string(StringLiteral::TREE_ID);
}

Parser* Parser::string(const std::string& factoryName)
{
	_elements.push_back(new StrToken(factoryName));
	return this;
}

Parser* Parser::token(unsigned n, ...)
{
	std::string* tokens = new std::string[n];
	unsigned index = 0;

	va_list args;
	va_start(args, n);
	while (index < n)
	{
		const char* text = va_arg(args, const char*);
		tokens[index++] = text;
	}
	va_end(args);
	_elements.push_back(new Leaf(tokens, n));
	return this;
}

Parser* Parser::sep(unsigned n, ...)
{
}

Parser* Parser:: orTree(unsigned n, ...)
{
}
NS_STONE_END