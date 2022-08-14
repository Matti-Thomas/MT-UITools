#include <stdio.h>
#include "mparts.h"  
/* 文件虽然可以编译成一个图片查看器，但写的随意，只有100多行代码
 * 只是简单展示下编写的UI模块功能
 * 可以当成一个测试功能的文件
 * 原本文件名就叫test.cpp
 */
constexpr int max_char_num=100;  //储存图片路径的最长大小
// 从文件管理器加载图片
void open_img(std::shared_ptr<ViewGlyph> v){
    char img_path[max_char_num];
    auto file=popen("zenity --file-selection --title=\"选择图片\" 2> /dev/null","r");
    if(file) fgets(img_path,max_char_num,file);
    else std::cerr<<"can't open file!\n";
    if(pclose(file)!=0) return;
    img_path[strlen(img_path)-1]='\0';  //去掉末尾的换行
    auto img=make_mglyph<Image>(img_path);
    v->bind_mg(img); 
}

// 单纯的将一些临时的重复操作抽取成函数暂时使用下，所以函数参数很丑陋
void set_dropitems(Mwindow& wdw, const std::array<std::pair<const char*,const wchar_t*>,4>& its,std::shared_ptr<Row>m, int idx){
     for(auto p:its){
          auto drop_item= make_mglyph<Row>(200,36,BLUE_ONE);
          drop_item->setPadding(LEFT,12);
          auto img=make_mglyph<Image>(p.first,32,32);
          img->set_depth(0.3);
          auto txt=make_mglyph<Text>(p.second,25);
          txt->set_color(WHITE_ONE);
          txt->set_depth(0.3);
          drop_item->push(img);
          drop_item->push(txt);
          drop_item->set_depth(0.3);
          addEventHandler(wdw,drop_item,MOUSE_ENTER,
                   [](Mwindow&,Row* p){p->set_bg(BLUE_TWO);});
          addEventHandler(wdw,drop_item,MOUSE_QUIT,
                   [](Mwindow&,Row* p){p->set_bg(BLUE_ONE);});
     
          m->get_item<DropList<Row>>(idx).add_item(drop_item);
     }
}
int main(){
     envirInital();  // 初始化
     Mwindow wdw("MViewer",200,200,1500,1000);   // 创建窗口
     // 选择需要监听的输入事件
     wdw.listen(FILE_DROP);
     wdw.listen(SCROLL);
     wdw.listen(KEYBORD);
     wdw.listen(INPUT);
     // 也可以取消监听 wdw.disable_listen(KEYBORD);
     auto mct=make_mglyph<Mcontent>(wdw,WHITE_ONE); //绑定上下文画布到窗口  
     mct->set_layout(alignTop); 
     // 生成图元的工厂函数，所有类型图元
     // 包括派生自基本图元的用户自定义图元类型均由其统一生成
     auto bgptr=make_mglyph<BoxMglyph<MsliderView>>(wdw,1200,900,20,60);
     addEventHandler(wdw,FILE_DROP,[bgptr](Mwindow& wd){
          auto img=make_mglyph<Image>(wd.get_drop_file());
          bgptr->bind_mg(img);      
     });
     
     // 主菜单栏
     auto menu=make_mglyph<Row>(1500,50,BLUE_ONE);
     flex_mglyph(menu,wdw);
     menu->setPadding(LEFT,12);
     mct->push(menu);
     std::array<const wchar_t*,5> items{L"文件",L"编辑",L"视图",L"设置",L"帮助"};
     // 下拉子菜单
     for(auto cp:items){
          auto item=make_mglyph<DropList<Row>>(100,50);
          item->set_bg(BLUE_ONE);
          item->center();
          auto txt=make_mglyph<Text>(cp,36);
          txt->set_color(WHITE_ONE);
          item->push(txt);
          item->inital(300,150,BLUE_ONE,0,-50);
          item->set_drop_depth(0.3);
          // 添加鼠标事件响应
          addEventHandler(wdw,item,MOUSE_LEFT_CLICK,
                                     [](Mwindow&,DropList<Row>* p){p->on();});
          addEventHandler(wdw,item,MOUSE_LEFT_RESET,
                                     [](Mwindow&,DropList<Row>* p){p->off();});
          addEventHandler(wdw,item,MOUSE_ENTER,
                                     [](Mwindow&,DropList<Row>* p){p->set_bg(BLUE_TWO);});
          addEventHandler(wdw,item,MOUSE_QUIT,
                                     [](Mwindow&,DropList<Row>* p){p->set_bg(BLUE_ONE);});
          menu->push(item);
          
     }
     
     std::array<std::pair<const char*,const wchar_t*>,4> drop_items1{
          std::make_pair("icons/file.png",L"打开文件"),std::make_pair("icons/folder.png",L"打开文件夹"),
          std::make_pair("icons/save.png",L"保存"), std::make_pair("icons/print.png",L"打印")};
     set_dropitems(wdw,drop_items1,menu,0);
     auto& tmp=menu->get_item<DropList<Row>>(0).get_drop<Row>(0);
     addEventHandler(wdw,&tmp,MOUSE_LEFT_CLICK,[bgptr](Mwindow&,Row*){
                 open_img(bgptr);
     });
     std::array<std::pair<const char*,const wchar_t*>,4> drop_items2{
          std::make_pair("icons/cut.png",L"剪切"),std::make_pair("icons/copy.png",L"复制"),
          std::make_pair("icons/paste.png",L"粘贴"), std::make_pair("icons/delete.png",L"删除")};
     
     set_dropitems(wdw,drop_items2,menu,1);
     auto drop_item= make_mglyph<Row>(200,36,BLUE_ONE);
     auto img=make_mglyph<Image>("icons/full.png",32,32);
     auto txt=make_mglyph<Text>(L"全屏",25);
     img->set_depth(0.3);
     txt->set_color(WHITE_ONE);
     txt->set_depth(0.3);
     drop_item->push(img);
     drop_item->push(txt);
     drop_item->set_depth(0.3);
     addEventHandler(wdw,drop_item,MOUSE_ENTER,
               [](Mwindow&,Row* p){p->set_bg(BLUE_TWO);});
     addEventHandler(wdw,drop_item,MOUSE_QUIT,
               [](Mwindow&,Row* p){p->set_bg(BLUE_ONE);});
     menu->get_item<DropList<Row>>(2).add_item(drop_item);
     menu->get_item<DropList<Row>>(0).set_attr(240,150);
     menu->get_item<DropList<Row>>(1).set_attr(220,150);
     menu->get_item<DropList<Row>>(2).set_attr(220,40);
     // 操作栏
     std::array<const char*,5> act_icons ={"icons/undo.png","icons/redo.png",
                                     "icons/edit.png","icons/zoom_in.png","icons/zoom_out.png"};
     auto act_bar=make_mglyph<Row>(1500,50,GRAY_ONE);
     flex_mglyph(act_bar,wdw);
     act_bar->center();
     mct->push(act_bar);
     for(auto c: act_icons){
          auto item=make_mglyph<BoxMglyph<Row>>(50,50);
          item->set_bg(GRAY_ONE);
          item->center();
          item->set_margin(LEFT,6);
          item->set_margin(RIGHT,6);
          auto img=make_mglyph<Image>(c,36,36);
          item->push(img);
          addEventHandler(wdw,item,MOUSE_ENTER,
               [](Mwindow&,Row* p){p->set_bg(WHITE_ONE);});
          addEventHandler(wdw,item,MOUSE_QUIT,
               [](Mwindow&,Row* p){p->set_bg(GRAY_ONE);});
          act_bar->push(item);
     }

     // 图片显示区
      auto show_bar=make_mglyph<Row>(1500,900,GRAY_TWO);
      show_bar->center();
      mct->push(show_bar);
      addEventHandler(wdw,RESIZE,[show_bar](Mwindow& wdw){show_bar->resize(wdw.getWidth(),
                                                                    wdw.getHeight()-100);});
     auto prev_icon=make_mglyph<Image>("icons/left.png",120,100);
     auto next_icon=make_mglyph<Image>("icons/right.png",120,100);
     
     
     bgptr->set_margin(TOP,60);
     show_bar->push(prev_icon);
     show_bar->push(bgptr);
     show_bar->push(next_icon);
     run(wdw,mct);
     return 0;
}



