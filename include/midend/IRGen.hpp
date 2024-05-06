#ifndef IRGEN_HPP
#define IRGEN_HPP

#include <map>
#include <memory>
#include <vector>

#include "BasicBlock.hpp"
#include "Value.hpp"
#include "Constant.hpp"
#include "Function.hpp"
#include "frontend/node.hpp"
#include "midend/Module.hpp"


namespace IRgen {

#define CONST_INT(num)  ConstantInt::get(num, module.get())
#define CONST_FP(num)   ConstantFP::get(num, module.get())

//& global variables

static Value *tmp_val = nullptr;       //& store tmp value
static Type  *cur_type = nullptr;      //& store current type
static bool require_lvalue = false;    //& whether require lvalue
static bool pre_enter_scope = false;   //& whether pre-enter scope

static bool from_func = false;         // replace pre_enter_scope

static Type *VOID_T;
static Type *INT1_T;
static Type *INT32_T;
static Type *FLOAT_T;
static Type *INT32PTR_T;
static Type *FLOATPTR_T;               //& types used for IR builder

struct true_false_BB {
    BasicBlock *trueBB = nullptr;
    BasicBlock *falseBB = nullptr;
};                              //& used for backpatching

static std::list<true_false_BB> IF_WHILE_Cond_Stack; //& used for Cond
static std::list<true_false_BB> While_Stack;         //& used for break and continue



static std::vector<BasicBlock*> cur_basic_block_list;

static Function *cur_fun = nullptr;    //& function that is being built
static BasicBlock *entry_block_of_cur_fun;
static BasicBlock *cur_block_of_cur_fun;   //& used for add instruction 

static bool has_global_init;
// static BasicBlock *global_init_block;

static bool is_init_const_array = false;
static int arr_total_size = 1;
static std::vector<int> array_bounds;
static std::vector<int> array_sizes;
// static std::vector<int> array_sizes;
// pair( the pos when into {, the offset bettween { and } )
static std::vector< std::pair<int, int> > array_pos;
static int cur_pos;
static int cur_depth;     
static std::map<int, Value*> init_val_map; 
static std::vector<Constant*> init_val;    //& used for constant initializer

static BasicBlock *ret_BB;
static Value *ret_addr;   //& ret BB

static bool is_inited_with_all_zero;



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