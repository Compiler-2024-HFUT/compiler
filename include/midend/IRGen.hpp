#ifndef IRGEN_HPP
#define IRGEN_HPP

#include <map>
#include <memory>
#include <vector>

#include "Function.hpp"
#include "frontend/node.hpp"

class ConstantArray;
namespace IRgen {


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
        IRGen();
        // std::unique_ptr<Module>& getModule() { return module; }
        Module*getModule() { return module.get(); }
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
        // std::unique_ptr<IRBuilder> builder;
        std::unique_ptr<Module> module;
        Scope scope;
};

}
#endif