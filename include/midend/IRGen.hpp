#ifndef IRGEN_HPP
#define IRGEN_HPP

#include <map>
#include <memory>
#include <vector>

#include "BasicBlock.hpp"
#include "Value.hpp"
#include "Constant.hpp"

#include "../frontend/node.hpp"

class Function;



std::unique_ptr<Module> module= std::unique_ptr<Module>(new Module("sysY code"));

class Scope {
    public:
        void enter() { 
            lookup_var_table.push_back({});
            lookup_func_table.push_back({});
            lookup_array_size_table.push_back({});
            lookup_array_const_table.push_back({});    
        }
        void exit() { 
            lookup_var_table.pop_back(); 
            lookup_func_table.pop_back();
            lookup_array_size_table.pop_back();
            lookup_array_const_table.pop_back();    
        }

        bool inGlobal() { return lookup_var_table.size() == 1; }

        bool push(std::string name, Value *val) {
            auto result = lookup_var_table[lookup_var_table.size() - 1].insert({name, val});
            return result.second; 
        }

        Value* find(std::string name) {
            for(auto v = lookup_var_table.rbegin(); v != lookup_var_table.rend(); v++) {
                auto iter = v->find(name);
                if(iter != v->end()) return iter->second;
            }

            return nullptr;
        }

        bool pushFunc(std::string name, Value *val) {
            auto result = lookup_func_table[lookup_func_table.size()-1].insert({name, val});
            return result.second;
        }

        Value* findFunc(std::string name) {
            for(auto v = lookup_func_table.rbegin(); v != lookup_func_table.rend(); v++) {
                auto iter = v->find(name);
                if(iter != v->end()) return iter->second;
            }

            return nullptr;
        }

        bool pushSize(std::string name, std::vector<int> size) {
            auto result = lookup_array_size_table[lookup_array_size_table.size()-1].insert({name, size});
            return result.second;
        }

        std::vector<int> findSize(std::string name) {
            for(auto v = lookup_array_size_table.rbegin(); v != lookup_array_size_table.rend(); v++) {
                auto iter = v->find(name);
                if(iter != v->end()) return iter->second;
            }

            return {};
        }

        bool pushConst(std::string name, ConstantArray* array) {
            auto result = lookup_array_const_table[lookup_array_const_table.size()-1].insert({name, array});
            return result.second;
        }

        ConstantArray* findConst(std::string name) {
            for(auto v = lookup_array_const_table.rbegin(); v != lookup_array_const_table.rend(); v++) {
                auto iter = v->find(name);
                if(iter != v->end()) return iter->second;
            }

            return nullptr;
        }


    private:
        std::vector<std::map<std::string, Value*>> lookup_var_table;
        std::vector<std::map<std::string, Value*>> lookup_func_table;
        std::vector<std::map<std::string, std::vector<int>>> lookup_array_size_table;
        std::vector<std::map<std::string, ConstantArray*>> lookup_array_const_table;
};


class IRGen : public ast::ASTVisitor {
   // public:
   //     static std::unique_ptr<Module> module;

    public:
        IRGen() {
           

         //  builder = std::make_unique<IRBuilder>(nullptr, module.get());

            auto VOID_T = Type::getVoidType(module.get());
            auto INT1_T = Type::getInt1Type(module.get());
            auto INT32_T = Type::getInt32Type(module.get());
            auto INT32PTR_T = Type::getInt32PtrType(module.get());
            auto FLOAT_T = Type::getFloatType(module.get());
            auto FLOATPTR_T = Type::getFloatPtrType(module.get());

            auto input_type = FunctionType::get(INT32_T, {});
            auto get_int =
                Function::create(
                        input_type,
                        "getint",
                        module.get());

            input_type = FunctionType::get(FLOAT_T, {});
            auto get_float =
                Function::create(
                        input_type,
                        "getfloat",
                        module.get());

            input_type = FunctionType::get(INT32_T, {});
            auto get_char =
                Function::create(
                        input_type,
                        "getch",
                        module.get());

            std::vector<Type *> input_params;
            std::vector<Type *>().swap(input_params);
            input_params.push_back(INT32PTR_T);
            input_type = FunctionType::get(INT32_T, input_params);
            auto get_array =
                Function::create(
                        input_type,
                        "getarray",
                        module.get());

            std::vector<Type *>().swap(input_params);
            input_params.push_back(FLOATPTR_T);
            input_type = FunctionType::get(INT32_T, input_params);
            auto get_farray =
                Function::create(
                        input_type,
                        "getfarray",
                        module.get());

            std::vector<Type *> output_params;
            std::vector<Type *>().swap(output_params);
            output_params.push_back(INT32_T);
            auto output_type = FunctionType::get(VOID_T, output_params);
            auto put_int =
                Function::create(
                        output_type,
                        "putint",
                        module.get());

            std::vector<Type *>().swap(output_params);
            output_params.push_back(FLOAT_T);
            output_type = FunctionType::get(VOID_T, output_params);
            auto put_float =
                Function::create(
                        output_type,
                        "putfloat",
                        module.get());

            std::vector<Type *>().swap(output_params);
            output_params.push_back(INT32_T);
            output_type = FunctionType::get(VOID_T, output_params);
            auto put_char =
                Function::create(
                        output_type,
                        "putch",
                        module.get());

            std::vector<Type *>().swap(output_params);
            output_params.push_back(INT32_T);
            output_params.push_back(INT32PTR_T);
            output_type = FunctionType::get(VOID_T, output_params);
            auto put_array =
                Function::create(
                        output_type,
                        "putarray",
                        module.get());

            std::vector<Type *>().swap(output_params);
            output_params.push_back(INT32_T);
            output_params.push_back(FLOATPTR_T);
            output_type = FunctionType::get(VOID_T, output_params);
            auto put_farray =
                Function::create(
                        output_type,
                        "putfarray",
                        module.get());

            std::vector<Type *>().swap(input_params);
            input_params.push_back(INT32_T);
            auto time_type = FunctionType::get(VOID_T, input_params);
            auto start_time =
                Function::create(
                        time_type,
                        "_sysy_starttime",
                        module.get());

            std::vector<Type *>().swap(input_params);
            input_params.push_back(INT32_T);
            time_type = FunctionType::get(VOID_T, input_params);
            auto stop_time =
                Function::create(
                        time_type,
                        "_sysy_stoptime",
                        module.get());

            scope.enter();
            scope.pushFunc("getint", get_int);
            scope.pushFunc("getfloat", get_float);
            scope.pushFunc("getch", get_char);
            scope.pushFunc("getarray", get_array);
            scope.pushFunc("getfarray", get_farray);
            scope.pushFunc("putint", put_int);
            scope.pushFunc("putfloat", put_float);
            scope.pushFunc("putch", put_char);
            scope.pushFunc("putarray", put_array);
            scope.pushFunc("putfarray", put_farray);
            scope.pushFunc("starttime", start_time);
            scope.pushFunc("stoptime", stop_time);
           
        }

        std::unique_ptr<Module> getModule() { return std::move(module); }
  

    private:
        virtual void visit(ast::CompunitNode &node) override;
        virtual void visit(ast::FuncFParam &node) override;
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
        virtual void visit(ast::BlockStmt &node) override;
        virtual void visit(ast::IfStmt &node) override;
        virtual void visit(ast::WhileStmt &node)override;
        virtual void visit(ast::CallExpr &node) override;
        virtual void visit(ast::RetStmt &node) override;
        virtual void visit(ast::ContinueStmt &node)override;
        virtual void visit(ast::BreakStmt &node) override;
        virtual void visit(ast::EmptyStmt &node) override;

    private:
       // InitZeroJudger zero_judger;
        //std::unique_ptr<IRBuilder> builder;
        Scope scope;
       

        
};



#endif