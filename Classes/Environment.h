#ifndef __Stone_Environment_H__
#define __Stone_Environment_H__

#include <string>

#include "STObject.h"
#include "Value.h"
#include "NativeFunction.h"

NS_STONE_BEGIN

class Environment: public Object
{
public:
	virtual ~Environment() {}
	//在本环境内直接插入
	virtual void putNew(const std::string& name, const Value& value) = 0;
	//插入数据
	virtual void put(const std::string& name, const Value& value) = 0;

	void putNative(const std::string& name, const nativeFunc& callback, const char** params, int len)
	{
		NativeFunction* function = new NativeFunction(params, len, callback, this);
		this->putNew(name, Value(function));
		function->release();
	}
	//获取数据
	virtual const Value* get(const std::string& name) const = 0;
};
NS_STONE_END
#endif