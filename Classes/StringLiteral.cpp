#include "StringLiteral.h"
#include "Token.h"
#include "Visitor.h"

NS_STONE_BEGIN
const std::string StringLiteral::TREE_ID = "StringLiteral";

StringLiteral::StringLiteral(Token* token)
	:ASTLeaf(token)
{
}

std::string StringLiteral::getValue() const
{
	return getToken()->asString();
}

void StringLiteral::accept(Visitor* v, Environment* env)
{
	v->visit(this, env);
}

NS_STONE_END