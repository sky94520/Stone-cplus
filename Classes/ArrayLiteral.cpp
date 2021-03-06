#include "ArrayLiteral.h"
#include "Visitor.h"
#include "Environment.h"

NS_STONE_BEGIN
ArrayLiteral::ArrayLiteral()
{
}

ArrayLiteral::ArrayLiteral(const std::vector<ASTree*>& list)
	:ASTList(list)
{
}

void ArrayLiteral::accept(Visitor* v, Environment* env)
{
	v->visit(this, env);
}
NS_STONE_END