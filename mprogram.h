#ifndef FONT_PATH
#define FONT_PATH "/usr/share/fonts/wqy-microhei/wqy-microhei.ttc"
#endif
#ifndef MPROGRAM_H
#define MPROGRAM_H
#include <utility>
#include <type_traits>
#include <map>
#include <array>
#include <glad/glad.h>
#include <glfw3.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <string>
#include <iostream>
#include "mevents.h"
#define vec3f glm::vec3 
#define vec2i glm::ivec2


// 判断两类型是否相同
template<typename T, typename U>  // 主模板
struct is_same_type {bool val=false;};
template<typename T>             // 偏特化
struct is_same_type<T, T>{bool val=true;};
template<typename T,typename U>          // 别名 
inline constexpr bool same_type=is_same_type<T,U>::val;
template<typename T>
struct Pos2D{
    T _X;
    T _Y;
};

inline size_t mhash(size_t winID,size_t win_ev,size_t key_btn,unsigned objID, unsigned mouse_btn){
        return (winID<<27)+(win_ev<<23)+(key_btn<<14)+(objID<<6)+mouse_btn;
}

class GLprogram{
public:
    GLprogram(const char* vertCode, const char* fragCode); 
    // 获取着色器程序ID
    unsigned int getid(){
        return id;
     }
    // 着色器是否可用
    bool isValid(){
        return valid;
     }
    // 激活程序
    void use();
    // 初始化属性数据信息
    void initAttri(const char*,int,int,int);
    // 设置着色器uniform数据的重载工具函数 
    void setUniform(const char* , int) const;
    void setUniform(const char* , float) const;
    void setUniform(const char* , int,int,int) const;
    void setUniform(const char* , float,float,float) const;
    void setUniform(const char* , const float*) const;
    std::string errLog;  //着色器的错误记录
private:
    GLuint id;
    GLuint vertSha;
    GLuint fragSha;
    bool valid=true;
    void compiled(const char* vertCode, const char* fragCode);
};

struct Input_info{
int key_in[3];
Pos2D<double> scroll;
std::string char_in;
std::vector<const char*> in_file;
};
// 初始化环境
void envirInital();

class Mwindow{
public:
    friend struct GLFW_CB;
    Mwindow(const char* name,int xpos,int ypos,int w,int h);
    ~Mwindow();
    
    void freeze(){
        glfwSetWindowAttrib(_window,GLFW_RESIZABLE,false);
     }
     void resize(int w,int h){
        if(GLFW_RESIZABLE){
            glfwSetWindowSize (_window, w, h);
            glfwGetWindowSize (_window, &width, &height);
        }
     }
    void setCloseFlag(bool val){
       glfwSetWindowShouldClose(_window,val);
     }
    void minimize(){
       glfwIconifyWindow(_window);
     }
    void maximize(){
       glfwMaximizeWindow(_window);
     }
    void restore(bool isFull=false){
        if(!isFull )
             glfwRestoreWindow(_window);
        else
           glfwSetWindowMonitor(_window,NULL,0,0,width,height,0);
     }
     void fullMode(){
         auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
         glfwSetWindowMonitor(_window,glfwGetPrimaryMonitor(),0,0,mode->width,mode->height,mode->refreshRate);
     }    
    bool isValid(){
        return valid; 
     }
   // void bindContent(Mcontainer* mc); 
    void setView(int xpos,int ypos,int w,int h);
    void refresh();

    int getWidth(){
         return width;
     }
    int getHeight(){
         return height;
     }
    unsigned get_id(){
        return winID;
    }
    int get_key(unsigned idx){
        return win_input->key_in[idx];
    }
    Pos2D<double> get_scroll(){
        return win_input->scroll;
    }
    const char* get_drop_file(){
        return win_input->in_file.back();
    }
    std::string get_input(){
        return win_input->char_in;
    }
     Mwindow& setSizeLimit(int minW,int minH,int maxW,int maxH){
        glfwSetWindowSizeLimits(_window,minW,minH,maxW,maxH);
        return *this;
    }
    Mwindow& setIcon(const char* path);
    void hideDecor(){
        glfwSetWindowAttrib(_window,GLFW_DECORATED,false);
    }
    void listen(MwinEvent e);
    void disable_listen(MwinEvent e);
    bool isClosed(){
        return glfwWindowShouldClose(_window);
     }
    void use(){
        glfwMakeContextCurrent(_window);
    }
    void draw( GLfloat*,glm::mat4&, std::size_t,int,float);
    void draw( GLfloat*,glm::mat4&,std::size_t,vec3f&,float);
    void draw( GLfloat*,glm::mat4&,std::size_t,int,vec3f&,float);

private:
    int posX,posY;
    int width,height;
    unsigned winID;
    std::unique_ptr<Input_info> win_input{std::make_unique<Input_info>()};
    class BaseDrawer;
    std::unique_ptr<BaseDrawer> bd_ptr;
    GLFWwindow* _window{nullptr};
    bool valid=true;
    void _init();
    inline static unsigned countID=0;   
};
void wait_event();
Pos2D<double> getCursorPos();
Pos2D<int> getWindowPos();

#endif




