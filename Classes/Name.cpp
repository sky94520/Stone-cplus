#include "Name.h"
#include "Token.h"

NS_STONE_BEGIN

Name::Name(Token* token)
	:ASTLeaf(token)
{
}

std::string Name::getName() const
{
	return getToken()->asString();
}
NS_STONE_END