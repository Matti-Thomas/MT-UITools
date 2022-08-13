#include <cuchar>
#include "mprogram.h"
#include "mglyph.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//glfw文档要求的回调函数类型，程序的事件处理使用其进行中转
struct GLFW_CB{
static void cb_fb_resize(GLFWwindow* wdw,int w,int h){
       Mwindow* mwdw = (Mwindow*) glfwGetWindowUserPointer(wdw);
       auto& event_mg=MeventManager::instance();
       glViewport(0,0,w,h); 
       mwdw->width=w;
       mwdw->height=h;
       int n=event_mg.get_count(RESIZE);
       for(int i=0;i!=n;++i)
             event_mg.receive(mhash(mwdw->get_id(),RESIZE,i,0,0));
}
static void cb_win_close(GLFWwindow* wdw){}
static void cb_mouse_on(GLFWwindow* wdw,double xpos,double ypos){
       static unsigned char last=0;
       Mwindow* mwdw = (Mwindow*) glfwGetWindowUserPointer(wdw);
       auto& event_mg=MeventManager::instance();
       int width,height;
       glfwGetFramebufferSize(wdw,&width,&height);
       unsigned index;
       size_t signal;
       glReadPixels(xpos,height-ypos,1,1,GL_STENCIL_INDEX,GL_UNSIGNED_INT,&index); 
       if(index && GLFW_PRESS==glfwGetMouseButton(wdw,0)){
             signal=mhash(mwdw->get_id(),0,0,index,CURSOR_DRAG);
             event_mg.receive(signal);
       }
       if(index!=last){
                if(last){
                    signal = mhash(mwdw->get_id(),0,0,last,MOUSE_QUIT);
                    event_mg.receive(signal);
                    signal = mhash(mwdw->get_id(),0,0,last,CURSOR_DRAG_RESET);
                    event_mg.receive(signal);
                }     
               signal = mhash(mwdw->get_id(),0,0,index,MOUSE_ENTER);
               event_mg.receive(signal);
               last = index; 
        }
       
}
  
static void cb_mouse_click(GLFWwindow* wdw,int btn,int act,int mods){
       Mwindow* mwdw = (Mwindow*) glfwGetWindowUserPointer(wdw);
       auto& event_mg=MeventManager::instance();
       static size_t last=0;
       static unsigned char lastobj=0;
       if(act==GLFW_RELEASE)
              return;
       int width,height;
       double xpos,ypos;
       glfwGetFramebufferSize(wdw,&width,&height);
       glfwGetCursorPos(wdw,&xpos,&ypos);
       unsigned index;
       glReadPixels(xpos,height-ypos,1,1,GL_STENCIL_INDEX,GL_UNSIGNED_INT,&index);
       if(last ){
             event_mg.receive(last);
             last=0;
         }    
       if(index==lastobj){
             lastobj=0;
             return;
         }
       auto signal = mhash(mwdw->get_id(),0,0,index,act*10+btn);
       if(event_mg.receive(signal))
             last = signal+2;
       lastobj=index;         
}

static void cb_key(GLFWwindow* wdw,int key,int scode,int act,int mods){
    Mwindow* mwdw = (Mwindow*) glfwGetWindowUserPointer(wdw);
    auto& keys=mwdw->win_input->key_in;
    auto& event_mg=MeventManager::instance();
    keys[0]=key;
    keys[1]=mods;
    keys[2]=act;
    int n=event_mg.get_count(KEYBORD);
       for(int i=0;i!=n;++i)
             event_mg.receive(mhash(mwdw->get_id(),KEYBORD,i,0,0));
}

static void cb_scroll(GLFWwindow* wdw,double xoffset,double yoffset){
    Mwindow* mwdw = (Mwindow*) glfwGetWindowUserPointer(wdw);
    auto& scroll=mwdw->win_input->scroll;
    auto& event_mg=MeventManager::instance();
    scroll._X=xoffset;
    scroll._Y=yoffset;
    int n=event_mg.get_count(SCROLL);
    for(int i=0;i!=n;++i)
            event_mg.receive(mhash(mwdw->get_id(),SCROLL,i,0,0));
}
static void cb_file_drop(GLFWwindow* wdw, int count,const char** paths){
    Mwindow* mwdw = (Mwindow*) glfwGetWindowUserPointer(wdw);
    auto& files=mwdw->win_input->in_file;
    auto& event_mg=MeventManager::instance();
    for(int i=0;i!=count;++i)
        files.push_back(*(paths+i));
    int n=event_mg.get_count(FILE_DROP);
    for(int i=0;i!=n;++i)
            event_mg.receive(mhash(mwdw->get_id(),FILE_DROP,i,0,0));
    files.clear();
}
static void cb_char_input(GLFWwindow* wdw,unsigned codepoint){
    Mwindow* mwdw = (Mwindow*) glfwGetWindowUserPointer(wdw);
    auto& char_in=mwdw->win_input->char_in;
    auto& event_mg=MeventManager::instance();
    char tmp[MB_LEN_MAX]={'\0'};
    std::mbstate_t state{};
    std::c32rtomb(tmp,codepoint, &state);
    char_in=tmp;
    std::cout<<mwdw->win_input->char_in<<std::endl;
    int n=event_mg.get_count(INPUT);
    for(int i=0;i!=n;++i)
            event_mg.receive(mhash(mwdw->get_id(),INPUT,i,0,0));
}
};
// GLprogram类的具体实现
GLprogram::GLprogram(const char* vertCode, const char* fragCode){
    
    compiled(vertCode,fragCode);
}

void GLprogram::use(){
    glUseProgram(id);
}

void GLprogram::initAttri(const char* attr,int n,int step,int offset){
    GLint loc = glGetAttribLocation(id, attr);
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, n, GL_FLOAT, GL_FALSE,
                          sizeof(float) * step, (void*)(sizeof(float)*offset)); 
}

void GLprogram::setUniform(const char* attr, int val) const{ 
    glUniform1i(glGetUniformLocation(id, attr), val); 
}  

void GLprogram::setUniform(const char* attr, float val) const{ 
    glUniform1f(glGetUniformLocation(id, attr), val); 
}

void GLprogram::setUniform(const char* attr, int x,int y,int z) const{ 
    glUniform3i(glGetUniformLocation(id, attr), x,y,z); 
}  

void GLprogram::setUniform(const char* attr,float x,float y,float z) const{ 
    glUniform3f(glGetUniformLocation(id, attr),x,y,z); 
} 
   
void GLprogram::setUniform(const char* attr, const float* val) const{ 
    glUniformMatrix4fv(glGetUniformLocation(id,attr), 1, GL_FALSE, val); 
}  

void GLprogram::compiled(const char* vertCode, const char* fragCode){
    GLint status;
    vertSha = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertSha, 1, &vertCode, NULL);
    glCompileShader(vertSha);
     // 检测着色器是否正确编译
    glGetShaderiv(vertSha,GL_COMPILE_STATUS,&status);
    if(status != GL_TRUE){
         char log[512];
         glGetShaderInfoLog(vertSha,512,NULL,log);
         errLog=log;
         valid=false; 
         return;
     }
    // 编译片段着色器，基本同上
    fragSha = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragSha, 1, &fragCode, NULL);
    glCompileShader(fragSha);
    glGetShaderiv(fragSha,GL_COMPILE_STATUS,&status);
    if(status != GL_TRUE){
         char log[512];
         glGetShaderInfoLog(fragSha,512,NULL,log);
         errLog=log;
         valid=false;
         return;
     }
    id = glCreateProgram();
    glAttachShader(id, vertSha);
    glAttachShader(id, fragSha);
    glLinkProgram(id);
    glDeleteShader(vertSha);
    glDeleteShader(fragSha);
    glGetProgramiv(id, GL_LINK_STATUS, &status);
    if(status != GL_TRUE){
         char log[512];
         glGetProgramInfoLog(id,512,NULL,log);
         errLog=log;
         valid=false;
     }
    
     
}     



void envirInital(){
    if(!glfwInit()){
        std::cerr<<"GLFW init error"<<'\n';
        return ;
     }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    std::cout<<"Initial setting ok"<<std::endl;
}


// 窗口类的具体实现
class Mwindow::BaseDrawer{
public:
     BaseDrawer(){
        glGenVertexArrays(2, vertArrayBuff);
        glGenBuffers(2, vertBuffs);
        glGenBuffers(1, &elemBuff);
        glBindVertexArray(vertArrayBuff[0]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,elemBuff);
        GLuint elem[6]={0,1,2,0,2,3};
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elem), elem, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, vertBuffs[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16, NULL, GL_DYNAMIC_DRAW);
        texprogram.initAttri("vPos",2,4,0);
        texprogram.initAttri("vTex",2,4,2);
        fonprogram.initAttri("vPos",2,4,0);
        fonprogram.initAttri("vTex",2,4,2);
        glBindVertexArray(vertArrayBuff[1]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,elemBuff);
        glBindBuffer(GL_ARRAY_BUFFER, vertBuffs[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, NULL, GL_DYNAMIC_DRAW);
        comprogram.initAttri("vPos",2,2,0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
     }
     ~BaseDrawer(){}
    void draw(GLfloat* vertices,glm::mat4&fproj,std::size_t sz,vec3f& color,float z){
        
        glBindVertexArray(vertArrayBuff[1]);
        glBindBuffer(GL_ARRAY_BUFFER, vertBuffs[1]);
        comprogram.use(); // 激活程序，使用着色器
        comprogram.setUniform("projection",glm::value_ptr(fproj));
        comprogram.setUniform("bgcol", color.x, color.y, color.z);
        comprogram.setUniform("zval", z);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sz, vertices);
        glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT ,0);
    }
    void draw(GLfloat* vertices,glm::mat4&fproj,std::size_t sz,int texID,vec3f& color,float z){
        glBindVertexArray(vertArrayBuff[0]);
        glBindBuffer(GL_ARRAY_BUFFER, vertBuffs[0]);
        fonprogram.use(); // 激活程序，使用着色器
        fonprogram.setUniform("projection",glm::value_ptr(fproj));
        fonprogram.setUniform("foncol", color.x, color.y, color.z);
        fonprogram.setUniform("zval", z);
        glBindTexture(GL_TEXTURE_2D,texID);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sz, vertices);
        glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT ,0);
    }
    void draw(GLfloat* vertices,glm::mat4&fproj,std::size_t sz,int texID,float z){
        glBindVertexArray(vertArrayBuff[0]);
        glBindBuffer(GL_ARRAY_BUFFER, vertBuffs[0]);
        texprogram.use(); // 激活程序，使用着色器
        texprogram.setUniform("projection",glm::value_ptr(fproj));
        texprogram.setUniform("zval",z);
        glBindTexture(GL_TEXTURE_2D,texID);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sz, vertices);
        glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT ,0);
    } 
private:
GLprogram comprogram{VERT_T.c_str(),FRAG_T.c_str()};
GLprogram texprogram{TEX_VERT_T.c_str(),TEX_FRAG_T.c_str()};
GLprogram fonprogram{TEX_VERT_T.c_str(),FONT_FRAG_T.c_str()};
GLuint vertBuffs[2],elemBuff;
GLuint vertArrayBuff[2];
//着色器程序的代码原始文本，用于生成基本着色器
inline static const  std::string TEX_VERT_T = R"(
#version 130 
in vec2 vPos;
in vec2 vTex;
uniform mat4 projection;
out vec2 tex;
void main(){
    gl_Position = projection*vec4(vPos, 0., 1.0);
    tex = vTex;
})";
inline static const std::string  VERT_T  = R"(
#version 130
in vec2 vPos;
uniform mat4 projection;
void main(){
gl_Position = projection*vec4(vPos, 0., 1.0);
})";

inline static const std::string  FRAG_T =R"(
#version 130
out vec4 col;
uniform sampler2D te;
uniform vec3 bgcol;
uniform float zval;
void main(){
    col=vec4(bgcol,1.0f);
    gl_FragDepth=zval;
})";

inline static const std::string  TEX_FRAG_T =R"(
#version 130
in vec2 tex;
out vec4 col;
uniform sampler2D te;
uniform float zval;
void main(){
       col=texture(te,tex);
       gl_FragDepth=zval;
})";

inline static const std::string FONT_FRAG_T =R"(
#version 130
in vec2 tex;
out vec4 col;
uniform sampler2D te;
uniform vec3 foncol;
uniform float zval;
void main(){
    col=vec4(foncol,texture(te,tex).r);
    gl_FragDepth=zval;
})";
};



Mwindow::Mwindow(const char* name,int xpos,int ypos,int w,int h):
    posX(xpos),posY(ypos),width(w),height(h){
       _window = glfwCreateWindow(width, height,name, NULL, NULL); 
      _init();
}
Mwindow::~Mwindow(){
    glfwTerminate();
}
void Mwindow::_init(){
        if(!_window){
            std::cerr<<"window create error"<<"\n";
            valid=false;
            return;
        }
       
        glfwSetCursorPosCallback(_window, GLFW_CB::cb_mouse_on);
        glfwSetMouseButtonCallback(_window, GLFW_CB::cb_mouse_click);
        glfwSetWindowCloseCallback(_window, GLFW_CB::cb_win_close);
        glfwSetFramebufferSizeCallback(_window, GLFW_CB::cb_fb_resize);
        glfwMakeContextCurrent(_window);
        glfwSwapInterval(0);
        
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
             std::cout << "Failed to initialize GLAD" << std::endl;
             return;
        }

        std::cout<<"OpenGL版本："<<glGetString(GL_VERSION)<<"\n";
        std::cout<<"着色器语言版本："<<glGetString(GL_SHADING_LANGUAGE_VERSION)<<"\n";
        std::cout<<"渲染器/GPU："<<glGetString(GL_RENDERER)<<"\n";
        bd_ptr=std::make_unique<BaseDrawer>();
        glfwSetWindowUserPointer(_window,this);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_STENCIL_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        winID=++countID;
}

Mwindow&  Mwindow::setIcon(const char* path){
        GLFWimage img;
        img.pixels = stbi_load(path, &img.width, &img.height,0, 4);
        if(img.pixels){
                glfwSetWindowIcon(_window,1,&img);
                 stbi_image_free(img.pixels);
        }        
        return *this;
}
void Mwindow::listen(MwinEvent e){
            switch(e){
                case KEYBORD:
                    glfwSetKeyCallback(_window,GLFW_CB::cb_key);
                    break;
                case SCROLL:
                    glfwSetScrollCallback(_window, GLFW_CB::cb_scroll);
                    break;
                case FILE_DROP:
                    glfwSetDropCallback(_window, GLFW_CB::cb_file_drop);
                    break;
                case INPUT:
                glfwSetCharCallback(_window, GLFW_CB::cb_char_input);
                break;
            }

}

void Mwindow::disable_listen(MwinEvent e){
            switch(e){
                case KEYBORD:
                    glfwSetKeyCallback(_window,NULL);
                    break;
                case SCROLL:
                    glfwSetScrollCallback(_window, NULL);
                    break;
                case FILE_DROP:
                    glfwSetDropCallback(_window, NULL);
                    break;
                case INPUT:
                glfwSetCharCallback(_window, NULL);
                break;
            }

}

void Mwindow::setView(int xpos,int ypos,int w,int h){
     glViewport(xpos,height+ypos-h,w,h);
}
void Mwindow::refresh(){
        glfwSwapBuffers(_window);
        glClear(GL_DEPTH_BUFFER_BIT);
        
}

    void Mwindow::draw( GLfloat* vertices,glm::mat4&fproj,std::size_t sz,vec3f& color,float z){
        bd_ptr->draw(vertices,fproj,sz,color,z);
    }
    void Mwindow::draw( GLfloat* vertices,glm::mat4&fproj,std::size_t sz,int texID,float z){
       bd_ptr->draw(vertices,fproj,sz,texID,z);
    }
    void Mwindow::draw( GLfloat* vertices,glm::mat4&fproj,std::size_t sz,int texID,vec3f& color,float z){
        bd_ptr->draw(vertices,fproj,sz,texID,color,z);
    }
    


void wait_event(){
    glfwWaitEvents();
}
Pos2D<double> getCursorPos(){
    Pos2D<double> csp;
    glfwGetCursorPos(glfwGetCurrentContext(),&csp._X,&csp._Y);
    return csp;
}
Pos2D<int> getWindowPos(){
    Pos2D<int> wdp;
    glfwGetWindowPos(glfwGetCurrentContext(), &wdp._X, &wdp._Y);
    return wdp;
}
