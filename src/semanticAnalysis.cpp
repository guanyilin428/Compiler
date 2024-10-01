#include "semanticAnalysis.h"
#include "tools.h"
#include <math.h>

void SemanticAnalysis::enterCompUnit(SysYParser::CompUnitContext * ctx)
{
    block = 0;
    curSymbolTable = SymbolTable::getGlobalSymbolTable();
    irGenerator = IRGenerator::getIRGenerator(IRProgram::getIRProgram(programName, curSymbolTable));
    SymbolTable *funcSymbolTable = curSymbolTable->insertFuncSymbolTableSafely("putch", MetaDataType::VOID, curSymbolTable);
    funcSymbolTable->insertParamSymbolSafely("", MetaDataType::INT, false, {});
    funcSymbolTable->setParamDataTypeList();
    funcSymbolTable->setParamNum();
    irGenerator->enterFunction(funcSymbolTable);
    irGenerator->exitFunction();

    funcSymbolTable = curSymbolTable->insertFuncSymbolTableSafely("putint", MetaDataType::VOID, curSymbolTable);
    funcSymbolTable->insertParamSymbolSafely("", MetaDataType::INT, false, {});
    funcSymbolTable->setParamDataTypeList();
    funcSymbolTable->setParamNum();
    irGenerator->enterFunction(funcSymbolTable);
    irGenerator->exitFunction();

    funcSymbolTable = curSymbolTable->insertFuncSymbolTableSafely("putfloat", MetaDataType::VOID, curSymbolTable);
    funcSymbolTable->insertParamSymbolSafely("", MetaDataType::FLOAT, false, {});
    funcSymbolTable->setParamDataTypeList();
    funcSymbolTable->setParamNum();
    irGenerator->enterFunction(funcSymbolTable);
    irGenerator->exitFunction();

    funcSymbolTable = curSymbolTable->insertFuncSymbolTableSafely("putarray", MetaDataType::VOID, curSymbolTable);
    funcSymbolTable->insertParamSymbolSafely("", MetaDataType::INT, true, {});
    funcSymbolTable->setParamDataTypeList();
    funcSymbolTable->setParamNum();
    irGenerator->enterFunction(funcSymbolTable);
    irGenerator->exitFunction();

    funcSymbolTable = curSymbolTable->insertFuncSymbolTableSafely("putfarray", MetaDataType::VOID, curSymbolTable);
    funcSymbolTable->insertParamSymbolSafely("", MetaDataType::FLOAT, true, {});
    funcSymbolTable->setParamDataTypeList();
    funcSymbolTable->setParamNum();
    irGenerator->enterFunction(funcSymbolTable);
    irGenerator->exitFunction();

    funcSymbolTable = curSymbolTable->insertFuncSymbolTableSafely("getch", MetaDataType::INT, curSymbolTable);
    irGenerator->enterFunction(funcSymbolTable);
    irGenerator->exitFunction();

    funcSymbolTable = curSymbolTable->insertFuncSymbolTableSafely("getint", MetaDataType::INT, curSymbolTable);
    irGenerator->enterFunction(funcSymbolTable);
    irGenerator->exitFunction();

    funcSymbolTable = curSymbolTable->insertFuncSymbolTableSafely("getfloat", MetaDataType::FLOAT, curSymbolTable);
    irGenerator->enterFunction(funcSymbolTable);
    irGenerator->exitFunction();

    funcSymbolTable = curSymbolTable->insertFuncSymbolTableSafely("getarray", MetaDataType::INT, curSymbolTable);
    funcSymbolTable->setParamDataTypeList();
    funcSymbolTable->setParamNum();
    irGenerator->enterFunction(funcSymbolTable);
    irGenerator->exitFunction();

    funcSymbolTable = curSymbolTable->insertFuncSymbolTableSafely("getfarray", MetaDataType::FLOAT, curSymbolTable);
    funcSymbolTable->setParamDataTypeList();
    funcSymbolTable->setParamNum();
    irGenerator->enterFunction(funcSymbolTable);
    irGenerator->exitFunction();
}

void SemanticAnalysis::exitCompUnit(SysYParser::CompUnitContext * ctx)
{
    if(curSymbolTable->getSymbolTableType() != TableType::GLOBAL || !curSymbolTable->lookUpFuncSymbolTable("main")) {
        throw std::runtime_error("[ERROR] > There is no main function.\n");
    }
    irGenerator->ir->targetGen(irGenerator->targetCodes, optimizationLevel);
    irGenerator->ir->print();
    irGenerator->ir->write(programName + ".ir");
    IRProgram::targetCodePrint(irGenerator->targetCodes);
    IRProgram::targetCodeWrite(irGenerator->targetCodes, programName + ".s");
}

void SemanticAnalysis::enterDecl(SysYParser::DeclContext * ctx)
{
}

void SemanticAnalysis::exitDecl(SysYParser::DeclContext * ctx)
{
}

void SemanticAnalysis::enterConstDecl(SysYParser::ConstDeclContext * ctx)
{
}

void SemanticAnalysis::exitConstDecl(SysYParser::ConstDeclContext * ctx)
{
    MetaDataType type = ctx->bType()->bMetaDataType;
    SymbolFactory symbolFactory;

    for(const auto & const_def : ctx->constDef())
    {
        // TODO: type conversion
        if (const_def->withType && const_def->type != type && const_def->type != MetaDataType::VOID) {
            const_def->value->setMetaDataType(type);
            std::vector<std::string> value = const_def->value->getValues();
            if (type == MetaDataType::INT) {
                for (auto &val: value) {
                    val = std::to_string(std::stoi(val, nullptr, 0));
                }
            }
            else {
                for (auto &val: value) {
                    val = std::to_string(std::stof(val));
                }
            }
            const_def->value->setValues(value);
        }
        AbstractSymbol *symbol = SymbolFactory::createSymbol(const_def->symbolName, SymbolType::CONST, type, const_def->isArray, const_def->shape);
        if (!curSymbolTable->insertAbstractSymbolSafely(symbol)) {
            throw std::runtime_error("[ERROR] > Redefine const symbol.\n");
        }

        if(curSymbolTable->getSymbolTableType() == TableType::GLOBAL){
            IRSymbolVariable* newConst = irGenerator->addGlobalVariable(symbol, const_def->value);
        } else {
            IRSymbolVariable* newConst = irGenerator->addSymbolVariable(block, symbol, const_def->value);
            if(const_def->value){
                IRCode* code = nullptr;
                const_def->value->setMetaDataType(type);
                if (const_def->value->getOperandType() != OperandType::VALUE){
                    switch (type)
                    {
                        case MetaDataType::INT:
                            code = new IRAssignI(newConst, const_def->value);
                            newConst->setInitialValue(new IRValue(MetaDataType::INT, "0", {}, false));
                            break;
                        case MetaDataType::FLOAT:
                            code = new IRAssignF(newConst, const_def->value);
                            newConst->setInitialValue(new IRValue(MetaDataType::FLOAT, "0", {}, false));
                            break;
                        default:
                            break;
                    }
                }
                else {
                    switch (type) {
                        case MetaDataType::INT:
                            code = new IRAssignI(newConst, const_def->value);
                            newConst->setInitialValue(const_def->value);
                            break;
                        case MetaDataType::FLOAT:
                            code = new IRAssignF(newConst, const_def->value);
                            newConst->setInitialValue(const_def->value);
                            break;
                        default:
                            break;
                    }
                }
                irGenerator->addCode(code);
            }
        }
    }
}

void SemanticAnalysis::enterBType(SysYParser::BTypeContext * ctx)
{
    ctx->bMetaDataType = MetaDataType::VOID;
}

void SemanticAnalysis::exitBType(SysYParser::BTypeContext * ctx)
{
    std::string dataTypeText = ctx->getText();
    if (dataTypeText == "int") {
        ctx->bMetaDataType = MetaDataType::INT;
    }
    else if (dataTypeText == "float") {
        ctx->bMetaDataType = MetaDataType::FLOAT;
    }
    else{
        throw std::runtime_error("[ERROR] > Data Type not supported.\n");
    }
}

// ConstDef
void SemanticAnalysis::enterConstDef(SysYParser::ConstDefContext * ctx)
{
    ctx->symbolName = "";
    ctx->type = MetaDataType::VOID;
    ctx->withType = false;
    ctx->shape = {};
    ctx->isArray = false;
    ctx->value = nullptr;
    ctx->constInitVal()->outside = true;
    if (!ctx->exp().empty()) {
        for (auto val : ctx->exp()) {
            val->fromVarDecl = true;
            val->commVal = new Comm();
            ctx->commVal.push_back(val->commVal);
            if (ctx->constInitVal()) {
                ctx->constInitVal()->commVal.push_back(val->commVal);
            }
        }
    }
    else {
        if (ctx->constInitVal()) {
            ctx->constInitVal()->commVal.clear();
        }
    }
}

void SemanticAnalysis::exitConstDef(SysYParser::ConstDefContext * ctx)
{
    ctx->symbolName = ctx->Ident()->getText();
    if(ctx->exp().empty()){
        ctx->isArray = false;
    } else {
        ctx->isArray = true;
        for (auto it : ctx->commVal) {
            ctx->shape.push_back(it->shareValue);
        }
    }
    if (ctx->constInitVal()) {
        ctx->withType = true;
        ctx->type = ctx->constInitVal()->type;
        ctx->value = ctx->constInitVal()->value;
    }
    else {
        if (ctx->isArray) {
            ctx->value = irGenerator->ir->addMulSameImmValue(MetaDataType::INT, "0", ctx->shape);
        }
        else {
            ctx->value = new IRValue(MetaDataType::INT, "0", {}, false);
        }
    }
}

// ConstInitVal
void SemanticAnalysis::enterConstInitValOfVar(SysYParser::ConstInitValOfVarContext * ctx)
{
    if (ctx->outside && !ctx->commVal.empty()) {
        throw std::runtime_error("declare array but initialize with number");
    }
    ctx->type = MetaDataType::VOID;
    ctx->isArray = false;
    ctx->value = nullptr;
    if (ctx->outside && ctx->commVal.empty()) {
        ctx->exp()->fromVarDecl = false;
    }
    else {
        ctx->exp()->fromVarDecl = true;
    }
    ctx->exp()->commVal = nullptr;
    ctx->shape.clear();
}

void SemanticAnalysis::exitConstInitValOfVar(SysYParser::ConstInitValOfVarContext * ctx)
{
    ctx->type = ctx->exp()->metaDataType;
    ctx->isArray = false;
    if (ctx->outside && ctx->commVal.empty()) {
        if (ctx->exp()->metaDataType == MetaDataType::FLOAT) {
            ctx->value = irGenerator->addImmValue(ctx->exp()->metaDataType, std::to_string(ctx->exp()->sizeNum));
        } else {
            ctx->value = new IRValue(ctx->exp()->metaDataType, std::to_string(ctx->exp()->sizeNum), {}, false);
        }
    }
    else {
        ctx->vals.push_back(ctx->getText());
    }
    ctx->shape = {};
}

void SemanticAnalysis::enterConstInitValOfArray(SysYParser::ConstInitValOfArrayContext * ctx)
{
    ctx->type = MetaDataType::VOID;
    ctx->isArray = true;
    ctx->value = nullptr;
    for (auto it : ctx->commVal) {
        ctx->shape.push_back(it->shareValue);
    }
    for (auto it : ctx->constInitVal()) {
        it->outside = false;
        it->shape = std::vector<std::size_t>(ctx->shape.begin() + 1, ctx->shape.end());
        it->commVal.clear();
    }
}

void SemanticAnalysis::exitConstInitValOfArray(SysYParser::ConstInitValOfArrayContext * ctx)
{
    int totalSize = 1;
    for (auto it : ctx->shape) {
        totalSize *= it;
    }
    std::vector<std::size_t> cur(totalSize, 0);
    if(!ctx->constInitVal().empty()){
        ctx->type = ctx->constInitVal(0)->type;
        for (auto it : ctx->constInitVal()) {
            if (it->isArray) {
                if (ctx->vals.size() % (totalSize / ctx->shape[0])) {
                    ctx->vals.insert(ctx->vals.end(), ctx->vals.size() % (totalSize / ctx->shape[0]), "0");
                }
                ctx->vals.insert(ctx->vals.end(), it->vals.begin(), it->vals.end());
            }
            else {
                ctx->vals.push_back(it->vals[0]);
            }
        }
        while (ctx->vals.size() != totalSize) {
            ctx->vals.push_back("0");
        }
    } else {
        ctx->type = MetaDataType::INT;
        ctx->vals.insert(ctx->vals.end(), totalSize, "0");
    }

    ctx->isArray = true;

    if (ctx->outside) {
        if (ctx->isArray) {
            ctx->value = irGenerator->ir->addMulImmValue(ctx->type, ctx->vals);
        } else {
            ctx->value = new IRValue(MetaDataType::INT, "0", {}, false);
        }
    }
}

// VarDecl
void SemanticAnalysis::enterVarDecl(SysYParser::VarDeclContext * ctx)
{
}
void SemanticAnalysis::exitVarDecl(SysYParser::VarDeclContext * ctx)
{
    MetaDataType type = ctx->bType()->bMetaDataType;
    SymbolFactory symbolFactory;

    for(const auto & var_def : ctx->varDef())
    {
        if (var_def->withType && var_def->type != type && var_def->type != MetaDataType::VOID) {
            // TODO: type conversion
        }
        AbstractSymbol *symbol = SymbolFactory::createSymbol(var_def->symbolName, SymbolType::VAR, type, var_def->isArray, var_def->shape);
        if (!curSymbolTable->insertAbstractSymbolSafely(symbol)) {
            throw std::runtime_error("[ERROR] > Redefine var symbol. " + symbol->getSymbolName());
        }

        if(curSymbolTable->getSymbolTableType() == TableType::GLOBAL){
            IRSymbolVariable* newVar = irGenerator->addGlobalVariable(symbol, var_def->value);
        } else {
            IRSymbolVariable* newVar = irGenerator->addSymbolVariable(block, symbol, var_def->value);
            newVar->setAssigned();
            IRCode* code = nullptr;
            var_def->value->setMetaDataType(type);
            if(var_def->value){
                if (var_def->value->getOperandType() != OperandType::VALUE) {
                    switch (type) {
                        case MetaDataType::INT:
                            code = new IRAssignI(newVar, var_def->value);
                            newVar->setInitialValue(new IRValue(MetaDataType::INT, "0", {}, false));
                            break;
                        case MetaDataType::FLOAT:
                            code = new IRAssignF(newVar, var_def->value);
                            newVar->setInitialValue(new IRValue(MetaDataType::FLOAT, "0", {}, false));
                            break;
                        default:
                            break;
                    }
                }
                else {
                    switch (type) {
                        case MetaDataType::INT:
                            code = new IRAssignI(newVar, var_def->value);
                            newVar->setInitialValue(var_def->value);
                            break;
                        case MetaDataType::FLOAT:
                            code = new IRAssignF(newVar, var_def->value);
                            newVar->setInitialValue(var_def->value);
                            break;
                        default:
                            break;
                    }
                }
            }
            irGenerator->addCode(code);
        }
    }
}

void SemanticAnalysis::enterVarDef(SysYParser::VarDefContext * ctx)
{
    ctx->symbolName = "";
    ctx->type = MetaDataType::VOID;
    ctx->withType = false;
    ctx->shape = {};
    ctx->isArray = false;
    ctx->value = nullptr;
    if (ctx->initVal()) {
        ctx->initVal()->outside = true;
        ctx->initVal()->shape.clear();
    }
    if (ctx->exp().size() > 0) {
        for (auto val : ctx->exp()) {
            val->commVal = new Comm();
            val->fromVarDecl = true;
            ctx->commVal.push_back(val->commVal);
            if (ctx->initVal()) {
                ctx->initVal()->commVal.push_back(val->commVal);
            }
        }
    }
    else {
        if (ctx->initVal()) {
            ctx->initVal()->commVal.clear();
        }
    }
}

void SemanticAnalysis::exitVarDef(SysYParser::VarDefContext * ctx)
{
    ctx->symbolName = ctx->Ident()->getText();
    if(ctx->exp().empty()){
        ctx->isArray = false;
    } else {
        ctx->isArray = true;
        for (auto it : ctx->commVal) {
            ctx->shape.push_back(it->shareValue);
        }
    }
    if (ctx->initVal()) {
        ctx->withType = true;
        ctx->type = ctx->initVal()->type;
        ctx->value = ctx->initVal()->value;
    }
    else {
        if (ctx->isArray) {
            ctx->value = irGenerator->ir->addMulSameImmValue(MetaDataType::INT, "0", ctx->shape);
        }
        else {
            ctx->value = new IRValue(MetaDataType::INT, "0", {}, false);
        }
    }
}

void SemanticAnalysis::enterInitValOfVar(SysYParser::InitValOfVarContext *ctx) {
    if (ctx->outside && !ctx->commVal.empty()) {
        throw std::runtime_error("declare array but initialize with number");
    }
    ctx->type = MetaDataType::VOID;
    ctx->isArray = false;
    ctx->value = nullptr;
    ctx->vals.clear();
    if (ctx->outside && ctx->commVal.empty()) {
        ctx->exp()->fromVarDecl = false;
    }
    else {
        ctx->exp()->fromVarDecl = true;
    }
    ctx->exp()->commVal = nullptr;
}

void SemanticAnalysis::exitInitValOfVar(SysYParser::InitValOfVarContext *ctx) {
    ctx->type = ctx->exp()->metaDataType;
    ctx->isArray = false;
    if (ctx->outside && ctx->shape.empty()) {
        ctx->value = ctx->exp()->operand;
    }
    else {
        ctx->vals.push_back(std::to_string(ctx->exp()->sizeNum));
    }
    ctx->shape = {};
}

void SemanticAnalysis::enterInitValOfArray(SysYParser::InitValOfArrayContext *ctx) {
    ctx->type = MetaDataType::VOID;
    ctx->isArray = true;
    ctx->value = nullptr;
    for (auto it : ctx->commVal) {
        ctx->shape.push_back(it->shareValue);
    }
    for (auto it : ctx->initVal()) {
        it->outside = false;
        it->shape = std::vector<std::size_t>(ctx->shape.begin() + 1, ctx->shape.end());
    }
}

void SemanticAnalysis::exitInitValOfArray(SysYParser::InitValOfArrayContext *ctx) {
    int totalSize = 1;
    for (auto it : ctx->shape) {
        totalSize *= it;
    }
    std::vector<std::size_t> cur(totalSize, 0);
    if(!ctx->initVal().empty()){
        ctx->type = ctx->initVal(0)->type;
        for (auto it : ctx->initVal()) {
            if (it->isArray) {
                if (ctx->vals.size() % (totalSize / ctx->shape[0])) {
                    ctx->vals.insert(ctx->vals.end(), ctx->vals.size() % (totalSize / ctx->shape[0]), "0");
                }
                ctx->vals.insert(ctx->vals.end(), it->vals.begin(), it->vals.end());
            }
            else {
                ctx->vals.push_back(it->vals[0]);
            }
        }
        while (ctx->vals.size() != totalSize) {
            ctx->vals.push_back("0");
        }
    } else {
        ctx->type = MetaDataType::INT;
        ctx->vals.insert(ctx->vals.end(), totalSize, "0");
    }

    ctx->isArray = true;

    if (ctx->outside) {
        if (ctx->isArray) {
            ctx->value = irGenerator->ir->addMulImmValue(ctx->type, ctx->vals);
        } else {
            ctx->value = new IRValue(MetaDataType::INT, "0", {}, false);
        }
    }
}

void SemanticAnalysis::enterFuncDef(SysYParser::FuncDefContext * ctx)
{
    if (curSymbolTable->getSymbolTableType() != TableType::GLOBAL) {
        throw std::runtime_error("[ERROR] > cannot define function in non-global area.\n");
    }

    std::string datatype = ctx->funcType()->getText();
    MetaDataType returnType;
    if (datatype == "void") {
        returnType = MetaDataType::VOID;
    }
    else if (datatype == "int") {
        returnType = MetaDataType::INT;
    }
    else if (datatype == "float") {
        returnType = MetaDataType::FLOAT;
    }
    else{
        throw std::runtime_error("[ERROR] > Data Type not supported.\n");
    }

    std::string funcName = ctx->Ident()->getText();

    std::cout << funcName << std::endl;

    if (funcName == "main") {
        if (returnType != MetaDataType::INT || ctx->funcFParams()) {
            throw std::runtime_error("[ERROR] > wrong definition of main function");
        }
    }

    SymbolTable *funcSymbolTable = new FuncSymbolTable(funcName, returnType);
    ctx->funcBlock()->returnType = returnType;
    if (!curSymbolTable->insertFuncSymbolTableSafely(funcSymbolTable)) {
        throw std::runtime_error("[ERROR] > Redefine function of same name.\n");
    }
    funcSymbolTable->setParentSymbolTable(curSymbolTable);
    curSymbolTable = funcSymbolTable;
    irGenerator->enterFunction(funcSymbolTable);
}

void SemanticAnalysis::exitFuncDef(SysYParser::FuncDefContext * ctx)
{
    if (!ctx->funcBlock()->hasReturn && (irGenerator->currentIRFunc->getFuncSymbolTable()->getReturnType() != MetaDataType::VOID)) {
        throw std::runtime_error("[ERROR] > Non void function has no return.\n");
    }
    if (ctx->funcType()->funcMetaDataType == MetaDataType::VOID && ctx->funcBlock()->funcBlockItem().back()->getText().find("return", 0) != std::string::npos) {
        irGenerator->exitFunction();
    }
}

void SemanticAnalysis::enterFuncType(SysYParser::FuncTypeContext * ctx)
{
}

void SemanticAnalysis::exitFuncType(SysYParser::FuncTypeContext * ctx)
{
}

void SemanticAnalysis::enterFuncFParams(SysYParser::FuncFParamsContext * ctx)
{
}

void SemanticAnalysis::exitFuncFParams(SysYParser::FuncFParamsContext * ctx)
{
    curSymbolTable->setParamNum();
    curSymbolTable->setParamDataTypeList();
    int gr = 1, fr = 0;
    for(auto & param : ctx->funcFParam()){
        IRValue* g_index = nullptr;
        IRValue* f_index = nullptr;
        // add ParamVariable
        irGenerator->currentIRFunc->addParamVariable(param->symbolVar);

        // add getParam code
        IRCode* code = nullptr;
        if(param->isArray){
            g_index = new IRValue(MetaDataType::INT, std::to_string(gr), {}, false);
            code = new IRGetParamA(param->symbolVar, g_index);
            gr++;
        } else {
            switch (param->paramType) {
                case MetaDataType::INT:
                    g_index = new IRValue(MetaDataType::INT, std::to_string(gr), {}, false);
                    code = new IRGetParamI(param->symbolVar, g_index);
                    gr++;
                    break;
                case MetaDataType::FLOAT:
                    f_index = new IRValue(MetaDataType::INT, std::to_string(fr), {}, false);
                    code = new IRGetParamF(param->symbolVar, f_index);
                    fr++;
                    break;
                default:
                    break;
            }
        }
        irGenerator->addCode(code);
    }
}

void SemanticAnalysis::enterFuncFParam(SysYParser::FuncFParamContext * ctx)
{
}
void SemanticAnalysis::exitFuncFParam(SysYParser::FuncFParamContext * ctx)
{
    SymbolFactory symbolFactory;
    AbstractSymbol *funcParamSymbol = nullptr;
    if (ctx->brackets()) {
        funcParamSymbol = SymbolFactory::createSymbol(ctx->Ident()->getText(), SymbolType::PARAM, ctx->bType()->bMetaDataType, true, ctx->brackets()->shape);
    }
    else {
        funcParamSymbol = SymbolFactory::createSymbol(ctx->Ident()->getText(), SymbolType::PARAM, ctx->bType()->bMetaDataType, false, {});
    }
    if (!curSymbolTable->insertParamSymbolSafely(funcParamSymbol)) {
        throw std::runtime_error("[ERROR] > Redefine Function ParamSymbol.\n");
    }

    auto *newParam = new IRSymbolVariable(funcParamSymbol, nullptr, false);
    ctx->symbolVar = newParam;
    irGenerator->currentIRFunc->addParamVariable(newParam);
    ctx->isArray = ctx->brackets() != nullptr;
    ctx->paramType = ctx->bType()->bMetaDataType;
}

void SemanticAnalysis::enterBrackets(SysYParser::BracketsContext * ctx) {}

void SemanticAnalysis::exitBrackets(SysYParser::BracketsContext * ctx) {
    for (auto it : ctx->exp()) {
        ctx->shape.push_back(stoi(it->operand->getValue(), nullptr, 0));
        it->commVal = nullptr;
    }
}

void SemanticAnalysis::enterFuncBlock(SysYParser::FuncBlockContext * ctx)
{
    ctx->hasReturn = false;
    if (curSymbolTable->getSymbolTableType() != TableType::FUNC) {
        SymbolTable *blkSymbolTable = new BlockSymbolTable(curSymbolTable);
        curSymbolTable->insertBlockSymbolTable(blkSymbolTable);
        curSymbolTable = blkSymbolTable;
    }
    if (ctx->docLVal) {
        for(auto it : ctx->funcBlockItem()) {
            it->docLVal = true;
        }
    }
    for(auto it : ctx->funcBlockItem()) {
        it->returnType = ctx->returnType;
    }
}

void SemanticAnalysis::exitFuncBlock(SysYParser::FuncBlockContext * ctx)
{
    curSymbolTable = curSymbolTable->getParentSymbolTable();
    for (auto & it : ctx->funcBlockItem()) {
        if (it->hasReturn) {
            ctx->hasReturn = true;
        }
    }
    std::unordered_map<IROperand *, std::vector<IROperand *>> toJoinVarDoc;
    if (ctx->docLVal) {
        for(auto it : ctx->funcBlockItem()) {
            for (auto in_it : it->lValDoc) {
                if (toJoinVarDoc.find(in_it.first) != toJoinVarDoc.end()) {
                    for (auto operand : in_it.second) {
                        toJoinVarDoc[in_it.first].push_back(operand);
                    }
                }
                else {
                    toJoinVarDoc[in_it.first] = std::vector<IROperand *>(in_it.second.begin(), in_it.second.end());
                }
            }
        }
        ctx->lValDoc = toJoinVarDoc;
    }
}

void SemanticAnalysis::enterFuncBlockItem(SysYParser::FuncBlockItemContext * ctx)
{
    ctx->hasReturn = false;
    if (ctx->stmt() && ctx->docLVal) {
        ctx->stmt()->docLVal = true;
        ctx->stmt()->lValDoc.clear();
    }
    if (ctx->stmt()) {
        ctx->stmt()->returnType = ctx->returnType;
    }
}

void SemanticAnalysis::exitFuncBlockItem(SysYParser::FuncBlockItemContext * ctx)
{
    if (ctx->stmt() && ctx->stmt()->hasReturn) {
        ctx->hasReturn = true;
    }
    if (ctx->stmt() && ctx->docLVal) {
        ctx->lValDoc = ctx->stmt()->lValDoc;
    }
}

void SemanticAnalysis::enterBlock(SysYParser::BlockContext * ctx)
{
    ctx->hasReturn = false;
    ctx->returnType = MetaDataType::VOID;
    SymbolTable *blkSymbolTable = new BlockSymbolTable(curSymbolTable);
    curSymbolTable->insertBlockSymbolTable(blkSymbolTable);
    curSymbolTable = blkSymbolTable;

    block++;
    if (ctx->docLVal) {
        for(auto it : ctx->blockItem()) {
            it->docLVal = true;
        }
    }
    for(auto it : ctx->blockItem()) {
        it->returnType = ctx->returnType;
    }
}

void SemanticAnalysis::exitBlock(SysYParser::BlockContext * ctx)
{
    curSymbolTable = curSymbolTable->getParentSymbolTable();
    for (auto & it : ctx->blockItem()) {
        ctx->hasReturn = it->hasReturn;
    }

    block--;
    std::unordered_map<IROperand *, std::vector<IROperand *>> toJoinVarDoc;
    if (ctx->docLVal) {
        for(auto it : ctx->blockItem()) {
            for (auto in_it : it->lValDoc) {
                if (toJoinVarDoc.find(in_it.first) != toJoinVarDoc.end()) {
                    for (auto operand : in_it.second) {
                        toJoinVarDoc[in_it.first].push_back(operand);
                    }
                }
                else {
                    toJoinVarDoc[in_it.first] = std::vector<IROperand *>(in_it.second.begin(), in_it.second.end());
                }
            }
        }
        ctx->lValDoc = toJoinVarDoc;
    }
}

void SemanticAnalysis::enterBlockItem(SysYParser::BlockItemContext * ctx)
{
    ctx->hasReturn = false;
    ctx->returnType = MetaDataType::VOID;
    if (ctx->subStmt() && ctx->docLVal) {
        ctx->subStmt()->docLVal = true;
        ctx->subStmt()->lValDoc.clear();
    }
    if (ctx->subStmt()) {
        ctx->subStmt()->returnType = ctx->returnType;
    }
}

void SemanticAnalysis::exitBlockItem(SysYParser::BlockItemContext * ctx)
{
    if (ctx->subStmt() && ctx->subStmt()->hasReturn) {
        ctx->hasReturn = true;
    }
    if (ctx->subStmt() && ctx->docLVal) {
        ctx->lValDoc = ctx->subStmt()->lValDoc;
    }
}

void SemanticAnalysis::enterStmtAssignment(SysYParser::StmtAssignmentContext * ctx)
{
    ctx->hasReturn = false;
    ctx->exp()->commVal = nullptr;
    ctx->exp()->fromVarDecl = false;
    ctx->lVal()->fromVarDecl = false;
}

void SemanticAnalysis::exitStmtAssignment(SysYParser::StmtAssignmentContext * ctx)
{
    if (ctx->lVal()->symbolType == SymbolType::CONST) {
        throw std::runtime_error("[ERROR] > cannot assign to a CONST statement.\n");
    }
    if (ctx->lVal()->lValMetaDataType != ctx->exp()->metaDataType) {
        throw std::runtime_error("[ERROR] > stmt type mismatch in assignment. " + std::to_string(static_cast<int>(ctx->lVal()->lValMetaDataType)) + std::to_string(static_cast<int>(ctx->exp()->metaDataType)));
    }

    if (ctx->lVal()->indexOperand) { // array[index] = value
        IRCode *assignCode = nullptr;
        switch (ctx->lVal()->lValMetaDataType) {
            case MetaDataType::INT:
                assignCode = new IRAssignArrayElemI(ctx->exp()->operand, ctx->lVal()->identOperand,
                                                    ctx->lVal()->indexOperand);
                break;
            case MetaDataType::FLOAT:
                assignCode = new IRAssignArrayElemF(ctx->exp()->operand, ctx->lVal()->identOperand,
                                                    ctx->lVal()->indexOperand);
                break;
            default:
                break;
        }
        irGenerator->addCode(assignCode);
    } else { // value_a = value_b
        // consider assigned attribute of IRSymbolVariable
        IROperand* operand = nullptr;
        if(ctx->lVal()->identOperand->getAssigned()) {
            IRTempVariable *temp = irGenerator->addTempVariable(ctx->lVal()->identOperand);
            ctx->lVal()->identOperand->addHistorySymbol(temp);
            operand = temp;
            if (ctx->docLVal) {
                if (ctx->lValDoc.find(ctx->lVal()->identOperand) != ctx->lValDoc.end()) {
                    ctx->lValDoc[ctx->lVal()->identOperand].push_back(temp);
                    if (std::find(ctx->lValDoc[ctx->lVal()->identOperand].begin(), ctx->lValDoc[ctx->lVal()->identOperand].end(), ctx->lVal()->identOperand) != ctx->lValDoc[ctx->lVal()->identOperand].end()) {
                        ctx->lValDoc[ctx->lVal()->identOperand].push_back(ctx->lVal()->identOperand);
                    }
                }
                else {
                    ctx->lValDoc[ctx->lVal()->identOperand] = std::vector<IROperand *>(1, temp);
                    ctx->lValDoc[ctx->lVal()->identOperand].push_back(ctx->lVal()->identOperand);
                }
            }
        } else {
            ctx->lVal()->identOperand->setAssigned();
            operand = ctx->lVal()->identOperand;
            if (ctx->docLVal) {
                if (ctx->lValDoc.find(ctx->lVal()->identOperand) != ctx->lValDoc.end()) {
                    ctx->lValDoc[ctx->lVal()->identOperand].push_back(ctx->lVal()->identOperand);
                }
                else {
                    ctx->lValDoc[ctx->lVal()->identOperand] = std::vector<IROperand *>(1, ctx->lVal()->identOperand);
                }
            }
        }
        IRCode *assignCode = nullptr;
        if (!ctx->lVal()->isArray) {
            switch (ctx->lVal()->lValMetaDataType) {
                case MetaDataType::INT:
                    assignCode = new IRAssignI(operand, ctx->exp()->operand);
                    break;
                case MetaDataType::FLOAT:
                    assignCode = new IRAssignF(operand, ctx->exp()->operand);
                    break;
                default:
                    break;
            }
        }
        else {
            switch (ctx->lVal()->lValMetaDataType) {
                case MetaDataType::INT:
                    assignCode = new IRAssignArrayElemI(operand, ctx->exp()->operand, ctx->lVal()->indexOperand);
                    break;
                case MetaDataType::FLOAT:
                    assignCode = new IRAssignArrayElemF(operand, ctx->exp()->operand, ctx->lVal()->indexOperand);
                    break;
                default:
                    break;
            }
        }
        irGenerator->addCode(assignCode);
    }

    if(!ctx->codes.empty())
        irGenerator->addCodes(ctx->codes);
}

void SemanticAnalysis::enterStmtExpression(SysYParser::StmtExpressionContext * ctx)
{
    ctx->hasReturn = false;
    if (ctx->exp()) {
        ctx->exp()->commVal = nullptr;
        ctx->exp()->fromVarDecl = false;
    }
}

void SemanticAnalysis::exitStmtExpression(SysYParser::StmtExpressionContext * ctx)
{
    if(!ctx->codes.empty())
        irGenerator->addCodes(ctx->codes);
}

void SemanticAnalysis::enterStmtBlock(SysYParser::StmtBlockContext * ctx)
{
    ctx->hasReturn = false;
    SymbolTable *blkSymbolTable = new BlockSymbolTable(curSymbolTable);
    curSymbolTable->insertBlockSymbolTable(blkSymbolTable);
    curSymbolTable = blkSymbolTable;
    if (ctx->docLVal) {
        ctx->funcBlock()->docLVal = true;
        ctx->funcBlock()->lValDoc.clear();
    }
    if (ctx->funcBlock()) {
        ctx->funcBlock()->returnType = ctx->returnType;
    }
}

void SemanticAnalysis::exitStmtBlock(SysYParser::StmtBlockContext * ctx)
{
    curSymbolTable = curSymbolTable->getParentSymbolTable();
    if (ctx->funcBlock()) {
        ctx->hasReturn = ctx->funcBlock()->hasReturn;
    }
    if(!ctx->codes.empty())
        irGenerator->addCodes(ctx->codes);

    if (ctx->docLVal) {
        ctx->lValDoc = ctx->funcBlock()->lValDoc;
    }
}

void SemanticAnalysis::enterStmtCtrlSeq(SysYParser::StmtCtrlSeqContext * ctx)
{
    ctx->hasReturn = false;

    IRLabel* falseLabel = irGenerator->addLabel();
    ctx->cond()->falseLabel = falseLabel;
    std::vector<IRCode *> codes;
    if(ctx->getText().rfind("if", 0) == 0){
        IRCode *code = new IRAddLabel(falseLabel);
        if (ctx->stmt().size() > 1) {
            IRLabel *exitLabel = irGenerator->addLabel();
            codes.push_back(new IRGoto(exitLabel));
            ctx->stmt(1)->codes = std::vector<IRCode *>(1, new IRAddLabel(exitLabel));
        }
        codes.push_back(code);
        ctx->stmt(0)->codes = codes;
    } else if (ctx->getText().rfind("while", 0) == 0){
        IRLabel* beginLabel = irGenerator->enterWhile();
        IRCode *code = new IRGoto(beginLabel);
        codes.push_back(code);
        code = new IRAddLabel(falseLabel);
        codes.push_back(code);
        ctx->subStmt()->codes = codes;
        whileFalse.push_back(falseLabel);
        whileBegin.push_back(beginLabel);
        ctx->beginWhileLabel = beginLabel;
        std::unordered_map<std::string, IRSymbolVariable *> localVars = irGenerator->currentIRFunc->getLocalVariables();
        std::unordered_map<std::string, IRSymbolVariable *> paramVars = irGenerator->currentIRFunc->getParamVariables();
        std::unordered_map<std::string, IRTempVariable *> tempVars = irGenerator->currentIRFunc->getTempVariables();
        for (auto it : localVars) {
            if (it.second->getLatestVersionSymbol() != it.second) {
                ctx->latestSymbolList.emplace(it.second, it.second->getLatestVersionSymbol());
            }
        }
        for (auto it : paramVars) {
            if (it.second->getLatestVersionSymbol() != it.second) {
                ctx->latestSymbolList.emplace(it.second, it.second->getLatestVersionSymbol());
            }
        }
        for (auto it : tempVars) {
            if (!it.second->getAliasToVar() && it.second->getLatestVersionSymbol() != it.second) {
                ctx->latestSymbolList.emplace(it.second, it.second->getLatestVersionSymbol());
            }
        }
    } else {
        throw std::runtime_error("[ERROR] > not if or while stmt\n");
    }

    for (auto it : ctx->stmt()) {
        it->docLVal = true;
        it->lValDoc.clear();
    }
    if (ctx->subStmt()) {
        ctx->subStmt()->docLVal = true;
        ctx->subStmt()->lValDoc.clear();
        ctx->subStmt()->returnType = ctx->returnType;
    }
    for (auto it : ctx->stmt()) {
        it->returnType = ctx->returnType;
    }
}

void SemanticAnalysis::exitStmtCtrlSeq(SysYParser::StmtCtrlSeqContext * ctx)
{
    for (auto & s : ctx->stmt()) {
        if (s->hasReturn) {
            ctx->hasReturn = true;
            break;
        }
    }
    if (ctx->subStmt() && ctx->subStmt()->hasReturn) {
        ctx->hasReturn = true;
    }

    if (ctx->getText().rfind("while", 0) == 0){
        whileBegin.pop_back();
        whileFalse.pop_back();
    }
    if(!ctx->codes.empty())
        irGenerator->addCodes(ctx->codes);

    std::unordered_map<IROperand *, std::vector<IROperand *>> lVal;
    lVal.clear();
    for (auto stmt : ctx->stmt()) {
        for (const auto& it: stmt->lValDoc) {
            if (lVal.find(it.first) != lVal.end()) {
                for (auto in_it : it.second) {
                    if (std::find(lVal[it.first].begin(), lVal[it.first].end(), in_it) == lVal[it.first].end()) {
                        lVal[it.first].push_back(in_it);
                    }
                }
            }
            else {
                lVal[it.first] = std::vector<IROperand *>(it.second.begin(), it.second.end());
            }
        }
    }

    for (auto it : lVal) {
        IRTempVariable *newTemp = irGenerator->addTempVariable(it.first->getMetaDataType());
        it.first->addHistorySymbol(newTemp);
        newTemp->setAliasToVar();
        newTemp->setParentVariable(it.first);
        irGenerator->addCode(new IRPhi(newTemp, it.second));

        if (ctx->docLVal) {
            if (ctx->lValDoc.find(it.first) != ctx->lValDoc.end()) {
                ctx->lValDoc[it.first].push_back(newTemp);
            }
            else {
                ctx->lValDoc[it.first] = std::vector<IROperand *>(1, newTemp);
            }
        }
    }

    if (ctx->subStmt()) {
        std::vector<IRCode *> codes = irGenerator->currentIRFunc->getCodes();
        int insertPoint = 0;
        for (int i = codes.size() - 1; i >= 0; i--) {
            if (codes[i]->getOperation() == IROperation::ADD_LABEL) {
                if (codes[i]->getArg1() == ctx->beginWhileLabel) {
                    insertPoint = i + 1;
                    break;
                }
            }
        }
        for (const auto& it: ctx->subStmt()->lValDoc) {
            IRTempVariable *newTemp = irGenerator->addTempVariable(it.first);
            it.first->addHistorySymbol(newTemp);
            irGenerator->addCode(new IRPhi(newTemp, it.second));
            lVal[it.first] = std::vector<IROperand *>(1, newTemp);

            IRTempVariable *newTempFront = irGenerator->addTempVariable(it.first);
            it.first->addHistorySymbol(newTempFront);
            std::vector<IROperand *> phiNodes(it.second.begin(), it.second.end());
            if (std::find(it.second.begin(), it.second.end(), it.second.front()->getParentVariable()) == it.second.end()) {
                if (ctx->latestSymbolList.find(it.first) != ctx->latestSymbolList.end()) {
                    phiNodes.push_back(ctx->latestSymbolList[it.first]);
                }
                else {
                    phiNodes.push_back(it.first);
                }
            }
            irGenerator->currentIRFunc->insertCode(new IRPhi(newTempFront, phiNodes), insertPoint);
            insertPoint++;
            codes = irGenerator->currentIRFunc->getCodes();
            for (int i = insertPoint; i < codes.size(); i++) {
                if (codes[i]->getOperation() != IROperation::PHI) {
                    if (ctx->latestSymbolList.find(it.first) != ctx->latestSymbolList.end()) {
                        IROperand *toReplace = ctx->latestSymbolList[it.first];
                        if (codes[i]->getArg1() == toReplace) {
                            codes[i]->setArg1(newTempFront);
                            irGenerator->currentIRFunc->replaceCode(codes[i], i);
                        }
                        if (codes[i]->getArg2() == toReplace) {
                            codes[i]->setArg2(it.first);
                            irGenerator->currentIRFunc->replaceCode(codes[i], i);
                        }
                    }
                    else {
                        if (codes[i]->getArg1() == it.first) {
                            codes[i]->setArg1(newTempFront);
                            irGenerator->currentIRFunc->replaceCode(codes[i], i);
                        }
                        if (codes[i]->getArg2() == it.first) {
                            codes[i]->setArg2(it.first);
                            irGenerator->currentIRFunc->replaceCode(codes[i], i);
                        }
                    }
                }
            }
        }

        if (ctx->docLVal) {
            ctx->lValDoc = lVal;
        }
    }
}

void SemanticAnalysis::enterStmtReturn(SysYParser::StmtReturnContext * ctx)
{
    if (ctx->exp()) {
        ctx->exp()->commVal = nullptr;
        ctx->exp()->fromVarDecl = false;
    }
}

void SemanticAnalysis::exitStmtReturn(SysYParser::StmtReturnContext * ctx)
{
    if (ctx->exp() && ctx->exp()->isArray) {
        throw std::runtime_error("[ERROR] > never return an array.\n");
    }
    if (curSymbolTable->getSymbolTableType() == TableType::FUNC) {
        if ((!ctx->exp() && curSymbolTable->getReturnType() != MetaDataType::VOID) || (ctx->exp() && ctx->exp()->metaDataType != curSymbolTable->getReturnType())) {
            throw std::runtime_error("[ERROR] > stmt return type mismatch." + std::to_string(static_cast<int>(curSymbolTable->getSymbolTableType())));
        }
    }
    MetaDataType expReturnType = ctx->exp() ? ctx->exp()->metaDataType : MetaDataType::VOID;
    if (ctx->returnType != expReturnType) {
        throw std::runtime_error("[ERROR] > wrong return type");
    }
    ctx->hasReturn = true;

    IRCode *code = nullptr;

    if(ctx->exp()){
        switch(ctx->exp()->metaDataType){
            case MetaDataType::INT:
                code = new IRReturnI(ctx->exp()->operand, new IRSymbolFunction(irGenerator->currentIRFunc->getFuncSymbolTable()));
                break;
            case MetaDataType::FLOAT:
                code = new IRReturnF(ctx->exp()->operand, new IRSymbolFunction(irGenerator->currentIRFunc->getFuncSymbolTable()));
                break;
            default:
                break;
        }
    } else {
        code = new IRReturnV(new IRSymbolFunction(irGenerator->currentIRFunc->getFuncSymbolTable()));
    }
    irGenerator->addCode(code);
    if(!ctx->codes.empty())
        irGenerator->addCodes(ctx->codes);
}

void SemanticAnalysis::enterSubStmtAssignment(SysYParser::SubStmtAssignmentContext * ctx)
{
    ctx->hasReturn = false;
    ctx->returnType = MetaDataType::VOID;
    ctx->exp()->commVal = nullptr;
    ctx->exp()->fromVarDecl = false;
    ctx->lVal()->fromVarDecl = false;
}

void SemanticAnalysis::exitSubStmtAssignment(SysYParser::SubStmtAssignmentContext * ctx)
{
    if (ctx->lVal()->symbolType == SymbolType::CONST) {
        throw std::runtime_error("[ERROR] > cannot assign to a CONST statement.\n");
    }
    if (ctx->lVal()->lValMetaDataType != ctx->exp()->metaDataType) {
        throw std::runtime_error("[ERROR] > substmt: type mismatch in assignment. " + std::to_string(static_cast<int>(ctx->lVal()->lValMetaDataType)) + std::to_string(static_cast<int>(ctx->exp()->metaDataType)));
    }

    if(ctx->lVal()->indexOperand){ // array[index] = value
        IRCode* assignCode = nullptr;
        switch (ctx->lVal()->lValMetaDataType) {
            case MetaDataType::INT:
                assignCode = new IRAssignArrayElemI(ctx->exp()->operand, ctx->lVal()->identOperand, ctx->lVal()->indexOperand);
                break;
            case MetaDataType::FLOAT:
                assignCode = new IRAssignArrayElemF(ctx->exp()->operand, ctx->lVal()->identOperand, ctx->lVal()->indexOperand);
                break;
            default:
                break;
        }
        irGenerator->addCode(assignCode);
    } else { // value_a = value_b
        // consider assigned attribute of IRSymbolVariable
        IROperand* operand = nullptr;
        if(ctx->lVal()->identOperand->getAssigned()) {
            IRTempVariable *temp = irGenerator->addTempVariable(ctx->lVal()->identOperand);
            ctx->lVal()->identOperand->addHistorySymbol(temp);
            operand = temp;
            if (ctx->docLVal) {
                if (ctx->lValDoc.find(ctx->lVal()->identOperand) != ctx->lValDoc.end()) {
                    ctx->lValDoc[ctx->lVal()->identOperand].push_back(temp);
                    if (std::find(ctx->lValDoc[ctx->lVal()->identOperand].begin(), ctx->lValDoc[ctx->lVal()->identOperand].end(), ctx->lVal()->identOperand) != ctx->lValDoc[ctx->lVal()->identOperand].end()) {
                        ctx->lValDoc[ctx->lVal()->identOperand].push_back(ctx->lVal()->identOperand);
                    }
                }
                else {
                    ctx->lValDoc[ctx->lVal()->identOperand] = std::vector<IROperand *>(1, temp);
                    ctx->lValDoc[ctx->lVal()->identOperand].push_back(ctx->lVal()->identOperand);
                }
            }
        } else {
            ctx->lVal()->identOperand->setAssigned();
            operand = ctx->lVal()->identOperand;
            if (ctx->docLVal) {
                if (ctx->lValDoc.find(ctx->lVal()->identOperand) != ctx->lValDoc.end()) {
                    ctx->lValDoc[ctx->lVal()->identOperand].push_back(ctx->lVal()->identOperand);
                }
                else {
                    ctx->lValDoc[ctx->lVal()->identOperand] = std::vector<IROperand *>(1, ctx->lVal()->identOperand);
                }
            }
        }
        IRCode *assignCode = nullptr;
        if (!ctx->lVal()->isArray) {
            switch (ctx->lVal()->lValMetaDataType) {
                case MetaDataType::INT:
                    assignCode = new IRAssignI(operand, ctx->exp()->operand);
                    break;
                case MetaDataType::FLOAT:
                    assignCode = new IRAssignF(operand, ctx->exp()->operand);
                    break;
                default:
                    break;
            }
        }
        else {
            switch (ctx->lVal()->lValMetaDataType) {
                case MetaDataType::INT:
                    assignCode = new IRAssignArrayElemI(operand, ctx->exp()->operand, ctx->lVal()->indexOperand);
                    break;
                case MetaDataType::FLOAT:
                    assignCode = new IRAssignArrayElemF(operand, ctx->exp()->operand, ctx->lVal()->indexOperand);
                    break;
                default:
                    break;
            }
        }
        irGenerator->addCode(assignCode);
    }

    if(!ctx->codes.empty())
        irGenerator->addCodes(ctx->codes);
}

void SemanticAnalysis::enterSubStmtExpression(SysYParser::SubStmtExpressionContext * ctx)
{
    ctx->hasReturn = false;
    ctx->returnType = MetaDataType::VOID;
    if (ctx->exp()) {
        ctx->exp()->commVal = nullptr;
        ctx->exp()->fromVarDecl = false;
    }
}

void SemanticAnalysis::exitSubStmtExpression(SysYParser::SubStmtExpressionContext * ctx)
{
    if(!ctx->codes.empty())
        irGenerator->addCodes(ctx->codes);
}

void SemanticAnalysis::enterSubStmtBlock(SysYParser::SubStmtBlockContext * ctx)
{
    SymbolTable *blkSymbolTable = new BlockSymbolTable(curSymbolTable);
    curSymbolTable->insertBlockSymbolTable(blkSymbolTable);
    curSymbolTable = blkSymbolTable;
    ctx->hasReturn = false;
    ctx->block()->returnType = ctx->returnType;
    if (ctx->docLVal) {
        ctx->block()->docLVal = true;
        ctx->block()->lValDoc.clear();
    }
}

void SemanticAnalysis::exitSubStmtBlock(SysYParser::SubStmtBlockContext * ctx)
{
    curSymbolTable = curSymbolTable->getParentSymbolTable();
    ctx->hasReturn = ctx->block()->hasReturn;
    if(!ctx->codes.empty())
        irGenerator->addCodes(ctx->codes);

    if (ctx->docLVal) {
        ctx->lValDoc = ctx->block()->lValDoc;
    }
}

void SemanticAnalysis::enterSubStmtCtrlSeq(SysYParser::SubStmtCtrlSeqContext * ctx)
{
    ctx->hasReturn = false;

    if (ctx->cond()) {
        IRLabel* falseLabel = irGenerator->addLabel();
        ctx->cond()->falseLabel = falseLabel;
        std::vector<IRCode *> codes;
        if(ctx->getText().rfind("if", 0) == 0){
            IRCode *code = new IRAddLabel(falseLabel);
            if (ctx->subStmt().size() > 1) {
                IRLabel *exitLabel = irGenerator->addLabel();
                codes.push_back(new IRGoto(exitLabel));
                ctx->subStmt(1)->codes = std::vector<IRCode *>(1, new IRAddLabel(exitLabel));
            }
            codes.push_back(code);
            ctx->subStmt(0)->codes = codes;
        } else if (ctx->getText().rfind("while", 0) == 0){
            IRLabel* beginLabel = irGenerator->enterWhile();
            IRCode *code = new IRGoto(beginLabel);
            codes.push_back(code);
            code = new IRAddLabel(falseLabel);
            codes.push_back(code);
            ctx->subStmt(0)->codes = codes;
            whileFalse.push_back(falseLabel);
            whileBegin.push_back(beginLabel);
            ctx->beginWhileLabel = beginLabel;
            std::unordered_map<std::string, IRSymbolVariable *> localVars = irGenerator->currentIRFunc->getLocalVariables();
            std::unordered_map<std::string, IRSymbolVariable *> paramVars = irGenerator->currentIRFunc->getParamVariables();
            std::unordered_map<std::string, IRTempVariable *> tempVars = irGenerator->currentIRFunc->getTempVariables();
            for (auto it : localVars) {
                if (it.second->getLatestVersionSymbol() != it.second) {
                    ctx->latestSymbolList.emplace(it.second, it.second->getLatestVersionSymbol());
                }
            }
            for (auto it : paramVars) {
                if (it.second->getLatestVersionSymbol() != it.second) {
                    ctx->latestSymbolList.emplace(it.second, it.second->getLatestVersionSymbol());
                }
            }
            for (auto it : tempVars) {
                if (!it.second->getAliasToVar() && it.second->getLatestVersionSymbol() != it.second) {
                    ctx->latestSymbolList.emplace(it.second, it.second->getLatestVersionSymbol());
                }
            }
        }
    }

    for (auto it : ctx->subStmt()) {
        it->docLVal = true;
        it->lValDoc.clear();
        it->returnType = ctx->returnType;
    }
}

void SemanticAnalysis::exitSubStmtCtrlSeq(SysYParser::SubStmtCtrlSeqContext * ctx)
{
    for (auto & s : ctx->subStmt()) {
        if (s->hasReturn) {
            ctx->hasReturn = true;
        }
    }

    if(ctx->getText().rfind("break", 0) == 0){
        IRLabel* falseLabel = whileFalse.back();
        IRCode* code = new IRGoto(falseLabel);
        irGenerator->addCode(code);
    } else if(ctx->getText().rfind("continue", 0) == 0){
        IRLabel* beginLabel = whileBegin.back();
        IRCode* code = new IRGoto(beginLabel);
        irGenerator->addCode(code);
    } else if (ctx->getText().rfind("while", 0) == 0){
        whileFalse.pop_back();
        whileBegin.pop_back();
    }

    if(!ctx->codes.empty())
        irGenerator->addCodes(ctx->codes);

    std::unordered_map<IROperand *, std::vector<IROperand *>> lVal;
    lVal.clear();
    for (auto stmt : ctx->subStmt()) {
        for (const auto& it: stmt->lValDoc) {
            if (lVal.find(it.first) != lVal.end()) {
                for (auto in_it : it.second) {
                    if (std::find(lVal[it.first].begin(), lVal[it.first].end(), in_it) == lVal[it.first].end()) {
                        lVal[it.first].push_back(in_it);
                    }
                }
            }
            else {
                lVal[it.first] = std::vector<IROperand *>(it.second.begin(), it.second.end());
            }
        }
    }

    std::vector<IRCode *> codes = irGenerator->currentIRFunc->getCodes();
    int insertPoint = 0;
    if (ctx->getText().rfind("while", 0) == 0) {
        for (int i = codes.size() - 1; i > 0; i--) {
            if (codes[i]->getOperation() == IROperation::ADD_LABEL) {
                if (codes[i]->getArg1() == ctx->beginWhileLabel) {
                    insertPoint = i + 1;
                    break;
                }
            }
        }
    }

    for (auto it : lVal) {
        IRTempVariable *newTemp = irGenerator->addTempVariable(it.first->getMetaDataType());
        it.first->addHistorySymbol(newTemp);
        newTemp->setAliasToVar();
        newTemp->setParentVariable(it.first);
        irGenerator->addCode(new IRPhi(newTemp, it.second));

        if (ctx->getText().rfind("while", 0) == 0) {
            IRTempVariable *newTempFront = irGenerator->addTempVariable(it.first);
            it.first->addHistorySymbol(newTempFront);
            std::vector<IROperand *> phiNodes(it.second.begin(), it.second.end());
            if (std::find(it.second.begin(), it.second.end(), it.second.front()->getParentVariable()) == it.second.end()) {
                if (ctx->latestSymbolList.find(it.first) != ctx->latestSymbolList.end()) {
                    phiNodes.push_back(ctx->latestSymbolList[it.first]);
                }
                else {
                    phiNodes.push_back(it.first);
                }
            }
            irGenerator->currentIRFunc->insertCode(new IRPhi(newTempFront, phiNodes), insertPoint);
            insertPoint++;
            codes = irGenerator->currentIRFunc->getCodes();
            for (int i = insertPoint; i < codes.size(); i++) {
                if (codes[i]->getOperation() != IROperation::PHI) {
                    if (ctx->latestSymbolList.find(it.first) != ctx->latestSymbolList.end()) {
                        IROperand *toReplace = ctx->latestSymbolList[it.first];
                        if (codes[i]->getArg1() == toReplace) {
                            codes[i]->setArg1(newTempFront);
                            irGenerator->currentIRFunc->replaceCode(codes[i], i);
                        }
                        if (codes[i]->getArg2() == toReplace) {
                            codes[i]->setArg2(it.first);
                            irGenerator->currentIRFunc->replaceCode(codes[i], i);
                        }
                    }
                    else {
                        if (codes[i]->getArg1() == it.first) {
                            codes[i]->setArg1(newTempFront);
                            irGenerator->currentIRFunc->replaceCode(codes[i], i);
                        }
                        if (codes[i]->getArg2() == it.first) {
                            codes[i]->setArg2(it.first);
                            irGenerator->currentIRFunc->replaceCode(codes[i], i);
                        }
                    }
                }
            }
        }

        if (ctx->docLVal) {
            if (ctx->lValDoc.find(it.first) != ctx->lValDoc.end()) {
                ctx->lValDoc[it.first].push_back(newTemp);
            }
            else {
                ctx->lValDoc[it.first] = std::vector<IROperand *>(1, newTemp);
            }
        }
    }
}

void SemanticAnalysis::enterSubStmtReturn(SysYParser::SubStmtReturnContext * ctx)
{
    if (ctx->exp()) {
        ctx->exp()->commVal = nullptr;
        ctx->exp()->fromVarDecl = false;
    }
}

void SemanticAnalysis::exitSubStmtReturn(SysYParser::SubStmtReturnContext * ctx)
{
    if (ctx->exp() && ctx->exp()->isArray) {
        throw std::runtime_error("[ERROR] > never return an array.\n");
    }
    if (curSymbolTable->getSymbolTableType() == TableType::FUNC) {
        if ((!ctx->exp() && curSymbolTable->getReturnType() != MetaDataType::VOID) || ctx->exp()->metaDataType != curSymbolTable->getReturnType()) {
            throw std::runtime_error("[ERROR] > SubStmtReturn return type mismatch.  " + ctx->getText());
        }
    }
    else {
        ctx->hasReturn = true;
        MetaDataType expMetaDataType;
        if (ctx->exp()){
            expMetaDataType = ctx->exp()->metaDataType;
        }
        else {
            expMetaDataType = MetaDataType::VOID;
        }
        if (ctx->returnType != expMetaDataType) {
            throw std::runtime_error("[ERROR] > SubStmtReturn return type mismatch.\n");
        }
    }

    IRCode *code = nullptr;
    if(ctx->exp()){
        switch(ctx->returnType){
            case MetaDataType::INT:
                code = new IRReturnI(ctx->exp()->operand, new IRSymbolFunction(irGenerator->currentIRFunc->getFuncSymbolTable()));
                break;
            case MetaDataType::FLOAT:
                code = new IRReturnF(ctx->exp()->operand, new IRSymbolFunction(irGenerator->currentIRFunc->getFuncSymbolTable()));
                break;
            default:
                break;
        }
    } else {
        code = new IRReturnV(new IRSymbolFunction(irGenerator->currentIRFunc->getFuncSymbolTable()));
    }
    irGenerator->addCode(code);

    if(!ctx->codes.empty())
        irGenerator->addCodes(ctx->codes);
}

void SemanticAnalysis::enterExp(SysYParser::ExpContext *ctx) {
    ctx->isArray = false;
    ctx->shape = {};
    ctx->metaDataType = MetaDataType::VOID;
    ctx->addExp()->indexOperand = ctx->indexOperand;
    if (ctx->fromVarDecl) {
        ctx->addExp()->fromVarDecl = true;
    }
    else {
        ctx->addExp()->fromVarDecl = false;
    }
}

void SemanticAnalysis::exitExp(SysYParser::ExpContext *ctx) {
    ctx->isArray = ctx->addExp()->isArray;
    ctx->shape = ctx->addExp()->shape;
    ctx->metaDataType = ctx->addExp()->metaDataType;
    ctx->operand = ctx->addExp()->operand;
    ctx->sizeNum = ctx->addExp()->sizeNum;
    if (ctx->commVal) {
        ctx->commVal->shareValue = ctx->sizeNum;
    }
}

// Cond
void SemanticAnalysis::enterCond(SysYParser::CondContext * ctx)
{

}

void SemanticAnalysis::exitCond(SysYParser::CondContext * ctx)
{
    if(ctx->lOrExp()->metaDataType != MetaDataType::INT) {
        throw std::runtime_error("[ERROR] > condition must be bool");
    }
    IRCode* code = new IRBeqz(ctx->lOrExp()->operand,  ctx->falseLabel);
    irGenerator->addCode(code);
}

void SemanticAnalysis::enterLVal(SysYParser::LValContext * ctx)
{
    ctx->isArray = false;
    ctx->shape = {};
    ctx->symbolType = SymbolType::PARAM;
    ctx->lValMetaDataType = MetaDataType::VOID;
    ctx->indexOperand = nullptr;
    if (ctx->fromVarDecl) {
        for (auto it : ctx->exp()) {
            it->commVal = nullptr;
            it->fromVarDecl = true;
        }
    }
    else {
        for (auto it : ctx->exp()) {
            it->commVal = nullptr;
            it->fromVarDecl = false;
        }
    }
}

void SemanticAnalysis::exitLVal(SysYParser::LValContext * ctx)
{
    // TODO: calculate inside first, then use index to calculate mem position
    AbstractSymbol *searchLVal = curSymbolTable->lookUpAbstractSymbolGlobal(ctx->Ident()->getText());
    if (!ctx->exp().empty()) {
        if (ctx->exp(0)->metaDataType != MetaDataType::INT) {
            throw std::runtime_error("[ERROR] > array index must be int.\n");
        }
    }
    if (!searchLVal){
        throw std::runtime_error("[ERROR] > var symbol used before defined. " + ctx->Ident()->getText() + " "  + std::to_string(static_cast<int>(curSymbolTable->getSymbolTableType())));
    }
    if (!ctx->fromVarDecl) {
        if (searchLVal->getIsArray() && ctx->exp().empty()) {
            ctx->isArray = true;
            if (searchLVal->getSymbolType() != SymbolType::PARAM) {
                ctx->shape = std::move(searchLVal->getShape());
            }
        }
        else if (searchLVal->getIsArray()) {
            if (searchLVal->getSymbolType() != SymbolType::PARAM) {
                if (ctx->exp().size() < searchLVal->getShape().size()) {
                    ctx->isArray = true;
                    ctx->shape = std::vector<size_t>(searchLVal->getShape().begin() + ctx->exp().size(), searchLVal->getShape().end());
                }
                else if (ctx->exp().size() == searchLVal->getShape().size()) {
                    ctx->isArray = true;
                    ctx->shape = {};
                }
                else {
                    throw std::runtime_error("[ERROR] > array shape not match\n");
                }
            }
        }
        else {
            ctx->isArray = false;
        }
    }
    ctx->symbolType = searchLVal->getSymbolType();
    ctx->lValMetaDataType = searchLVal->getMetaDataType();

    // search lVal IRSymbolVariable
    IRSymbolVariable* symVar = nullptr;
    std::string varName = searchLVal->getSymbolName();
    symVar = irGenerator->currentIRFunc->getLocalVariable(block, varName);
    if(!symVar)
        symVar = irGenerator->currentIRFunc->getParamVariable(varName);
    if(!symVar)
        symVar = irGenerator->ir->getGlobalVariable(varName);
    ctx->identOperand = symVar;

    // assigned
    if (!ctx->fromVarDecl) {
        if(!ctx->exp().empty() && searchLVal->getSymbolType() != SymbolType::PARAM){
            std::vector<size_t> shape = symVar->getArrayShape();
            int width = shape.back();
            IRTempVariable *mulTemp = irGenerator->addTempVariable(MetaDataType::INT);
            IRTempVariable *addTemp = irGenerator->addTempVariable(MetaDataType::INT);
            irGenerator->addCode(new IRAssignI(mulTemp, ctx->exp().back()->operand));
            mulTemp->setAssigned();
            irGenerator->addCode(new IRAssignI(addTemp, ctx->exp().back()->operand));
            addTemp->setAssigned();
            for (int i = ctx->exp().size() - 2; i >= 0; i--) {
                IRTempVariable *replaceTemp = irGenerator->addTempVariable(mulTemp);
                mulTemp->addHistorySymbol(replaceTemp);
                irGenerator->addCode(new IRMulI(replaceTemp, ctx->exp(i)->operand,
                                                new IRValue(MetaDataType::INT, std::to_string(width), {}, false)));
                IRTempVariable *replaceAddTemp = irGenerator->addTempVariable(addTemp);
                irGenerator->addCode(new IRAddI(replaceAddTemp, mulTemp->getLatestVersionSymbol(), addTemp->getLatestVersionSymbol()));
                addTemp->addHistorySymbol(replaceAddTemp);
                width *= shape[i];
            }
            ctx->indexOperand = addTemp->getLatestVersionSymbol();
        }
    }
    
    if (symVar->getInitialValue()) {
        IROperand *idInitValue = symVar->getInitialValue();
        if (idInitValue->getIsArray()) {
            std::vector<std::size_t> arrShape = symVar->getArrayShape();
            int totalSize = 0, tmpWidth = 1;
            for (int i = ctx->exp().size() - 1; i >= 0; i--) {
                totalSize += tmpWidth * ctx->exp(i)->sizeNum;
                tmpWidth *= arrShape[i];
            }
            ctx->sizeNum = std::stoi(idInitValue->getValue(totalSize), nullptr, 0);
        }
        else {
            ctx->sizeNum = std::stoi(idInitValue->getValue(), nullptr, 0);
        }
    }
}


void SemanticAnalysis::enterPrimaryExpNestExp(SysYParser::PrimaryExpNestExpContext * ctx)
{
    ctx->isArray = false;
    ctx->shape = {};
    ctx->metaDataType = MetaDataType::VOID;
    ctx->exp()->indexOperand = ctx->indexOperand;
    ctx->exp()->commVal = nullptr;
    if (ctx->fromVarDecl) {
        ctx->exp()->fromVarDecl = true;
    }
    else {
        ctx->exp()->fromVarDecl = false;
    }
}

void SemanticAnalysis::exitPrimaryExpNestExp(SysYParser::PrimaryExpNestExpContext * ctx)
{
    ctx->isArray = ctx->exp()->isArray;
    ctx->shape = ctx->exp()->shape;
    ctx->metaDataType = ctx->exp()->metaDataType;
    ctx->operand = ctx->exp()->operand;
    ctx->sizeNum = ctx->exp()->sizeNum;
}

void SemanticAnalysis::enterPrimaryExplVal(SysYParser::PrimaryExplValContext * ctx)
{
    ctx->isArray = false;
    ctx->shape = {};
    ctx->metaDataType = MetaDataType::VOID;
    if (ctx->fromVarDecl) {
        ctx->lVal()->fromVarDecl = true;
    }
    else {
        ctx->lVal()->fromVarDecl = false;
    }
}

void SemanticAnalysis::exitPrimaryExplVal(SysYParser::PrimaryExplValContext * ctx)
{
    ctx->metaDataType = ctx->lVal()->lValMetaDataType;
    if (!ctx->fromVarDecl) {
        if (ctx->lVal()->shape.empty()) {
            ctx->isArray = false;
        }
        else {
            ctx->isArray = true;
        }
        ctx->shape = ctx->lVal()->shape;

        if (ctx->lVal()->isArray && ctx->lVal()->indexOperand) {
            IRTempVariable* tmp = irGenerator->addTempVariable(ctx->metaDataType);
            IRCode* fetchCode = nullptr;
            switch (ctx->lVal()->lValMetaDataType) {
                case MetaDataType::INT:
                    fetchCode = new IRFetchArrayElemI(tmp, ctx->lVal()->identOperand, ctx->lVal()->indexOperand);
                    break;
                case MetaDataType::FLOAT:
                    fetchCode = new IRFetchArrayElemF(tmp, ctx->lVal()->identOperand, ctx->lVal()->indexOperand);
                    break;
                default:
                    break;
            }
            irGenerator->addCode(fetchCode);
            ctx->operand = tmp;
        } else { // normal symbolVar
            ctx->operand = ctx->lVal()->identOperand->getLatestVersionSymbol();
        }
    }
    ctx->sizeNum = ctx->lVal()->sizeNum;
}

void SemanticAnalysis::enterPrimaryExpNumber(SysYParser::PrimaryExpNumberContext * ctx)
{
    ctx->isArray = false;
    ctx->shape = {};
    ctx->metaDataType = MetaDataType::VOID;
}

void SemanticAnalysis::exitPrimaryExpNumber(SysYParser::PrimaryExpNumberContext * ctx)
{
    ctx->metaDataType = ctx->number()->metaDataType;
    if (!ctx->fromVarDecl) {
        ctx->isArray = false;
        if (ctx->metaDataType == MetaDataType::FLOAT) {
            ctx->operand = irGenerator->addImmValue(ctx->metaDataType, ctx->getText());
        }
        else {
            if (ctx->metaDataType == MetaDataType::INT) {
                ctx->operand = new IRValue(ctx->metaDataType, ctx->getText(), {}, false);
            }
            else {
                ctx->operand = irGenerator->addImmValue(ctx->metaDataType, ctx->getText());
            }
        }
    }
    ctx->sizeNum = stoi(ctx->number()->getText(), nullptr, 0);
}


// Unary
void SemanticAnalysis::enterUnaryExpPrimaryExp(SysYParser::UnaryExpPrimaryExpContext * ctx)
{
    ctx->isArray = false;
    ctx->shape = {};
    ctx->metaDataType = MetaDataType::VOID;
    if (ctx->fromVarDecl) {
        ctx->primaryExp()->fromVarDecl = true;
    }
    else {
        ctx->primaryExp()->fromVarDecl = false;
    }
}

void SemanticAnalysis::exitUnaryExpPrimaryExp(SysYParser::UnaryExpPrimaryExpContext * ctx)
{
    ctx->isArray = ctx->primaryExp()->isArray;
    ctx->shape = ctx->primaryExp()->shape;
    ctx->metaDataType = ctx->primaryExp()->metaDataType;
    ctx->operand = ctx->primaryExp()->operand;
    ctx->sizeNum = ctx->primaryExp()->sizeNum;
}

void SemanticAnalysis::enterUnaryExpFunc(SysYParser::UnaryExpFuncContext * ctx)
{
    ctx->isArray = false;
    ctx->shape = {};
    ctx->metaDataType = MetaDataType::VOID;
}

void SemanticAnalysis::exitUnaryExpFunc(SysYParser::UnaryExpFuncContext * ctx)
{
    SymbolTable *funcSymbolTable = curSymbolTable->lookUpFuncSymbolTable(ctx->Ident()->getText());
    if (!funcSymbolTable) {
        throw std::runtime_error("[ERROR] > function called before definined. " + ctx->Ident()->getText());
    }
    if (ctx->funcRParams()) {
        if (ctx->funcRParams()->isArrayList.size() != funcSymbolTable->getParamNum()) {
            throw std::runtime_error("[ERROR] > in function calling, parameter number not match. " + std::to_string(ctx->funcRParams()->isArrayList.size()) + " " + std::to_string(funcSymbolTable->getParamNum()));
        }
        for (int i = 0; i < ctx->funcRParams()->isArrayList.size(); ++i) {
            if (!funcSymbolTable->compareParamSymbolDataType(i, ctx->funcRParams()->metaDataTypeList[i], ctx->funcRParams()->isArrayList[i], ctx->funcRParams()->shapeList[i])) {
                throw std::runtime_error("[ERROR] > calling function parameter type error.\n");
            }
        }
    }
    else {
        if (funcSymbolTable->getParamNum()) {
            throw std::runtime_error("[ERROR] > in function calling, parameter number not match. ");
        }
    }
    ctx->isArray = false;
    ctx->metaDataType = funcSymbolTable->getReturnType();

    IRSymbolFunction *func = irGenerator->getSymbolFunction(funcSymbolTable->getFuncName());
    IRSymbolFunction *selfFunc = irGenerator->ir->getSymbolFunction(irGenerator->currentIRFunc->getFunctionName());
    IRCode *code = new IRCall(func, selfFunc);
    irGenerator->addCode(code);

    if(funcSymbolTable->getReturnType() != MetaDataType::VOID){
        IRTempVariable *newResult = irGenerator->addTempVariable(funcSymbolTable->getReturnType());
        switch (funcSymbolTable->getReturnType())
        {
            case MetaDataType::INT:
                code = new IRGetReturnI(newResult);
                irGenerator->addCode(code);
                break;
            case MetaDataType::FLOAT:
                code = new IRGetReturnF(newResult);
                irGenerator->addCode(code);
                break;
            default:
                break;
        }
        ctx->operand = newResult;
    }
}

void SemanticAnalysis::enterUnaryExpNestUnaryExp(SysYParser::UnaryExpNestUnaryExpContext * ctx)
{
    ctx->isArray = false;
    ctx->shape = {};
    ctx->metaDataType = MetaDataType::VOID;
    ctx->unaryExp()->indexOperand = ctx->indexOperand;
    if (ctx->fromVarDecl) {
        ctx->unaryExp()->fromVarDecl = true;
    }
    else {
        ctx->unaryExp()->fromVarDecl = false;
    }
}

void SemanticAnalysis::exitUnaryExpNestUnaryExp(SysYParser::UnaryExpNestUnaryExpContext * ctx)
{
    ctx->metaDataType = ctx->unaryExp()->metaDataType;
    if (!ctx->fromVarDecl) {
        ctx->isArray = ctx->unaryExp()->isArray;
        ctx->shape = ctx->unaryExp()->shape;

        IRTempVariable* result = irGenerator->addTempVariable(ctx->metaDataType);
        IRCode* code = nullptr;
        if(ctx->unaryOp()->getText() == "-"){
            switch (ctx->unaryExp()->operand->getMetaDataType()) {
                case MetaDataType::INT:
                    code = new IRNegI(result, ctx->unaryExp()->operand);
                    break;
                case MetaDataType::FLOAT:
                    code = new IRNegF(result, ctx->unaryExp()->operand);
                    break;
                default:
                    break;
            }
        }
        else if(ctx->unaryOp()->getText() == "!"){
            code = new IRNot(result, ctx->unaryExp()->operand);
        }
        else {
            code = new IRAssignI(result, ctx->unaryExp()->operand);
        }
        irGenerator->addCode(code);
        ctx->operand = result;
    }
    
    if (ctx->unaryOp()->getText() == "+") {
        ctx->sizeNum = ctx->unaryExp()->sizeNum;
    }
    else if (ctx->unaryOp()->getText() == "-") {
        ctx->sizeNum = -ctx->unaryExp()->sizeNum;
    }
    else {
        ctx->sizeNum = !ctx->unaryExp()->sizeNum;
    }
}

void SemanticAnalysis::enterUnaryOp(SysYParser::UnaryOpContext * ctx)
{
}

void SemanticAnalysis::exitUnaryOp(SysYParser::UnaryOpContext * ctx)
{
}

// funcRParams
void SemanticAnalysis::enterFuncRParams(SysYParser::FuncRParamsContext * ctx)
{
    ctx->isArrayList.clear();
    ctx->shapeList.clear();
    ctx->metaDataTypeList.clear();
    for (auto it : ctx->exp()) {
        it->commVal = nullptr;
        it->fromVarDecl = false;
    }
}

void SemanticAnalysis::exitFuncRParams(SysYParser::FuncRParamsContext * ctx)
{
    int ireg = 1, freg = 0;
    for (auto & it : ctx->exp()) {
        ctx->isArrayList.emplace_back(it->isArray);
        ctx->shapeList.emplace_back(it->shape);
        ctx->metaDataTypeList.emplace_back(it->metaDataType);
        if (it->isArray) {
            irGenerator->addCode(new IRAddParamA(it->operand, new IRValue(MetaDataType::INT, std::to_string(ireg), "", false), irGenerator->ir->getSymbolFunction(irGenerator->currentIRFunc->getFunctionName()), ctx->exp().size()));
            ireg++;
        }
        else {
            switch (it->metaDataType)
            {
                case MetaDataType::INT:
                    irGenerator->addCode(new IRAddParamI(it->operand, new IRValue(MetaDataType::INT, std::to_string(ireg), "", false), irGenerator->ir->getSymbolFunction(irGenerator->currentIRFunc->getFunctionName()), ctx->exp().size()));
                    ireg++;
                    break;
                case MetaDataType::FLOAT:
                    irGenerator->addCode(new IRAddParamF(it->operand, new IRValue(MetaDataType::INT, std::to_string(ireg), "", false)));
                    freg++;
                    break;
                default:
                    throw std::runtime_error("[ERROR] > data type fault.\n");
                    break;
            }
        }
    }
}

// MulExp
void SemanticAnalysis::enterMulExpMulExp(SysYParser::MulExpMulExpContext * ctx)
{
    ctx->isArray = false;
    ctx->shape = {};
    ctx->metaDataType = MetaDataType::VOID;
    ctx->mulExp()->indexOperand = ctx->indexOperand;
    ctx->unaryExp()->indexOperand = ctx->indexOperand;
    if (ctx->fromVarDecl) {
        ctx->mulExp()->fromVarDecl = true;
        ctx->unaryExp()->fromVarDecl = true;
    }
    else {
        ctx->mulExp()->fromVarDecl = false;
        ctx->unaryExp()->fromVarDecl = false;
    }
}

void SemanticAnalysis::exitMulExpMulExp(SysYParser::MulExpMulExpContext * ctx)
{
    ctx->metaDataType = ctx->mulExp()->metaDataType;
    if (!ctx->fromVarDecl) {
        ctx->isArray = ctx->mulExp()->isArray;
        ctx->shape = ctx->mulExp()->shape;
        if (ctx->metaDataType != ctx->unaryExp()->metaDataType) {
            throw std::runtime_error("[ERROR] > mul: type mismatch in calculation. " + std::to_string(static_cast<int>(ctx->metaDataType)) + std::to_string(static_cast<int>(ctx->unaryExp()->metaDataType)));
        }
        if (ctx->isArray) {
            if (!ctx->unaryExp()->isArray) {
                throw std::runtime_error("[ERROR] > mul: array:non-array calculation.\n");
            }
            if (ctx->shape != ctx->mulExp()->shape) {
                throw std::runtime_error("[ERROR] > array size mismatch in calculation.\n");
            }
        }
        else {
            if (ctx->unaryExp()->isArray) {
                throw std::runtime_error("[ERROR] > mul: non-array:array calculation.\n");
            }
        }

    //    TODO: "divisor equals zero"
        if(ctx->mulOp()->getText() == "%"){
            if(ctx->metaDataType != MetaDataType::INT)
                throw std::runtime_error("[ERROR] > mod: operands not INT.\n");
            if(!ctx->unaryExp()->isArray){
                if(ctx->unaryExp()->operand->getValue() == "0")
                    throw std::runtime_error("[ERROR] > divisor is zero.\n");
            } else {
                for(auto &val : ctx->unaryExp()->operand->getValues()){
                    if(val == "0")
                        throw std::runtime_error("[ERROR] > divisor is zero.\n");
                }
            }
        } else if (ctx->mulOp()->getText() == "/") {
            if(!ctx->unaryExp()->isArray){
                if(ctx->unaryExp()->operand->getValue() == "0")
                    throw std::runtime_error("[ERROR] > divisor is zero.\n");
            } else {
                for(auto &val : ctx->unaryExp()->operand->getValues()){
                    if(val == "0")
                        throw std::runtime_error("[ERROR] > divisor is zero.\n");
                }
            }
        }

        IRTempVariable* result = irGenerator->addTempVariable(ctx->metaDataType);
        IRCode* code = nullptr;
        if (ctx->mulOp()->getText() == "*"){
            switch (ctx->mulExp()->operand->getMetaDataType()) {
                case MetaDataType::INT:
                    code = new IRMulI(result, ctx->mulExp()->operand, ctx->unaryExp()->operand);
                    break;
                case MetaDataType::FLOAT:
                    code = new IRMulF(result, ctx->mulExp()->operand, ctx->unaryExp()->operand);
                    break;
                default:
                    break;
            }
        }
        else if (ctx->mulOp()->getText() == "/"){
            switch (ctx->mulExp()->operand->getMetaDataType()) {
                case MetaDataType::INT:
                    code = new IRDivI(result, ctx->mulExp()->operand, ctx->unaryExp()->operand, irGenerator->ir->getSymbolFunction(irGenerator->currentIRFunc->getFunctionName()));
                    break;
                case MetaDataType::FLOAT:
                    code = new IRDivF(result, ctx->mulExp()->operand, ctx->unaryExp()->operand);
                    break;
                default:
                    break;
            }
        }
        else if (ctx->mulOp()->getText() == "%"){
            code = new IRMod(result, ctx->mulExp()->operand, ctx->unaryExp()->operand, irGenerator->ir->getSymbolFunction(curSymbolTable->getFuncName()));
        }
        else
            throw std::runtime_error("[ERROR] > mulop illegal.\n");
        irGenerator->addCode(code);
        ctx->operand = result;
    }

    if (ctx->mulOp()->getText() == "*") {
        ctx->sizeNum = ctx->mulExp()->sizeNum * ctx->unaryExp()->sizeNum;
    }
    else if (ctx->mulOp()->getText() == "/" && ctx->unaryExp()->sizeNum != 0) {
        ctx->sizeNum = ctx->mulExp()->sizeNum / ctx->unaryExp()->sizeNum;
    }
    else if (ctx->mulOp()->getText() == "%" && ctx->unaryExp()->sizeNum != 0) {
        ctx->sizeNum = ctx->mulExp()->sizeNum % ctx->unaryExp()->sizeNum;
    }
}

void SemanticAnalysis::enterMulExpUnaryExp(SysYParser::MulExpUnaryExpContext * ctx)
{
    ctx->isArray = false;
    ctx->shape = {};
    ctx->metaDataType = MetaDataType::VOID;
    if (ctx->fromVarDecl) {
        ctx->unaryExp()->fromVarDecl = true;
    }
    else {
        ctx->unaryExp()->fromVarDecl = false;
    }
}

void SemanticAnalysis::exitMulExpUnaryExp(SysYParser::MulExpUnaryExpContext * ctx)
{
    ctx->isArray = ctx->unaryExp()->isArray;
    ctx->metaDataType = ctx->unaryExp()->metaDataType;
    ctx->shape = ctx->unaryExp()->shape;
    ctx->operand = ctx->unaryExp()->operand;
    ctx->sizeNum = ctx->unaryExp()->sizeNum;
}

void SemanticAnalysis::enterMulOp(SysYParser::MulOpContext * ctx)
{
}

void SemanticAnalysis::exitMulOp(SysYParser::MulOpContext * ctx)
{
}

// AddExp
void SemanticAnalysis::enterAddExpAddExp(SysYParser::AddExpAddExpContext * ctx)
{
    ctx->isArray = false;
    ctx->shape = {};
    ctx->metaDataType = MetaDataType::VOID;
    ctx->addExp()->indexOperand = ctx->indexOperand;
    ctx->mulExp()->indexOperand = ctx->indexOperand;
    if (ctx->fromVarDecl) {
        ctx->mulExp()->fromVarDecl = true;
        ctx->addExp()->fromVarDecl = true;
    }
    else {
        ctx->mulExp()->fromVarDecl = false;
        ctx->addExp()->fromVarDecl = false;
    }
}

void SemanticAnalysis::exitAddExpAddExp(SysYParser::AddExpAddExpContext * ctx)
{
    ctx->metaDataType = ctx->addExp()->metaDataType;
    if (!ctx->fromVarDecl) {
        ctx->isArray = ctx->addExp()->isArray;
        ctx->shape = ctx->addExp()->shape;
        if (ctx->metaDataType != ctx->mulExp()->metaDataType) {
            throw std::runtime_error("[ERROR] > add: type mismatch in calculation. " + std::to_string(static_cast<int>(ctx->metaDataType)) + std::to_string(static_cast<int>(ctx->mulExp()->metaDataType)));
        }
        if (ctx->isArray) {
            if (!ctx->mulExp()->isArray) {
                throw std::runtime_error("[ERROR] > add: array:non-array calculation.\n");
            }
            if (ctx->shape != ctx->mulExp()->shape) {
                throw std::runtime_error("[ERROR] > array size mismatch in calculation.\n");
            }
        }
        else {
            if (ctx->mulExp()->isArray) {
                throw std::runtime_error("[ERROR] > add: non-array:array calculation. " + curSymbolTable->getFuncName() + " " + ctx->mulExp()->getText());
            }
        }

        IRTempVariable* result = irGenerator->addTempVariable(ctx->metaDataType);
        IRCode* code = nullptr;

        if (ctx->addOp()->getText() == "+"){
            switch (ctx->addExp()->metaDataType) {
                case MetaDataType::INT:
                    code = new IRAddI(result, ctx->addExp()->operand, ctx->mulExp()->operand);
                    break;
                case MetaDataType::FLOAT:
                    code = new IRAddF(result, ctx->addExp()->operand, ctx->mulExp()->operand);
                    break;
                default:
                    break;
            }
        }
        else if (ctx->addOp()->getText() == "-"){
            switch (ctx->addExp()->metaDataType) {
                case MetaDataType::INT:
                    code = new IRSubI(result, ctx->addExp()->operand, ctx->mulExp()->operand);
                    break;
                case MetaDataType::FLOAT:
                    code = new IRSubF(result, ctx->addExp()->operand, ctx->mulExp()->operand);
                    break;
                default:
                    break;
            }
        }
        else
            throw std::runtime_error("[ERROR] > addop illegal.\n");

        irGenerator->addCode(code);
        ctx->operand = result;
    }
    if (ctx->addOp()->getText() == "+") {
        ctx->sizeNum = ctx->addExp()->sizeNum + ctx->mulExp()->sizeNum;
    }
    else {
        ctx->sizeNum = ctx->addExp()->sizeNum - ctx->mulExp()->sizeNum;
    }
}

void SemanticAnalysis::enterAddExpMulExp(SysYParser::AddExpMulExpContext * ctx)
{
    ctx->isArray = false;
    ctx->shape = {};
    ctx->metaDataType = MetaDataType::VOID;
    ctx->mulExp()->indexOperand = ctx->indexOperand;
    if (ctx->fromVarDecl) {
        ctx->mulExp()->fromVarDecl = true;
    }
    else {
        ctx->mulExp()->fromVarDecl = false;
    }
}

void SemanticAnalysis::exitAddExpMulExp(SysYParser::AddExpMulExpContext * ctx)
{
    ctx->isArray = ctx->mulExp()->isArray;
    ctx->metaDataType = ctx->mulExp()->metaDataType;
    ctx->shape = ctx->mulExp()->shape;
    ctx->operand = ctx->mulExp()->operand;
    ctx->sizeNum = ctx->mulExp()->sizeNum;
}

void SemanticAnalysis::enterAddOp(SysYParser::AddOpContext *ctx) {}
void SemanticAnalysis::exitAddOp(SysYParser::AddOpContext *ctx) {}

// RelExp
void SemanticAnalysis::enterRelExpRelExp(SysYParser::RelExpRelExpContext * ctx)
{
    ctx->metaDataType = MetaDataType::VOID;
    ctx->addExp()->fromVarDecl = false;
}

void SemanticAnalysis::exitRelExpRelExp(SysYParser::RelExpRelExpContext * ctx)
{
    if (ctx->addExp()->isArray) {
        throw std::runtime_error("[ERROR] > rel: array cannot be operands of logic operators.\n");
    }
    ctx->metaDataType = ctx->relExp()->metaDataType;
    if (ctx->metaDataType != ctx->addExp()->metaDataType) {
        throw std::runtime_error("[ERROR] > rel: relation calculation with different types.\n");
    }
    ctx->metaDataType = MetaDataType::INT;

    IRTempVariable* result = irGenerator->addTempVariable(ctx->metaDataType);
    IRCode* code = nullptr;
    if (ctx->relOp()->getText() == "<"){
        switch (ctx->addExp()->metaDataType) {
            case MetaDataType::INT:
                code = new IRSltI(result, ctx->relExp()->operand, ctx->addExp()->operand);
                break;
            case MetaDataType::FLOAT:
                code = new IRSltF(result, ctx->relExp()->operand, ctx->addExp()->operand);
                break;
            default:
                break;
        }
    }
    else if (ctx->relOp()->getText() == ">"){
        switch (ctx->addExp()->metaDataType) {
            case MetaDataType::INT:
                code = new IRSgtI(result, ctx->relExp()->operand, ctx->addExp()->operand);
                break;
            case MetaDataType::FLOAT:
                code = new IRSgtF(result, ctx->relExp()->operand, ctx->addExp()->operand);
                break;
            default:
                break;
        }
    }
    else if (ctx->relOp()->getText() == "<="){
        switch (ctx->addExp()->metaDataType) {
            case MetaDataType::INT:
                code = new IRSleqI(result, ctx->relExp()->operand, ctx->addExp()->operand);
                break;
            case MetaDataType::FLOAT:
                code = new IRSleqF(result, ctx->relExp()->operand, ctx->addExp()->operand);
                break;
            default:
                break;
        }
    }
    else if (ctx->relOp()->getText() == ">="){
        switch (ctx->addExp()->metaDataType) {
            case MetaDataType::INT:
                code = new IRSgeqI(result, ctx->relExp()->operand, ctx->addExp()->operand);
                break;
            case MetaDataType::FLOAT:
                code = new IRSgeqF(result, ctx->relExp()->operand, ctx->addExp()->operand);
                break;
            default:
                break;
        }
    }
    else
        throw std::runtime_error("[ERROR] > addop illegal.\n");
    irGenerator->addCode(code);
    ctx->operand = result;
}

void SemanticAnalysis::enterRelExpAddExp(SysYParser::RelExpAddExpContext * ctx)
{
    ctx->metaDataType = MetaDataType::VOID;
    ctx->addExp()->fromVarDecl = false;
}

void SemanticAnalysis::exitRelExpAddExp(SysYParser::RelExpAddExpContext * ctx)
{
    if (ctx->addExp()->isArray) {
        throw std::runtime_error("[ERROR] > rel add: array cannot be operands of logic operators. " + curSymbolTable->getFuncName());
    }
    ctx->metaDataType = ctx->addExp()->metaDataType;
    ctx->operand = ctx->addExp()->operand;
}

void SemanticAnalysis::enterRelOp(SysYParser::RelOpContext *ctx) {}
void SemanticAnalysis::exitRelOp(SysYParser::RelOpContext *ctx) {}

//EqExp
void SemanticAnalysis::enterEqExpRelExp(SysYParser::EqExpRelExpContext * ctx)
{
    ctx->metaDataType = MetaDataType::VOID;

}
void SemanticAnalysis::exitEqExpRelExp(SysYParser::EqExpRelExpContext * ctx)
{
    ctx->metaDataType = ctx->relExp()->metaDataType;
    ctx->operand = ctx->relExp()->operand;
}

void SemanticAnalysis::enterEqExpEqExp(SysYParser::EqExpEqExpContext * ctx)
{
    ctx->metaDataType = MetaDataType::VOID;
}

void SemanticAnalysis::exitEqExpEqExp(SysYParser::EqExpEqExpContext * ctx)
{
    if (ctx->eqExp()->metaDataType != ctx->relExp()->metaDataType) {
        throw std::runtime_error("[ERROR] > eq operator with different data type.\n");
    }
    ctx->metaDataType = MetaDataType::INT;

    IRTempVariable* result = irGenerator->addTempVariable(ctx->metaDataType);
    IRCode* code = nullptr;
    if(ctx->eqOp()->getText() == "=="){
        switch (ctx->eqExp()->metaDataType) {
            case MetaDataType::INT:
                code = new IRSeqI(result, ctx->eqExp()->operand, ctx->relExp()->operand);
                break;
            case MetaDataType::FLOAT:
                code = new IRSeqF(result, ctx->eqExp()->operand, ctx->relExp()->operand);
                break;
            default:
                break;
        }
    } else if(ctx->eqOp()->getText() == "!=") {
        switch (ctx->eqExp()->metaDataType) {
            case MetaDataType::INT:
                code = new IRSneI(result, ctx->eqExp()->operand, ctx->relExp()->operand);
                break;
            case MetaDataType::FLOAT:
                code = new IRSneF(result, ctx->eqExp()->operand, ctx->relExp()->operand);
                break;
            default:
                break;
        }
    }
    irGenerator->addCode(code);
    ctx->operand = result;
}

void SemanticAnalysis::enterEqOp(SysYParser::EqOpContext *ctx) {}
void SemanticAnalysis::exitEqOp(SysYParser::EqOpContext *ctx) {}

//LAndExp
void SemanticAnalysis::enterLAndExpLAndExp(SysYParser::LAndExpLAndExpContext * ctx)
{
    ctx->metaDataType = MetaDataType::VOID;
}

void SemanticAnalysis::exitLAndExpLAndExp(SysYParser::LAndExpLAndExpContext * ctx)
{
    ctx->metaDataType = ctx->lAndExp()->metaDataType;
    if (ctx->metaDataType != MetaDataType::INT || ctx->eqExp()->metaDataType != MetaDataType::INT) {
        throw std::runtime_error("[ERROR] > logic calculation with non-boolean operands.\n");
    }
    IRTempVariable* result = irGenerator->addTempVariable(ctx->metaDataType);
    auto code = new IRAnd(result, ctx->lAndExp()->operand, ctx->eqExp()->operand);
    irGenerator->addCode(code);
    ctx->operand = result;
}

void SemanticAnalysis::enterLAndExpEqExp(SysYParser::LAndExpEqExpContext * ctx)
{
    ctx->metaDataType = MetaDataType::VOID;
}

void SemanticAnalysis::exitLAndExpEqExp(SysYParser::LAndExpEqExpContext * ctx)
{
    ctx->metaDataType = ctx->eqExp()->metaDataType;
    ctx->operand = ctx->eqExp()->operand;
}

//LOrExp
void SemanticAnalysis::enterLOrExpLAndExp(SysYParser::LOrExpLAndExpContext * ctx)
{
    ctx->metaDataType = MetaDataType::VOID;
}

void SemanticAnalysis::exitLOrExpLAndExp(SysYParser::LOrExpLAndExpContext * ctx)
{
    ctx->metaDataType = ctx->lAndExp()->metaDataType;

    ctx->operand = ctx->lAndExp()->operand;
}
void SemanticAnalysis::enterLOrExpLOrExp(SysYParser::LOrExpLOrExpContext * ctx)
{
    ctx->metaDataType = MetaDataType::VOID;
}

void SemanticAnalysis::exitLOrExpLOrExp(SysYParser::LOrExpLOrExpContext * ctx)
{
    ctx->metaDataType = ctx->lAndExp()->metaDataType;
    if (ctx->metaDataType != MetaDataType::INT || ctx->lAndExp()->metaDataType != MetaDataType::INT) {
        throw std::runtime_error("[ERROR] > logic calculation with non-boolean operands.\n");
    }

    IRTempVariable* result = irGenerator->addTempVariable(ctx->metaDataType);

    IROr *code = new IROr(result, ctx->lOrExp()->operand, ctx->lAndExp()->operand);
    irGenerator->addCode(code);
    ctx->operand = result;
}


// Number
void SemanticAnalysis::enterNumberIntConst(SysYParser::NumberIntConstContext * ctx)
{

}
void SemanticAnalysis::exitNumberIntConst(SysYParser::NumberIntConstContext * ctx)
{
    ctx->metaDataType = MetaDataType::INT;
}

void SemanticAnalysis::enterNumberFloatConst(SysYParser::NumberFloatConstContext * ctx)
{

}
void SemanticAnalysis::exitNumberFloatConst(SysYParser::NumberFloatConstContext * ctx)
{
    ctx->metaDataType = MetaDataType::FLOAT;
}

void SemanticAnalysis::enterEveryRule(antlr4::ParserRuleContext * ctx) {}
void SemanticAnalysis::exitEveryRule(antlr4::ParserRuleContext * ctx) {}
void SemanticAnalysis::visitTerminal(antlr4::tree::TerminalNode * node) {}
void SemanticAnalysis::visitErrorNode(antlr4::tree::ErrorNode * node) {}