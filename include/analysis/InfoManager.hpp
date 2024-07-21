#ifndef INFO_MAN_HPP
#define INFO_MAN_HPP
#include "midend/Function.hpp"
#include "analysis/Info.hpp"
#include "utils/Logger.hpp"

#include <map>
#include <memory>
#include <vector>

class InfoManager{
    Module*module_;
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
        
        if(res->isInvalid()) {
            res->analyse();
        }
        
        if(res == nullptr) {
            LOG_ERROR("you don't add info", 1)
        }

        return res;
    }

    std::vector<Info*>& getInfos() {
        return infos;
    }
    template<class InfoType>
    void addInfo() {
        infos.push_back( new InfoType(module_, this) );
    }

    void run();

    explicit InfoManager(Module*m):module_(m){}
};
#endif