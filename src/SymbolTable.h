#include "Registers.h"

#include <unordered_map>
#include <vector>
#include <utility>
#include <string>
#include <iostream>

#pragma once

enum class SymbolType {
    PARAM,
    VAR,
    CONST
};

enum class TableType {
    GLOBAL,
    FUNC,
    BLOCK
};

enum class MetaDataType {
    VOID,
    INT,
    FLOAT
};

class AbstractSymbol {
private:
    std::string symbolName;
    SymbolType symbolType;
    MetaDataType metaDataType;
    bool isArray;
    std::vector<std::size_t> shape;
    int offset;
    uint64_t memPosition{};

protected:

public:
    AbstractSymbol();
    AbstractSymbol(std::string inSymbolName, SymbolType inSymbolType, MetaDataType inMetaDataType, bool inIsArray, std::vector<std::size_t> inShape);

    virtual std::string getSymbolName() const;
    virtual SymbolType getSymbolType() const;
    virtual MetaDataType getMetaDataType() const;
    virtual bool getIsArray() const;
    virtual std::vector<std::size_t> getShape() const;
    virtual int getOffset() const;
    virtual uint64_t getMemPosition() const;

    virtual bool setAttributes(std::string inSymbolName, SymbolType inSymbolType, MetaDataType inMetaDataType, bool inIsArray, std::vector<std::size_t> inShape);
    virtual bool setOffset(int inOffset);
    virtual bool setMemPosition(uint64_t inMemPosition);

    static int getOffsetFromDataType(MetaDataType inDataType);
};

class ParamSymbol : public AbstractSymbol {
private:

protected:

public:
    ParamSymbol(const std::string& inSymbolName, MetaDataType inMetaDataType);

};

class VarSymbol : public AbstractSymbol {
private:

protected:

public:
    VarSymbol(const std::string& inSymbolName, MetaDataType inMetaDataType);

};

class ConstSymbol : public AbstractSymbol {
private:

protected:

public:
    ConstSymbol(const std::string& inSymbolName, MetaDataType inMetaDataType);
};


class ParamArraySymbol : public AbstractSymbol {
private:

protected:

public:
    ParamArraySymbol(const std::string& inSymbolName, MetaDataType inMetaDataType, std::vector<std::size_t> inShape);

};

class VarArraySymbol : public AbstractSymbol {
private:

protected:

public:
    VarArraySymbol(const std::string& inSymbolName, MetaDataType inMetaDataType, std::vector<std::size_t> inShape);

};

class ConstArraySymbol : public AbstractSymbol {
private:

protected:

public:
    ConstArraySymbol(const std::string& inSymbolName, MetaDataType inMetaDataType, std::vector<std::size_t> inShape);
};

class SymbolFactory {
private:

protected:

public:
    static AbstractSymbol *createSymbol(const std::string& inSymbolName, SymbolType inSymbolType, MetaDataType inMetaDataType, bool inIsArray, std::vector<std::size_t> inShape);

};

// --------- SymbolTables ----------

class FuncSymbolTableList;
class BlockSymbolTableList;
class SymbolTable;
class GlobalSymbolTable;
class FuncSymbolTable;
class BlockSymbolTable;

class FuncSymbolTableList {
private:

protected:
    std::unordered_map<std::string, SymbolTable *> funcSymbolTableList;

public:
    FuncSymbolTableList();
    virtual SymbolTable *insertFuncSymbolTableSafely(std::string inFuncName, MetaDataType inReturnType);
    virtual SymbolTable *insertFuncSymbolTableSafely(std::string inFuncName, MetaDataType inReturnType, SymbolTable *inParentSymbolTable);
    virtual SymbolTable *insertFuncSymbolTableSafely(SymbolTable *inFuncSymbolTable);

    virtual SymbolTable *lookUpFuncSymbolTable(std::string inFuncName) const;

};

class BlockSymbolTableList {
private:

protected:
    std::vector<SymbolTable *> blockSymbolTableList;

public:
    BlockSymbolTableList();
    virtual SymbolTable *insertBlockSymbolTable();
    //virtual SymbolTable *insertBlockSymbolTable(SymbolTable *inParentSymbolTable);
    virtual SymbolTable *insertBlockSymbolTable(SymbolTable *inBlockSymbolTable);
    
    virtual SymbolTable *getBlockSymbolTable(int index) const;

};

class SymbolTable : public FuncSymbolTableList, public BlockSymbolTableList {
private:
    SymbolTable *parentSymbolTable;
    TableType symbolTableType;

protected:
    std::unordered_map<std::string, AbstractSymbol *> abstractSymbolList;

public:
    SymbolTable();  // note: should promiss GlobalSymbolTable construct for once
    explicit SymbolTable(TableType inTableType);
    SymbolTable(SymbolTable *inParentSymbolTable);
    SymbolTable(TableType inTableType, SymbolTable *inParentSymbolTable);

    virtual AbstractSymbol *insertAbstractSymbolSafely(std::string inSymbolName, SymbolType inSymbolType, MetaDataType inMetaDataType, bool inIsArray, std::vector<std::size_t> inShape);
    virtual AbstractSymbol *insertAbstractSymbolSafely(AbstractSymbol *inAbstractSymbol);
    virtual AbstractSymbol *insertParamSymbolSafely(const std::string &inSymbolName,  MetaDataType inMetaDataType, bool inIsArray, std::vector<std::size_t> inShape) { return nullptr;};
    virtual AbstractSymbol *insertParamSymbolSafely(AbstractSymbol *inParamSymbol) { return nullptr;};
    virtual bool insertParamType(MetaDataType inMetaDataType, bool inIsArray, std::vector<std::size_t> inShape) { return false; };
    virtual bool insertParamType(AbstractSymbol *inParamSymbol) { return false; };
    virtual bool insertBindRegisters(Register *inRegister) { return false; };

    virtual AbstractSymbol *lookUpAbstractSymbol(std::string inSymbolName) const;
    virtual AbstractSymbol *lookUpAbstractSymbolGlobal(std::string inSymbolName) const;
    SymbolTable *lookUpFuncSymbolTable(std::string inFuncName) const override;
    virtual AbstractSymbol *lookUpParamSymbol(const std::string &inSymbolName) const { return nullptr; };
    virtual std::tuple <MetaDataType, bool, std::vector<std::size_t>> lookUpParamDataType(const std::string &inSymbolName) const { return {}; };

    virtual TableType getSymbolTableType() const;
    virtual SymbolTable *getParentSymbolTable() const;
    static SymbolTable *getGlobalSymbolTable();
    virtual std::string getFuncName() const { return {}; };
    virtual MetaDataType getReturnType() const { return MetaDataType::VOID; };
    virtual std::vector<std::tuple <MetaDataType, bool, std::vector<std::size_t>> > getParamDataTypeList() const { return {}; };
    virtual int getParamNum() const { return 0; };
    virtual int getFrameSize() const { return 0; };
    virtual std::vector<Register *> getBindRegisters() const { return {}; };
    
    virtual bool setSymbolTableType(TableType inSymbolTableType);
    virtual bool setParentSymbolTable(SymbolTable *parentSymbolTable);
    virtual bool setFuncName(const std::string &inFuncname) { return false;};
    virtual bool setReturnType(MetaDataType inReturnType) { return false;};
    virtual int setParamNum() { return 0; };
    virtual bool setParamDataTypeList() { return false; };
    virtual bool setFrameSize(int inSize) { return false; }

    virtual bool compareAbstractSymbolDataType(std::string inSymbolName, SymbolType inSymbolType, MetaDataType inMetaDataType, bool inIsArray, std::vector<std::size_t> inShape) const;
    virtual bool compareParamSymbolDataType(int index, MetaDataType inMetaDataType, bool inIsArray, std::vector<std::size_t> inShape) const { return false; };

};

// parent help to implement singleton pattern
class GlobalSymbolTable : public SymbolTable {
private:

protected:

public:
    GlobalSymbolTable(GlobalSymbolTable &globalSymbolTable) = delete;
    GlobalSymbolTable &operator = (const GlobalSymbolTable &globalSymbolTable) = delete;
    GlobalSymbolTable();

};

class FuncSymbolTable : public SymbolTable {
private:
    std::string funcName;
    MetaDataType returnType;
    int paramNum{};
    std::unordered_map<std::string, AbstractSymbol *> paramSymbolList;
    std::vector<std::tuple <MetaDataType, bool, std::vector<std::size_t>> > paramDataTypeList;
    int frameSize;

    std::vector<Register *> bindRegisters;

protected:

public:
    FuncSymbolTable();
    FuncSymbolTable(std::string inFuncName, MetaDataType inReturnType);
    FuncSymbolTable(std::string inFuncName, MetaDataType inReturnType, SymbolTable *inParentSymbolTable);
    AbstractSymbol *insertAbstractSymbolSafely(std::string inSymbolName, SymbolType inSymbolType, MetaDataType inMetaDataType, bool inIsArray, std::vector<std::size_t> inShape) override;
    AbstractSymbol *insertAbstractSymbolSafely(AbstractSymbol *inAbstractSymbol) override;
    AbstractSymbol *insertParamSymbolSafely(const std::string &inSymbolName, MetaDataType inMetaDataType, bool inIsArray, std::vector<std::size_t> inShape) override;
    AbstractSymbol *insertParamSymbolSafely(AbstractSymbol *inParamSymbol) override;
    bool insertParamType(MetaDataType inMetaDataType, bool inIsArray, std::vector<std::size_t> inShape) override;
    bool insertParamType(AbstractSymbol *inParamSymbol) override;
    bool insertBindRegisters(Register *inRegister) override { bindRegisters.push_back(inRegister); return true;};

    AbstractSymbol *lookUpParamSymbol(const std::string &inSymbolName) const override;
    std::tuple <MetaDataType, bool, std::vector<std::size_t>> lookUpParamDataType(const std::string &inSymbolName) const override;

    std::string getFuncName() const override;
    MetaDataType getReturnType() const override;
    int getParamNum() const override;
    std::vector<std::tuple <MetaDataType, bool, std::vector<std::size_t>>> getParamDataTypeList() const override;
    int getFrameSize() const override;
    std::vector<Register *> getBindRegisters() const override { return bindRegisters; };

    bool setFuncName(const std::string &inFuncName) override;
    bool setReturnType(MetaDataType inReturnType) override;
    int setParamNum() override;
    bool setParamDataTypeList() override;
    bool setFrameSize(int inSize) override;

    bool compareParamSymbolDataType(int index, MetaDataType inMetaDataType, bool inIsArray, std::vector<std::size_t> inShape) const override;

};

class BlockSymbolTable : public SymbolTable {
private:

protected:

public:
    BlockSymbolTable();
    explicit BlockSymbolTable(SymbolTable *inParentSymbolTable);

};

class SymbolTableFactory {
private:

protected:

public:
    static SymbolTable *createSymbolTable(TableType inTableType);

};
