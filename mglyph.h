#ifndef MGLYPH_H
#define MGLYPH_H
#include "mprogram.h"
constexpr int M_NUM=10;
constexpr int AL_NUM=10000;
enum Direction {TOP=0,BOTTOM,LEFT,RIGHT,LEFT_TOP,
               RIGHT_TOP,LEFT_BOTTOM,RIGHT_BOTTOM};

class Container;   // Container类的提前声明，使其对Mglyph可见
// 基本图元的抽象概念，最小绘图单元
class Mglyph {
    public:
        Mglyph(){                   //抽象基类一般不需要构造函数和数据成员，但为了后续
           gID=++objCount;     // 设计方便，因此加上,
        }
        virtual void draw(Mwindow&)=0;
        virtual ~Mglyph(){}
        virtual float getWidth()=0;
        virtual float getHeight()=0;
        virtual void setPos(float x,float y){  // 设置图元的二维坐标,屏幕左上角为原点(0,0)
                origin._X=x;                //设为虚函数是因为不同图元的坐标可能有不同的限制因素
                origin._Y=y;               
        }
        unsigned get_id(){
                return gID;
        }
        Pos2D<float> getPos(){
                return origin;
        }
        void bind_mc(Container* mc){
                pa=mc;
        }
        void pre_draw(float&,float&);  // 预处理渲染数据
        void set_depth(float val){
             z_val=val;
        }
        bool has_handler(McurEvent e){
            return  event & (1<<(e-M_NUM));
        }
        void mark_handler(McurEvent e){
            event=event | (1<<(e-M_NUM));
        }
        void clear_handler(McurEvent e){
            event=event &(~(1<<(e-M_NUM)));
        }
    protected:
        Pos2D<float> origin{0,0};
        inline static glm::mat4 proj;                            //  正交投影矩阵
        float z_val=0.9;             // Z轴上的深度,用于两个图元重叠时显示的顺序
        unsigned gID;
        unsigned char event{0};     // 用来记录cursor事件处理 
        Container* pa;    //  图元所属的容器
    private:
        
        inline static unsigned objCount=0;                   //用于分配图元标识        
}; 


class Rectangle:public Mglyph { 
     public:
          Rectangle(float w,float h):Mglyph(),width(w),height(h){}
          Rectangle(float w,float h,const vec3f& co): Mglyph(),width(w),height(h),color(co){}
          void draw(Mwindow& wdw) override;
          float getWidth() override {
            return width;
          }
          float getHeight() override {
            return height;
          }
          void set_bg(const vec3f& co){
             color=co;
          } 
          void gen_data();
          
     protected:
          float width;
          float height;
     private:
         vec3f color{0,0,0};
         inline static std::array<GLfloat,8> ddata2;
};

class Container:public Rectangle {
    public:
        using item_t=std::shared_ptr<Mglyph>;
        Container(float w,float h): Rectangle(w,h){
        }
        Container(float w,float h,const vec3f& co): Rectangle(w,h,co){
        }
        void draw(Mwindow& wdw) override;
        void add(unsigned index,item_t mg);
        void push(item_t mg);
        void del(unsigned index);
        void pop();
        void re_layout(){
            if(!need_layout)
                    need_layout=true;
        }
        void setPos(float x,float y) override{
            Mglyph::setPos(x,y);
            re_layout();
        }   
        float totalWidth(){
                return chilTotalWidth;
        }
        float totalHeight(){
                return chilTotalHeight;
        }
        void resize(float w,float h){
               width=w;
               height=h;
               re_layout();
        }
        auto begin(){
                return children.begin();
        }
        auto end(){
               return children.end();
        }
        float getPadding(Direction d){
                return padding[d];
        }
        void setPadding(Direction d,float newVal){
                padding[d]=newVal;
                re_layout();
        }
        template<typename T>
        T& get_item(unsigned index){
                auto mg = children.at(index);
                return *(std::dynamic_pointer_cast<T,Mglyph>(mg));
        }
        template<typename F>
        void set_layout(F func ){
             layout=std::make_unique<LayoutFunc<F>>(func);
        }
    private:
        struct LayoutBase{
            void virtual call(Container&)=0;
             ~LayoutBase()=default; 
        };
        template<typename T>
        class LayoutFunc:public LayoutBase{
            T func;
        public:
            LayoutFunc(T t):func(t){}
            void call(Container& c) override{
                func(c);
            }
        } ;
        std::unique_ptr<LayoutBase> layout;  
        float padding[4]{0};
        std::vector<std::shared_ptr<Mglyph>> children;
        bool need_layout{false};
    protected:
        float chilTotalWidth{0};
        float chilTotalHeight{0};
};

void alignLeft(Container&);
void alignRight(Container&);
void alignCenterH(Container&);
void alignTop(Container&);
void alignBottom(Container&);
void alignCenterV(Container&);
class Image:public Mglyph {
     public:
          Image(const char* path);
          Image(const char* path,float w,float h);
          Image(const Image&)=delete;
          ~Image(){
               glDeleteTextures(1,&texID);
          }
          void draw(Mwindow& wdw) override;
          float getWidth() override {
              return imgw;
             }
          float getHeight() override {
              return imgh;
             }
          void resize(float w,float h){
               imgw=w;
               imgh=h;
               if(pa) pa->re_layout();
             }
            void gen_img(const char*);
            void gen_data();
     private:
          //Imgdata data;
            int   imgh,imgw;   // 图片高宽
            int   imgc;           // 图片通道数
            GLuint   texID;          // 图片纹理的ID
            inline static std::array<GLfloat,16> ddata4;
};

struct Font{
    GLuint     texID;       // 字形纹理的ID
    int          height;     // 字形位图高度   
    vec2i      size;        // 字形大小
    vec2i      offset;     // 从基准线到字形左部/顶部的偏移值
    int   step;             // 原点距下一个字形原点的距离
};
class FontGenerater{
public:
    FontGenerater(FontGenerater&)=delete;
    static FontGenerater& instance(){
         static FontGenerater ftg;
         return ftg;
    }
    void close();
    int get_fontsz(){
         return base_sz;
    }
    const Font& get_font(const wchar_t);
    bool isValid(){
         return valid;
     }

private:
    FontGenerater();
    FT_Library ft;
    FT_Face face;
    inline static int base_sz=36; 
    bool valid=true;
    std::map<wchar_t,Font> fontmap;
    
};
//文本图元不特殊处理换行字符
class Text:public Mglyph {
     public:
          Text(const wchar_t*,float);
          Text(const wchar_t*,float,float,float);
          void addTxt(const wchar_t* p);
          void draw(Mwindow& wdw) override;
          void set_color(const vec3f& co){
             foncol=co;
          } 
          void gen_data(float posX,float posY,Font& ft);
        float getWidth() override {
            return width;
        }
        float getHeight() override {
            return height;
        }
        vec3f foncol{0,0,0};      
     private:
          float width,height;
          float scale{1};
          
          std::vector<wchar_t> _str;
          FontGenerater& fgr{FontGenerater::instance()};
          inline static std::array<GLfloat,16> ddata4;
};

class ViewGlyph:public Mglyph {
     public:
          ViewGlyph(float w,float h):width(w),
          height(h){}
          void draw(Mwindow& wdw) override;
          float getWidth() override {
              return width;
             }
          float getHeight() override {
              return height;
             }
          void resize(float w,float h){
               width=w;
               height=h;
               if(pa ) pa->re_layout();
             }
            virtual void custom_draw(Mwindow& wdw ){
                if(_source)
                    _source->draw(wdw);
            }
          void bind_mg(std::shared_ptr<Mglyph> m){
             _source=m;
             float xpos=m->getWidth()<width? (width-m->getWidth())/2:0;
             float ypos=m->getHeight()<height? -(height-m->getHeight())/2:0;
             m->setPos(xpos,ypos);
          }
     protected:
          float width,height;
          std::shared_ptr<Mglyph> _source;

};

class Row:public Container{
public:
    Row(float w,float h):Container(w,h){
        set_layout(alignLeft);
    }
    Row(float w,float h,const vec3f& co):Container(w,h,co){
        set_layout(alignLeft);
    }
    void reverse(){
         set_layout(alignRight);
    } 
    void center(){
        set_layout(alignCenterH);
    }
};

class Column:public Container{
public:
    Column(float w,float h):Container(w,h){
        set_layout(alignTop);
    }
    Column(float w,float h,const vec3f& co):Container(w,h,co){
        set_layout(alignTop);
    }
    void reverse(){
         set_layout(alignBottom);
    } 
    void center(){
        set_layout(alignCenterV);
    }
};
// 添加margin的图元,用于布局，参考web前端css3的盒子模型
template<typename T>
class BoxMglyph:public T{
    public:
        template<typename... Args>
         BoxMglyph(Args&&... args):T(std::forward<Args>(args)...){}
         float getWidth() override{
            return T::getWidth()+margin[LEFT]+margin[RIGHT];
         }
         float getHeight() override{
            return T::getHeight()+margin[TOP]+margin[BOTTOM];
         }
        
         void setPos(float x,float y) override{
               T::setPos(x,y);
               T::origin._X+=margin[LEFT];
               T::origin._Y-=margin[TOP];  
         }
       
         void set_margin(Direction d,float val){                        //与图元的间距
            margin[d]=val;
            if(T::pa) T::pa->re_layout();
         }
    private:
         float margin[4]{0};
};



#endif




