#include "backend/Interval.hpp"

void Interval::addShortInterval(int low, int high){
    ::std::pair<int, int>* short_interval = new ::std::pair(low, high);
    auto iter = small_intervals.begin();
    if(iter!=small_intervals.end()){
        auto short_interval_ = *iter;
        if(low<short_interval_->first){
            if(high>=short_interval_->first && high<=short_interval_->second){
                short_interval_->first = low;
            }
            else{
                small_intervals.insert(small_intervals.begin(), short_interval);
            }
        }
        else if(low>=short_interval_->first && low<=short_interval_->second){
            short_interval_->second = high>short_interval_->second?high:short_interval_->second;
        }
        else{
            small_intervals.insert(small_intervals.begin(), short_interval);
        }
    }
    else{
        small_intervals.insert(small_intervals.begin(), short_interval);
    }
    return;
}



bool Interval::isOverLap(Interval* interval){
    for(auto si_1: small_intervals){
        for(auto si_2: interval->small_intervals){
            if(!(si_1->second<=si_2->first||si_2->second<=si_1->first)){
                return true;
            }
        }
    }
    return false;
}




