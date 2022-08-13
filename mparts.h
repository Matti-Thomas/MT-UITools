#include "mglyph.h"
#ifndef MPARTS_H
#define MPARTS_H
#define BLUE_ONE vec3f(20.f/255,130.f/255,172.f/255)
#define BLUE_TWO vec3f(100.f/255,30.f/255,170.f/255)
#define WHITE_ONE vec3f(1,1,1)
#define GRAY_ONE vec3f(192.f/255,192.f/255,192.f/255)
#define GRAY_TWO vec3f(255.f/255,245.f/255,238.f/255)

// 检测类型有无resize成员函数
template<typename T,typename=void>    // 主模板
struct has_resize_mem : std::false_type { };
// 特化识别具有resize的类型
template<typename T>
struct has_resize_mem<T,std::void_t<decltype( std::declval<T>().resize(1,1))>>:
         std::true_type { };

template<typename T, typename... Args>   // 图元的通用工厂函数模板
auto make_mglyph(Args&&... args){          
   if constexpr (std::is_base_of_v<Mglyph,T>)
       return std::make_shared<T>(std::forward<Args>(args)...);
   else
       return nullptr;
}

//  通用事件添加函数及其重载
template<typename T>
void addEventHandler(Mwindow& wdw,MwinEvent e,T&&t){
      auto& event_mg=MeventManager::instance();
      auto signal =mhash(wdw.get_id(),e,event_mg.get_count(e),0,0);
      event_mg.addEventCB(signal, [&wdw,t=std::forward<T>(t)](){t(wdw);},e);
}

template<typename T,typename M>
void addEventHandler(Mwindow& wdw,std::shared_ptr<M> m,McurEvent e,T&&t){
      decltype(auto) func=std::forward<T>(t);
      auto& event_mg=MeventManager::instance();
      auto signal =mhash(wdw.get_id(),0,0,m->get_id(),e);
      event_mg.addEventCB(signal, [&wdw,m=m.get(),&func](){func(wdw,m);});
      m->mark_handler(e);
      if constexpr (std::is_base_of_v<Container,M>){
            for(auto it:*m){
                if(!it->has_handler(e)){
                auto signal =mhash(wdw.get_id(),0,0,it->get_id(),e);
                event_mg.addEventCB(signal, [&wdw,m=m.get(),&func](){func(wdw,m);});
                it->mark_handler(e);
                }
            }
      }
}

template<typename T,typename M>
void addEventHandler(Mwindow& wdw,M* m,McurEvent e,T&&t){
      decltype(auto) func=std::forward<T>(t);
      auto& event_mg=MeventManager::instance();
      auto signal =mhash(wdw.get_id(),0,0,m->get_id(),e);
      event_mg.addEventCB(signal, [&wdw,m,&func](){func(wdw,m);});
      m->mark_handler(e);
      if constexpr (std::is_base_of_v<Container,M>){
            for(auto it:*m){
                if(!it->has_handler(e)){
                auto signal =mhash(wdw.get_id(),0,0,it->get_id(),e);
                event_mg.addEventCB(signal, [&wdw,m,&func](){func(wdw,m);});
                it->mark_handler(e);
                }
            }
      }
}


// 设置图元尺寸随窗口变化
template<typename T,typename=std::enable_if_t<has_resize_mem<T>::value>>
void flex_mglyph(std::shared_ptr<T> m ,Mwindow& wdw,float wr,float yr){
          addEventHandler(wdw,RESIZE,[m,wr,yr](Mwindow& wdw){
                                         m->resize(wr*wdw.getWidth(),
                                                        yr*wdw.getHeight());});
}

template<typename T,typename=std::enable_if_t<has_resize_mem<T>::value>>
void flex_mglyph(std::shared_ptr<T> m ,Mwindow& wdw){
          addEventHandler(wdw,RESIZE,[m](Mwindow& wdw){
                                         m->resize(wdw.getWidth(),
                                                        m->getHeight());});
}

template<typename T,typename=std::enable_if_t<has_resize_mem<T>::value>>
void flex_mglyph(std::shared_ptr<T> m ,Mwindow& wdw,float w){
          addEventHandler(wdw,RESIZE,[m,w](Mwindow& wdw){
                                         m->resize(w,wdw.getHeight());});
}

// 上下文画布
class Mcontent:public Row{
    public:
        Mcontent(Mwindow& wdw,const vec3f& co):_w(wdw),
             Row(wdw.getWidth(),wdw.getHeight(),co){
             addEventHandler(wdw,RESIZE,[this](Mwindow& wdw){resize(wdw.getWidth(),
                                                                    wdw.getHeight());});
        }
        
       void display(){
             
             draw(_w);
             
             _w.refresh();
             
       }
    private:
       Mwindow& _w;
};

// 可拖动图元wrapper类
template<typename T>
class DragGlyph:public T{
   public:
            template<typename... Args>
            DragGlyph(Mwindow& wdw,Args&&...  args):T(std::forward<Args>(args)...){
                  addEventHandler(wdw,this,CURSOR_DRAG,
                              [](Mwindow&,DragGlyph<T>* p){
                                   p->drag_fun();});
                  addEventHandler(wdw,this,CURSOR_DRAG_RESET,
                              [](Mwindow&,DragGlyph<T>* p){p->drag_rfun();});
            }  
            void draw(Mwindow& wdw) override {
                  if(is_moved){
                        auto [originx,originy]=T::getPos();
                        T::setPos(real_pos._X,real_pos._Y);
                        T::draw(wdw);
                        T::setPos(originx,originy);
                  }      
                  else {
                       T::draw(wdw); 
                  }       
            }
            void drag_fun(){
                     if(first_inter){
                           cur_pos=getCursorPos();
                           first_inter=false;
                           if (!real_pos._X && !real_pos._Y)
                                 real_pos=T::getPos();
                           return;
                     }      
                     auto [currX,currY]=getCursorPos();
                     float deltaX=currX-cur_pos._X;
                     float deltaY=currY-cur_pos._Y;
                     _callback->call(this,deltaX,deltaY);
                     cur_pos._X=currX;
                     cur_pos._Y=currY;
                     is_moved=true;
            }
            void drag_rfun(){
                  first_inter=true;
            }
            void move_toX(float x,float lo,float hi){
                  real_pos._X += x;
                  if(real_pos._X<lo)
                        real_pos._X=lo;
                  if(real_pos._X>hi)
                        real_pos._X=hi;
            }
            void move_toY(float y,float lo,float hi){
                  real_pos._Y -= y;
                  if(real_pos._Y<lo)
                        real_pos._Y=lo;
                  if(real_pos._Y>hi)
                        real_pos._Y=hi;
            }
           float get_realX(){
               return real_pos._X;
           }
           float get_realY(){
               return real_pos._Y;
           }

           template<typename G>
            void set_cb(G func ){
             _callback=std::make_unique<CallFunc<G>>(func);
            }
   private:
            Pos2D<float> real_pos;
            Pos2D<double> cur_pos;
            struct CallBase{
            void virtual call(DragGlyph<T>*,float,float)=0;
             ~CallBase()=default; 
            };
            template<typename F>
            class CallFunc:public CallBase{
                  F func;
            public:
                  CallFunc(F t):func(t){}
                  void call(DragGlyph<T>* dg,float dx,float dy) override{
                  func(dg,dx,dy);
                  }
            } ;
            std::unique_ptr<CallBase> _callback;
            bool first_inter=true;
            bool is_moved = false;     
};

// 下拉菜单wrapper类
template<typename T>
class DropList:public T{
   public:
      template<typename... Args>
      DropList(Args&&...  args):T(std::forward<Args>(args)...){
          
      }
      void setPos(float x,float y) override {
               T::setPos(x,y);
               set_list_pos();
      }
        
      void draw(Mwindow& wdw) override {
            T::draw(wdw);
            if(expand)
                  _list->draw(wdw);
        }
      void on(){
            expand=true;
      }
      void off(){
            expand=false;
      }
      // 初始化下拉菜单，设置形状属性以及与装饰图元的偏移距离
      void inital(float w,float h,const vec3f& co,float offsetx,float offsety){
              _list=std::make_unique<Column>(w,h,co);
              _listX=offsetx;
              _listY=offsety;
              _list->center();
      }
      void set_attr(const vec3f& co){
          _list->set_bg(co);
      }
      void set_attr(Direction d,float val){
          _list->setPadding(d,val);
      }
       void set_attr(float w,float h){
          _list->resize(w,h);
      }
      void add_item(std::shared_ptr<Mglyph> it){      
         //it->addEventHandler(MOUSE_LEFT_CLICK,clickCB);
         _list->push(it);
      }

      void set_drop_depth(float val){
            _list->set_depth(val);
      } 
      void set_padding(Direction d,float x){
            _list->setPadding(d,x);
      }
      template<typename F>
      F& get_drop(unsigned index){
            return _list->get_item<F>(index);
      }
private:
      bool expand=false;
      float _listX,_listY;
      std::unique_ptr<Column> _list; 
      void set_list_pos(){
                  _list->setPos(_listX+T::origin._X,T::origin._Y+_listY);
      }   
};


/*菜单栏图元，暂时没必要写
class Menubar:public Row{
   public:
      Menubar(Mwindow& wdw,const vec3f& co,float h):
           Row(wdw.getWidth(),h,co){ 
           addEventHandler(wdw,RESIZE,[this,h](Mwindow& wdw){resize(wdw.getWidth(),h);});
      }
};
*/
class MsliderView:public ViewGlyph{
   public:
        using Slider=DragGlyph<Rectangle>;
        MsliderView(Mwindow& wdw, float w,float h,float t,float len):
        ViewGlyph(w,h),sli_len(len),sli_th(t),
        x_slider{std::make_unique<Slider>(wdw,len,t,vec3f(0.3,0.3,0.3))},
        y_slider{std::make_unique<Slider>(wdw,t,len,vec3f(0.3,0.3,0.3))},
        x_rect(w,t,vec3f(0.7,0.7,0.7)),y_rect(t,h,vec3f(0.7,0.7,0.7)){
             x_rect.setPos(0,t-height);
             y_rect.setPos(width-t,0);
             x_slider->setPos(0,t-height);
             y_slider->setPos(width-t,0);
             x_slider->set_cb([this](Slider* s,float x,float){x_sliderCB(s,x);});
             y_slider->set_cb([this](Slider* s,float ,float y){y_sliderCB(s,y);});
        }
        void x_sliderCB(Slider* sd,float dx){
              auto r=(_source->getWidth()-width)/(width-sli_len-sli_th);
              sd->move_toX(dx,0,width-sli_len-sli_th);
              _source->setPos(-r*sd->get_realX(),_source->getPos()._Y);
        }
        void y_sliderCB(Slider* sd,float dy){
             auto r=(_source->getHeight()-height)/(height-sli_len-sli_th);
             sd->move_toY(dy,-height+sli_len+sli_th,0); 
            _source->setPos(_source->getPos()._X,-r*sd->get_realY());
        }
        void custom_draw(Mwindow& wdw) override{
               ViewGlyph::custom_draw(wdw);
               if(!_source) return;
               if(_source->getHeight()>height){
                   y_rect.draw(wdw);
                   y_slider->draw(wdw);
                }
                if(_source->getWidth()>width){
                  x_rect.draw(wdw);
                  x_slider->draw(wdw);
                }
        }
        void set_slider_color(const vec3f& co){
            x_slider->set_bg(co);
            y_slider->set_bg(co);
        }
        void set_axis_color(const vec3f& co){
            x_rect.set_bg(co);
            y_rect.set_bg(co);
        }
        void bind_mg(std::shared_ptr<Mglyph> m){
            ViewGlyph::bind_mg(m);
            x_rect.setPos(0,sli_th-height);
            y_rect.setPos(width-sli_th,0);
            x_slider->setPos(0,sli_th-height);
            y_slider->setPos(width-sli_th,0);
        }
        ~MsliderView(){}
   private:
       float sli_len,sli_th; 
       std::unique_ptr<Slider> x_slider,y_slider;
       Rectangle x_rect,y_rect;
};

void run(Mwindow& wdw,std::shared_ptr<Mcontent> mct){
      while(!wdw.isClosed()){
         mct->display();
         wait_event();
      }
}
#endif

