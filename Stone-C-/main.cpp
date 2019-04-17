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
#include "../Classes/Parser.h"
#include "../Classes/ASTree.h"
#include "../Classes/ASTLeaf.h"
#include "../Classes/ParseException.h"
#include "../Classes/StoneException.h"
#include "../Classes/NestedEnv.h"
#include "../Classes/EvalVisitor.h"
#include "../Classes/STAutoreleasePool.h"

using namespace std;
USING_NS_STONE;

std::unique_ptr<char> getUniqueDataFromFile(const std::string& filename);
void outputLexer(Lexer* lexer);
Value print(Environment* env);

int main() {
	auto uniquePtr = std::move(getUniqueDataFromFile("1.txt"));
	if (uniquePtr == nullptr)
	{
		cout << "�ļ���ʧ��" << endl;
		return 1;
	}
	Lexer* lexer = new Lexer(uniquePtr.get());
	uniquePtr.reset();

	Parser* parser = new Parser();
	parser->setLexer(lexer);
	//��������
	NestedEnv* env = new NestedEnv();
	//���뷽��
	const char* params[] = { "value" };
	env->putNative("print", print, params, 1);
	//����������
	EvalVisitor* visitor = new EvalVisitor();

	try
	{
		//�ʷ�����
		while (lexer->peek(0) != Token::TOKEN_EOF)
		{
		//�﷨����
			auto t = parser->parse();

			if (t != nullptr) 
			{
				//����
				t->accept(visitor, env);
				cout << t->toString() << "=>" << visitor->result->asString() << endl;
				t->release();
			}
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

	delete visitor;
	delete env;
	delete parser;
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
			cout << " ��ʶ��";
		else if (type == Token::Type::Number)
			cout << " ����";
		else if (type == Token::Type::String)
			cout << " �ַ���";
		cout << token->asString() << endl;

		delete token;
		token = lexer->read();
	}
}

std::unique_ptr<char> getUniqueDataFromFile(const std::string& filename) {
	std::unique_ptr<char> points;
	std::ifstream in;
	in.open(filename);

	//���ļ�ʧ��
	if (!in.is_open()) {
		return points;
	}
	//��λ���ļ���ĩβ
	in.seekg(0, std::ios::end);
	//��ȡ�ܳ���
	int size = (int)in.tellg();

	char* buffer = new char[size + 1];
	memset(buffer, '\0', size + 1);
	
	//��ȡ
	in.seekg(0, std::ios::beg);
	in.read(buffer, size);
	//�ر��ļ�
	in.close();

	points = std::unique_ptr<char>(buffer);
	return points;
}

Value print(Environment* env)
{
	//��ȡ����
	const Value* value = env->get("value");
	std::cout << value->asString() << std::endl;

	return *value;
}