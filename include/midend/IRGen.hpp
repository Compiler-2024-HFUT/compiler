#ifndef IRGen_HPP
#define IRGen_HPP

#include <map>
#include <memory>
#include <vector>

#include "Value.hpp"
#include "Module.hpp"
#include "IRPut.hpp"
#include "Constant.hpp"
#include "frontend/node.hpp"

class IRBuilder;

class Scope {
    public:
        void enter() { 
            varTable.push_back({});
            funcTable.push_back({});
            arraySizeTable.push_back({});
            arrayConstTable.push_back({});    
        }
        void exit() { 
            varTable.pop_back(); 
            funcTable.pop_back();
            arraySizeTable.pop_back();
            arrayConstTable.pop_back();    
        }

        bool inGlobal() { return varTable.size() == 1; }

        bool push(std::string name, Value *val) {
            auto result = varTable[varTable.size() - 1].insert({name, val});
            return result.second; 
        }

        Value* find(std::string name) {
            for(auto v = varTable.rbegin(); v != varTable.rend(); v++) {
                auto iter = v->find(name);
                if(iter != v->end()) return iter->second;
            }

            return nullptr;
        }

        bool pushFunc(std::string name, Value *val) {
            auto result = funcTable[funcTable.size()-1].insert({name, val});
            return result.second;
        }

        Value* findFunc(std::string name) {
            for(auto v = funcTable.rbegin(); v != funcTable.rend(); v++) {
                auto iter = v->find(name);
                if(iter != v->end()) return iter->second;
            }

            return nullptr;
        }

        bool pushSize(std::string name, std::vector<int> size) {
            auto result = arraySizeTable[arraySizeTable.size()-1].insert({name, size});
            return result.second;
        }

        std::vector<int> find_size(std::string name) {
            for(auto v = arraySizeTable.rbegin(); v != arraySizeTable.rend(); v++) {
                auto iter = v->find(name);
                if(iter != v->end()) return iter->second;
            }

            return {};
        }

        bool pushConst(std::string name, ConstantArray* array) {
            auto result = arrayConstTable[arrayConstTable.size()-1].insert({name, array});
            return result.second;
        }

        ConstantArray* findConst(std::string name) {
            for(auto v = arrayConstTable.rbegin(); v != arrayConstTable.rend(); v++) {
                auto iter = v->find(name);
                if(iter != v->end()) return iter->second;
            }

            return nullptr;
        }


    private:
        std::vector<std::map<std::string, Value*>> varTable;
        std::vector<std::map<std::string, Value*>> funcTable;
        std::vector<std::map<std::string, std::vector<int>>> arraySizeTable;
        std::vector<std::map<std::string, ConstantArray*>> arrayConstTable;
};


class IRGen : public ast::ASTVisitor {
  public:
    IRGen() {
        module = std::unique_ptr<Module>(new Module("sysY code"));
        builder = std::make_unique<IRBuilder>(nullptr, module.get());

        auto voidType = Type::getVoidType(module.get());
        auto int1Type = Type::getInt1Type(module.get());
        auto int32Type = Type::getInt32Type(module.get());
        auto int32PtrType = Type::getInt32PtrType(module.get());
        auto floatType = Type::getFloatType(module.get());
        auto floatPtrType = Type::getFloatPtrType(module.get());

        auto inputType = FuncType::makeFuncType(int32Type, {});
        auto getInt =
            Function::makeFunc(
                    inputType,
                    "getint",
                    module.get());

        inputType = FuncType::makeFuncType(floatType, {});
        auto getFloat =
            Function::makeFunc(
                    inputType,
                    "getfloat",
                    module.get());

        inputType = FuncType::makeFuncType(int32Type, {});
        auto getChar =
            Function::makeFunc(
                    inputType,
                    "getch",
                    module.get());

        std::vector<Type *> inputParams;
        std::vector<Type *>().swap(inputParams);
        inputParams.push_back(int32PtrType);
        inputType = FuncType::makeFuncType(int32Type, inputParams);
        auto getArray =
            Function::makeFunc(
                    inputType,
                    "getarray",
                    module.get());

        std::vector<Type *>().swap(inputParams);
        inputParams.push_back(floatPtrType);
        inputType = FuncType::makeFuncType(int32Type, inputParams);
        auto getFArray =
            Function::makeFunc(
                    inputType,
                    "getfarray",
                    module.get());

        std::vector<Type *> outputParams;
        std::vector<Type *>().swap(outputParams);
        outputParams.push_back(int32Type);
        auto output_type = FuncType::makeFuncType(voidType, outputParams);
        auto putInt =
            Function::makeFunc(
                    output_type,
                    "putint",
                    module.get());

        std::vector<Type *>().swap(outputParams);
        outputParams.push_back(floatType);
        output_type = FuncType::makeFuncType(voidType, outputParams);
        auto putFloat =
            Function::makeFunc(
                    output_type,
                    "putfloat",
                    module.get());

        std::vector<Type *>().swap(outputParams);
        outputParams.push_back(int32Type);
        output_type = FuncType::makeFuncType(voidType, outputParams);
        auto putChar =
            Function::makeFunc(
                    output_type,
                    "putch",
                    module.get());

        std::vector<Type *>().swap(outputParams);
        outputParams.push_back(int32Type);
        outputParams.push_back(int32PtrType);
        output_type = FuncType::makeFuncType(voidType, outputParams);
        auto putArray =
            Function::makeFunc(
                    output_type,
                    "putarray",
                    module.get());

        std::vector<Type *>().swap(outputParams);
        outputParams.push_back(int32Type);
        outputParams.push_back(floatPtrType);
        output_type = FuncType::makeFuncType(voidType, outputParams);
        auto putFArray =
            Function::makeFunc(
                    output_type,
                    "putfarray",
                    module.get());

        std::vector<Type *>().swap(inputParams);
        inputParams.push_back(int32Type);
        auto timeType = FuncType::makeFuncType(voidType, inputParams);
        auto startTime =
            Function::makeFunc(
                    timeType,
                    "_sysy_starttime",
                    module.get());

        std::vector<Type *>().swap(inputParams);
        inputParams.push_back(int32Type);
        timeType = FuncType::makeFuncType(voidType, inputParams);
        auto stopTime =
            Function::makeFunc(
                    timeType,
                    "_sysy_stoptime",
                    module.get());

        scope.enter();
        scope.pushFunc("getint", getInt);
        scope.pushFunc("getfloat", getFloat);
        scope.pushFunc("getch", getChar);
        scope.pushFunc("getarray", getArray);
        scope.pushFunc("getfarray", getFArray);
        scope.pushFunc("putint", putInt);
        scope.pushFunc("putfloat", putFloat);
        scope.pushFunc("putch", putChar);
        scope.pushFunc("putarray", putArray);
        scope.pushFunc("putfarray", putFArray);
        scope.pushFunc("starttime", startTime);
        scope.pushFunc("stoptime", stopTime);
        
    }

    std::unique_ptr<Module> get_module() { return std::move(module); }

  private:
    virtual void visit(ast::CompunitNode &node) override;
    virtual void visit(ast::FuncDef &node) override;
    virtual void visit(ast::ValDeclStmt &node) override;
    virtual void visit(ast::ValDefStmt &node) override;
    virtual void visit(ast::ArrDefStmt &node) override;
    virtual void visit(ast::ConstDeclStmt &node)override;
    virtual void visit(ast::ConstDefStmt &node)override;
    virtual void visit(ast::ConstArrDefStmt &node) override;
    virtual void visit(ast::ExprStmt &node) override;
    virtual void visit(ast::AssignStmt &node) override;
    virtual void visit(ast::UnaryExpr &node) override;
    // virtual void visast::it(InfixExpr &node) = 0;
    virtual void visit(ast::AssignExpr &node) override;
    virtual void visit(ast::RelopExpr &node) override;
    virtual void visit(ast::EqExpr &node) override;
    virtual void visit(ast::AndExp &node) override;
    virtual void visit(ast::ORExp &node)override;
    virtual void visit(ast::BinopExpr &node) override;
    virtual void visit(ast::LvalExpr &node)override;
    virtual void visit(ast::IntConst &node) override;
    virtual void visit(ast::InitializerExpr &node) override;
    virtual void visit(ast::FloatConst &node)override;
    // virtual void visast::it(AssignStmt &node) = 0;
    virtual void visit(ast::BlockStmt &node) override;
    virtual void visit(ast::IfStmt &node) override;
    virtual void visit(ast::WhileStmt &node)override;
    virtual void visit(ast::CallExpr &node) override;
    virtual void visit(ast::RetStmt &node) override;
    virtual void visit(ast::ContinueStmt &node)override;
    virtual void visit(ast::BreakStmt &node) override;
    virtual void visit(ast::EmptyStmt &node) override;

    //  bool is_all_zero(ASTInitVal &);


    // InitZeroJudger zero_judger;
    std::unique_ptr<IRBuilder> builder;
    Scope scope;
    std::unique_ptr<Module> module;
};
#endif