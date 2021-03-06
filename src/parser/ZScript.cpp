#include "CompileError.h"
#include "DataStructs.h"
#include "Scope.h"
#include "ZScript.h"

using namespace std;
using namespace ZScript;

// ZScript::Program

Program::Program(ASTProgram* node)
	: node(node),
	  table(*new SymbolTable()),
	  globalScope(*new GlobalScope(table))
{
	assert(node);

	// Create a ZScript::Script for every script in the program.
	for (vector<ASTScript*>::const_iterator it = node->scripts.begin();
		 it != node->scripts.end(); ++it)
	{
		scripts.push_back(new Script(*this, *it));
		Script* script = scripts.back();
		scriptsByName[script->getName()] = script;
		scriptsByNode[*it] = script;
	}
}

Program::~Program()
{
	for (vector<Script*>::iterator it = scripts.begin(); it != scripts.end(); ++it)
		delete *it;
	delete &table;
	delete &globalScope;
}

Script* Program::getScript(string const& name) const
{
	map<string, Script*>::const_iterator it = scriptsByName.find(name);
	if (it == scriptsByName.end()) return NULL;
	return it->second;
}

Script* Program::getScript(ASTScript* node) const
{
	map<ASTScript*, Script*>::const_iterator it = scriptsByNode.find(node);
	if (it == scriptsByNode.end()) return NULL;
	return it->second;
}

vector<Function*> Program::getUserGlobalFunctions() const
{
	vector<Function*> functions = globalScope.getLocalFunctions();
	for (vector<Function*>::iterator it = functions.begin(); it != functions.end();)
	{
		Function& function = **it;
		if (!function.node) it = functions.erase(it);
		else ++it;
	}
	return functions;
}

vector<Function*> Program::getUserFunctions() const
{
	vector<Function*> functions = getFunctionsInBranch(globalScope);
	for (vector<Function*>::iterator it = functions.begin(); it != functions.end();)
	{
		Function& function = **it;
		if (!function.node) it = functions.erase(it);
		else ++it;
	}
	return functions;
}

vector<CompileError const*> Program::getErrors() const
{
	vector<CompileError const*> errors;
	for (vector<Script*>::const_iterator it = scripts.begin(); it != scripts.end(); ++it)
	{
		vector<CompileError const*> scriptErrors = (*it)->getErrors();
		errors.insert(errors.end(), scriptErrors.begin(), scriptErrors.end());
	}
	return errors;
}

// ZScript::Script

Script::Script(Program& program, ASTScript* script) : node(script)
{
	assert(node);

	// Create script scope.
	scope = program.globalScope.makeScriptChild(*this);
	scope->varDeclsDeprecated = true;
}

string Script::getName() const {return node->name;}

ScriptType Script::getType() const {return node->type->type;}

Function* Script::getRun() const
{
	vector<Function*> possibleRuns = scope->getLocalFunctions("run");
	if (possibleRuns.size() != 1) return NULL;
	return possibleRuns[0];
}

vector<CompileError const*> Script::getErrors() const
{
	vector<CompileError const*> errors;
	string const& name = getName();

	// Failed to create scope means an existing script already has this name.
	if (!scope) errors.push_back(&CompileError::ScriptRedef);

	// Invalid script type.
	ScriptType scriptType = getType();
	if (scriptType != SCRIPTTYPE_GLOBAL
		&& scriptType != SCRIPTTYPE_ITEM
		&& scriptType != SCRIPTTYPE_FFC)
	{
		errors.push_back(&CompileError::ScriptBadType);
	}

	// Invalid run function.
	vector<Function*> possibleRuns = scope->getLocalFunctions("run");
	if (possibleRuns.size() > 1)
		errors.push_back(&CompileError::TooManyRun);
	else if (possibleRuns.size() == 0)
		errors.push_back(&CompileError::ScriptNoRun);
	else if (*possibleRuns[0]->returnType != ZVarType::ZVOID)
		errors.push_back(&CompileError::ScriptRunNotVoid);

	return errors;
}

// ZScript::Datum

Datum::Datum(Scope& scope, ZVarType const& type)
	: scope(scope), type(type), id(ScriptParser::getUniqueVarID())
{}

bool Datum::tryAddToScope(CompileErrorHandler& errorHandler)
{
	return scope.add(*this, errorHandler);
}

bool ZScript::isGlobal(Datum const& datum)
{
	return (datum.scope.isGlobal() || datum.scope.isScript())
		&& datum.getName();
}

optional<int> ZScript::getStackOffset(Datum const& datum)
{
	return lookupStackPosition(datum.scope, datum);
}

// ZScript::Literal

Literal* Literal::create(
		Scope& scope, ASTLiteral& node, ZVarType const& type,
		CompileErrorHandler& errorHandler)
{
	Literal* literal = new Literal(scope, node, type);
	if (literal->tryAddToScope(errorHandler)) return literal;
	delete literal;
	return NULL;
}

Literal::Literal(Scope& scope, ASTLiteral& node, ZVarType const& type)
	: Datum(scope, type), node(node)
{
	node.manager = this;
}

// ZScript::Variable

Variable* Variable::create(
		Scope& scope, ASTDataDecl& node, ZVarType const& type,
		CompileErrorHandler& errorHandler)
{
	Variable* variable = new Variable(scope, node, type);
	if (variable->tryAddToScope(errorHandler)) return variable;
	delete variable;
	return NULL;
}

Variable::Variable(
		Scope& scope, ASTDataDecl& node, ZVarType const& type)
	: Datum(scope, type),
	  node(node),
	  globalId((scope.isGlobal() || scope.isScript())
	           ? optional<int>(ScriptParser::getUniqueGlobalID())
	           : nullopt)
{
	node.manager = this;
}

// ZScript::BuiltinVariable

BuiltinVariable* BuiltinVariable::create(
		Scope& scope, ZVarType const& type, string const& name,
		CompileErrorHandler& errorHandler)
{
	BuiltinVariable* builtin = new BuiltinVariable(scope, type, name);
	if (builtin->tryAddToScope(errorHandler)) return builtin;
	delete builtin;
	return NULL;
}

BuiltinVariable::BuiltinVariable(
		Scope& scope, ZVarType const& type, string const& name)
	: Datum(scope, type),
	  name(name),
	  globalId((scope.isGlobal() || scope.isScript())
	           ? optional<int>(ScriptParser::getUniqueGlobalID())
	           : nullopt)
{}

// ZScript::Constant

Constant* Constant::create(
		Scope& scope, ASTDataDecl& node, ZVarType const& type, long value,
		CompileErrorHandler& errorHandler)
{
	Constant* constant = new Constant(scope, node, type, value);
	if (constant->tryAddToScope(errorHandler)) return constant;
	delete constant;
	return NULL;
}

Constant::Constant(
		Scope& scope, ASTDataDecl& node, ZVarType const& type, long value)
	: Datum(scope, type), node(node), value(value)
{
	node.manager = this;
}

optional<string> Constant::getName() const {return node.name;}

// ZScript::BuiltinConstant


BuiltinConstant* BuiltinConstant::create(
		Scope& scope, ZVarType const& type, string const& name, long value,
		CompileErrorHandler& errorHandler)
{
	BuiltinConstant* builtin = new BuiltinConstant(scope, type, name, value);
	if (builtin->tryAddToScope(errorHandler)) return builtin;
	delete builtin;
	return NULL;
}

BuiltinConstant::BuiltinConstant(
		Scope& scope, ZVarType const& type, string const& name, long value)
	: Datum(scope, type), name(name), value(value)
{}

// ZScript::Function::Signature

Function::Signature::Signature(
		string const& name, vector<ZVarType const*> const& parameterTypes)
	: name(name), parameterTypes(parameterTypes)
{}

Function::Signature::Signature(Function const& function)
	: name(function.name), parameterTypes(function.paramTypes)
{}
		
int Function::Signature::compare(Function::Signature const& other) const
{
	int c = name.compare(other.name);
	if (c) return c;
	c = parameterTypes.size() - other.parameterTypes.size();
	if (c) return c;
	for (int i = 0; i < (int)parameterTypes.size(); ++i)
	{
		c = parameterTypes[i]->compare(*other.parameterTypes[i]);
		if (c) return c;
	}
	return 0;
}

bool Function::Signature::operator==(Function::Signature const& other) const
{
	return compare(other) == 0;
}

bool Function::Signature::operator<(Function::Signature const& other) const
{
	return compare(other) < 0;
}

string Function::Signature::asString() const
{
	string result;
	result += name;
	result += "(";
	bool comma = false;
	for (vector<ZVarType const*>::const_iterator it = parameterTypes.begin();
		 it != parameterTypes.end(); ++it)
	{
		if (comma) {result += ", "; comma = true;}
		result += (*it)->getName();
	}
	result += ")";
	return result;
}

// ZScript::Function

Function::Function(ZVarType const* returnType, string const& name,
				   vector<ZVarType const*> paramTypes, int id)
	: node(NULL), internalScope(NULL), thisVar(NULL),
	  returnType(returnType), name(name), paramTypes(paramTypes),
	  id(id), label(nullopt)
{}

Script* Function::getScript() const
{
	if (!internalScope) return NULL;
	Scope* parentScope = internalScope->getParent();
	if (!parentScope) return NULL;
	if (!parentScope->isScript()) return NULL;
	ScriptScope* scriptScope =
		dynamic_cast<ScriptScope*>(parentScope);
	return &scriptScope->script;
}

int Function::getLabel() const
{
	if (!label) label = ScriptParser::getUniqueLabelID();
	return *label;
}

bool ZScript::isRun(Function const& function)
{
	return function.internalScope->getParent()->isScript()
		&& *function.returnType == ZVarType::ZVOID
		&& function.name == "run";
}

int ZScript::getStackSize(Function const& function)
{
	return *lookupStackSize(*function.internalScope);
}

int ZScript::getParameterCount(Function const& function)
{
	return function.paramTypes.size() + (isRun(function) ? 1 : 0);
}
