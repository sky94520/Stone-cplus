#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <regex>
#include <memory>
#include <sstream>

#include "../Classes/Lexer.h"
#include "../Classes/Token.h"
#include "../Classes/Value.h"
#include "../Classes/ASTree.h"
#include "../Classes/ASTLeaf.h"
#include "../Classes/ParseException.h"
#include "../Classes/StoneException.h"
#include "../Classes/NestedEnv.h"
#include "../Classes/EvalVisitor.h"
#include "../Classes/STAutoreleasePool.h"
#include "../Classes/BasicParser.h"
#include "../Classes/FuncParser.h"
#include "../Classes/ClosureParser.h"

using namespace std;
USING_NS_STONE;

std::unique_ptr<char> getUniqueDataFromFile(const std::string& filename);
void outputLexer(Lexer* lexer);
Value print(Environment* env);

int main() {
	auto uniquePtr = std::move(getUniqueDataFromFile("function.txt"));
	if (uniquePtr == nullptr)
	{
		cout << "文件打开失败" << endl;
		return 1;
	}
	Lexer* lexer = new Lexer(uniquePtr.get());
	uniquePtr.reset();
	//创建环境
	NestedEnv* env = new NestedEnv();
	//加入方法
	const char* params[] = { "value" };
	env->putNative("print", print, params, 1);
	//创建解析器
	EvalVisitor* visitor = new EvalVisitor();
	//语法分析树
	auto parser = new ClosureParser();

	try
	{
		//词法分析
		int line = 1;
		while (lexer->peek(0) != Token::TOKEN_EOF)
		{
			auto t = parser->parse(lexer);
			//暂时没想到跳过NullStmnt的好方法
			if (t->getNumChildren() > 0)
			{
				cout << t->toString() << "=>"<< endl;
				t->accept(visitor, env);
				cout << visitor->result->asString() << endl;
			}
			//不需要手动释放内存
			AutoreleasePool::getInstance()->clear();
		}
	}
	catch (ParseException& e)
	{
		cout << e.what() << endl;
	}
	catch (StoneException& e)
	{
		cout << e.what() << endl;
	}

	delete parser;
	delete visitor;
	delete env;
	delete lexer;
	delete Token::TOKEN_EOF;
	AutoreleasePool::purge();

	return 0;
}

void outputLexer(Lexer* lexer)
{
	Token* token = lexer->read();
	while (token != Token::TOKEN_EOF) {

		cout << token->getLineNumber();
		auto type = token->getType();
		if (type == Token::Type::Identifier)
			cout << " 标识符";
		else if (type == Token::Type::Number)
			cout << " 数字";
		else if (type == Token::Type::String)
			cout << " 字符串";
		cout << token->asString() << endl;

		delete token;
		token = lexer->read();
	}
}

std::unique_ptr<char> getUniqueDataFromFile(const std::string& filename) {
	std::unique_ptr<char> points;
	std::ifstream in;
	in.open(filename);

	//打开文件失败
	if (!in.is_open()) {
		return points;
	}
	//定位到文件的末尾
	in.seekg(0, std::ios::end);
	//获取总长度
	int size = (int)in.tellg();

	char* buffer = new char[size + 1];
	memset(buffer, '\0', size + 1);
	
	//读取
	in.seekg(0, std::ios::beg);
	in.read(buffer, size);
	//关闭文件
	in.close();

	points = std::unique_ptr<char>(buffer);
	return points;
}

Value print(Environment* env)
{
	//获取参数
	const Value* value = env->get("value");
	std::cout << value->asString() << std::endl;

	return *value;
}
