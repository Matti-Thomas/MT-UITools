#ifndef MEVENT_H
#define MEVENT_H
#include <functional>

enum  MwinEvent {RESIZE=1,KEYBORD,SCROLL,FILE_DROP,INPUT,CLOSE,END};
enum  McurEvent {MOUSE_ENTER=14,MOUSE_QUIT=15,
                           MOUSE_LEFT_CLICK =10,MOUSE_LEFT_RESET =12,
                           MOUSE_RIGHT_CLICK=11,MOUSE_RIGHT_RESET=13,
                           CURSOR_DRAG =16, CURSOR_DRAG_RESET=17
                           };
class MeventManager{
public:
   using CBfunc=std::function<void()>;
   MeventManager(MeventManager&)=delete;
   static MeventManager& instance(){
        static MeventManager mem;
        return mem;
    }
   void addEventCB(const size_t signal,const CBfunc& func){
       cbmap[signal]=func;
    }
    void addEventCB(const size_t signal,const CBfunc& func,MwinEvent e){
       cbmap[signal]=func;
       ++count_win_event[e];
    }
   void updateEventCB(const size_t& signal,const CBfunc& func) {
        cbmap[signal]=func;
    }
   void removeEventCB(const size_t& signal){
        cbmap.erase(signal);
    }
    bool receive(const size_t& signal) {
        auto iter=cbmap.find(signal);
        if(iter!=cbmap.end()){
            (iter->second)();
            return true;
        }
        return false;
    }
    decltype(auto) getEventCB(const size_t& signal){
        return cbmap.at(signal);
    }
    int get_count(MwinEvent e)  {
        return count_win_event[e];
    }
private:
   MeventManager(){}
   int count_win_event[END]={0};
   std::map<size_t,CBfunc> cbmap;    
};



#endif
