#ifndef GLOBALSYMBOLS_H
#define GLOBALSYMBOLS_H

#include "DataStructs.h"
#include <string>
#include <map>
#include <vector>

using std::string;
using std::map;
using std::vector;

class SymbolTable;
namespace ZScript
{
	class Function;
	class Scope;
}

static const int SETTER = 0;
static const int GETTER = 1;
static const int FUNCTION = 2;

struct AccessorTable
{
    string name;
    int rettype;
    int setorget;
    int var;
    int numindex;
    int params[20];
};

class LibrarySymbols
{
public:
	static LibrarySymbols* getTypeInstance(ZVarTypeId typeId);

    virtual map<int, vector<Opcode*> > generateCode();
	
	virtual void addSymbolsToScope(ZScript::Scope& scope);
    virtual ~LibrarySymbols();
protected:
    AccessorTable *table;
    LibrarySymbols() {}
    int firstid;
    int refVar;
    map<string, ZScript::Function*> functions;
	map<string, ZScript::Function*> getters;
	map<string, ZScript::Function*> setters;

	ZScript::Function* getFunction(string const& name) const;
};

class GlobalSymbols : public LibrarySymbols
{
public:
    static GlobalSymbols &getInst()
    {
        return singleton;
    }

	map<int, vector<Opcode*> > generateCode();
private:
    static GlobalSymbols singleton;
    GlobalSymbols();
};

class FFCSymbols : public LibrarySymbols
{
public:
    static FFCSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > generateCode();
private:
    static FFCSymbols singleton;
    FFCSymbols();
};

class LinkSymbols : public LibrarySymbols
{
public:
    static LinkSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > generateCode();
private:
    static LinkSymbols singleton;
    LinkSymbols();
};

class ScreenSymbols : public LibrarySymbols
{
public:
    static ScreenSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > generateCode();
private:
    static ScreenSymbols singleton;
    ScreenSymbols();
};

class ItemSymbols : public LibrarySymbols
{
public:
    static ItemSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > generateCode();
protected:
private:
    static ItemSymbols singleton;
    ItemSymbols();
};

class ItemclassSymbols : public LibrarySymbols
{
public:
    static ItemclassSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > generateCode();
private:
    static ItemclassSymbols singleton;
    ItemclassSymbols();
};

class GameSymbols : public LibrarySymbols
{
public:
    static GameSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > generateCode();
private:
    static GameSymbols singleton;
    GameSymbols();
};

class NPCSymbols : public LibrarySymbols
{
public:
    static NPCSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > generateCode();
protected:
private:
    static NPCSymbols singleton;
    NPCSymbols();
};

class LinkWeaponSymbols : public LibrarySymbols
{
public:
    static LinkWeaponSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > generateCode();
protected:
private:
    static LinkWeaponSymbols singleton;
    LinkWeaponSymbols();
};

class EnemyWeaponSymbols : public LibrarySymbols
{
public:
    static EnemyWeaponSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > generateCode();
protected:
private:
    static EnemyWeaponSymbols singleton;
    EnemyWeaponSymbols();
};


class AudioSymbols : public LibrarySymbols
{
public:
    static AudioSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > generateCode();
protected:
private:
    static AudioSymbols singleton;
    AudioSymbols();
};

class DebugSymbols : public LibrarySymbols
{
public:
    static DebugSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > generateCode();
protected:
private:
    static DebugSymbols singleton;
    DebugSymbols();
};

class NPCDataSymbols : public LibrarySymbols
{
public:
    static NPCDataSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > generateCode();
protected:
private:
    static NPCDataSymbols singleton;
    NPCDataSymbols();
};

#endif

