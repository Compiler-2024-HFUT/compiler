#include "node.hpp"
#include "parser.hpp"
#include <iomanip>
#include <memory>
#include <fstream>
#include <iostream>
using namespace std;
int main(int argc , char**argv){
    ifstream ifs;
        ifs.open("../../../../test/1.sy",ios::in);

    //std::string some_str(begin, end);

 
    if (!ifs.is_open())
    {
        cout << "read fail." << endl;
        return -1;
    }
	string content( (istreambuf_iterator<char>(ifs) ),
					 (istreambuf_iterator<char>() ) );

    // char buf[1000] = { 0 };
    // // int a=0;
    // // while (ifs >> buf)
    // // {   a++;
    // //     cout << buf <<"\tline "<<a<<endl;
    // // }
	// cout << content << endl;
	ifs.close();
    Lexer lexTest{content};
    std::unique_ptr<Token> tok;
    cout<<"line"<<std::setw(8)<<"column"<<std::setw(8)<<"type"<<std::setw(8)<<"literal"<<endl;
    do{
        tok=lexTest.nextToken();
        cout<<tok->line<<std::setw(8)<<tok->column<<std::setw(8)<<tok->type<<std::setw(8)<<tok->literal<<'\n';
    }while(tok->type!=LEXEOF);
    Parser *p=new Parser{content};
    p->parserComp();
    for(auto&i:p->comp->global_defs){
        cout<<i->name<<endl;
        if(i->getType()==ast::StmtType::FUNSTMT){
            auto * fun=(ast::funcDef*)(i.get());
            for(auto & [ i, b]:fun->argv){
                cout<<i<<"\t"<<b<<endl;
            }
        }
    }
    delete (p);

}