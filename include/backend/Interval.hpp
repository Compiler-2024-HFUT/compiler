#ifndef INTERVAL_HPP__
#define INTERVAL_HPP__
#include "midend/Module.hpp"

class Interval{
    public:
        Interval(Value* value): value(value), is_float(value->getType()->isFloatType()){}
        void addUP(int up){ups.push_front(up);}
        void addShortInterval(int low, int high);      

        bool isOverLap(Interval* interval);
        bool isFloat(){return is_float;}

    private:
        Value* value;
        bool is_float;
        int reg=-1;
        ::std::list<int> ups;
        ::std::list<::std::pair<int, int>*> small_intervals;

        friend class cmp_interval;
        friend class LSRA;
        friend class AsmGen;
};
#endif