#ifndef INFO_MAN_HPP
#define INFO_MAN_HPP
#include "midend/Function.hpp"
// #include "Dominators.hpp"
#include "analysis/Info.hpp"
// #include "optimization/PassManager.hpp"
#include <map>
#include <memory>
#include <vector>

class InfoManager{
    Module*module_;
    // std::map<Function*,std::unique_ptr<Dominators> >func_doms_;
    std::vector<Info*> infos;

public:
    template<class InfoType>
    InfoType* getInfo() {
        InfoType *res = nullptr;
        int i = 0;
        do {
            res = dynamic_cast<InfoType*>(infos[i]);
            i++;
        } while (res == nullptr && i < infos.size());
        return res;
    }

    template<class InfoContainer, class InfoType>
    void addInfo(InfoContainer *ic) {
        infos.push_back( new InfoType(ic, this) );
    }

    static InfoManager *createInfoManager(Module *m) { return new InfoManager(m); }

    // Dominators* getFuncDom(Function*f);
    void run();

    explicit InfoManager(Module*m):module_(m){}
};
#endif