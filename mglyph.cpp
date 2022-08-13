#include "mglyph.h"
#include <stb_image.h>
void Mglyph::pre_draw(float& w,float& h){
        float view[4];
        glGetFloatv(GL_VIEWPORT,view);
        w = view[2]/2.0f;
        h = view[3]/2.0f;
        proj=glm::ortho(-w, w, -h, h);
        glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
        glStencilFunc(GL_ALWAYS,gID,-1);
}
void Rectangle::draw(Mwindow& wdw){
              gen_data();
              wdw.draw(ddata2.data(),proj,8*sizeof(GLfloat),color,z_val);
              
      }

void Rectangle::gen_data(){
    float whalf,hhalf; 
    pre_draw(whalf,hhalf);
    ddata2={origin._X-whalf,             origin._Y+hhalf,            
                 origin._X-whalf + width, origin._Y+hhalf,
                 origin._X-whalf + width, origin._Y+hhalf-height,
                 origin._X-whalf,             origin._Y+hhalf-height}; 
           
}

void Container::add(unsigned index,item_t mg){
        mg->bind_mc(this);
        children.insert(children.begin()+index,mg);
        chilTotalWidth += mg->getWidth();
        chilTotalHeight += mg->getHeight();
        re_layout();
}

void Container::push(item_t mg){
        mg->bind_mc(this);
        chilTotalWidth += mg->getWidth();
        chilTotalHeight += mg->getHeight();
        children.push_back(mg); 
        re_layout();
}

void Container::del(unsigned index){
        if(index >= children.size())
            return; 
        chilTotalWidth -= children[index]->getWidth();
        chilTotalHeight -= children[index]->getHeight();
        children[index]->bind_mc(nullptr);
        children.erase(children.begin()+index);
        re_layout();
}

void Container::pop(){
        if(children.empty())
            return;
        chilTotalWidth -= children.back()->getWidth();
        chilTotalHeight -= children.back()->getHeight();
        children.back()->bind_mc(nullptr);
        children.pop_back();
        re_layout();
}

Image::Image(const char* path):Mglyph() {
              gen_img(path);
             }
Image::Image(const char* path,float w,float h){
              gen_img(path);
              imgw = w;
              imgh = h;
             }
void Image::draw(Mwindow& wdw){
              gen_data();
              wdw.draw(ddata4.data(),proj,16*sizeof(GLfloat),texID,z_val);
             }
void Image::gen_data(){
    float whalf,hhalf; 
    pre_draw(whalf,hhalf);
    ddata4={origin._X-whalf,            origin._Y+hhalf,        0.0,0.0,        
                 origin._X-whalf + imgw, origin._Y+hhalf,        1.0,0.0,
                 origin._X-whalf + imgw, origin._Y+hhalf-imgh,1.0,1.0,
                 origin._X-whalf,            origin._Y+hhalf-imgh, 0.0,1.0};
}
void Image::gen_img(const char* path){ 
    unsigned char* imgd = stbi_load(path, &imgw, &imgh, &imgc, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    auto ccolor = (imgc>3)? GL_RGBA:GL_RGB;
    if(imgd){ 
        //std::cout<<ccolor<<'\n';
       glTexImage2D(GL_TEXTURE_2D, 0,ccolor, imgw, imgh, 0, ccolor, GL_UNSIGNED_BYTE, imgd);
       glGenerateMipmap(GL_TEXTURE_2D);
       stbi_image_free(imgd);
     } 
    else {
       std::cerr<<"load img error:\n";
       std::cout << stbi_failure_reason()<<'\n';
    }
}

const Font& FontGenerater::get_font(const wchar_t wc){
    decltype(auto) rst=fontmap[wc];
    if(rst.texID)
        return rst;
    FT_Set_Pixel_Sizes(face,0,base_sz);
    if (FT_Load_Char(face,wc, FT_LOAD_RENDER)){
       std::cerr<<"load font glyph error\n";
       return rst;
    }
    GLuint texID;
    rst.height=base_sz;
    rst.size.x=face->glyph->bitmap.width;
    rst.size.y=face->glyph->bitmap.rows;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,rst.size.x,
           rst.size.y,0,GL_RED,GL_UNSIGNED_BYTE,
           face->glyph->bitmap.buffer);
    glGenerateMipmap(GL_TEXTURE_2D);
    rst.texID=texID;
    rst.offset.x=face->glyph->bitmap_left;
    rst.offset.y=face->glyph->bitmap_top;
    rst.step=face->glyph->advance.x;
    return rst;
}

FontGenerater::FontGenerater(){
    if(FT_Init_FreeType(&ft)){
       valid=false;
       std::cerr<<"freetype init error\n";
    }
    if(FT_New_Face(ft,FONT_PATH,0,&face)){    // 从文件加载字形数据
       valid=false;
       std::cerr<<"fonts file path error\n";
    }  
}

void FontGenerater::close(){
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

Text::Text(const wchar_t* p,float size):width(0),height(size),
     scale(size/fgr.get_fontsz()){
     addTxt(p);
}
Text::Text(const wchar_t* p,float size,float w,float h):width(w),height(h),
     scale(size/fgr.get_fontsz()){
     for(;*p!=L'\0';++p){
         Font ft=fgr.get_font(*p);
         _str.push_back(*p);
     }
 }
void Text::addTxt(const wchar_t* p){
             float offset=0;
             for(;*p!=L'\0';++p){
                  Font ft=fgr.get_font(*p);
                  _str.push_back(*p);
                  width += (ft.step >> 6)*scale;
                  auto newh=(ft.size.y-ft.offset.y)*scale;
                  offset=offset>newh? offset:newh;
                 
                 }
            height+=offset;
             if(pa ) pa->re_layout();
}
void Text::draw(Mwindow& wdw){
            //Rectangle::draw(wdw);
            float posX=origin._X;
            for(auto wc:_str){
                auto ft= fgr.get_font(wc);
                gen_data(posX,origin._Y,ft);
                wdw.draw(ddata4.data(),proj,16*sizeof(GLfloat),ft.texID,foncol,z_val);
                posX += (ft.step >> 6)*scale; 
            }
}   
void Text::gen_data(float posX,float posY,Font& ft){
    float whalf,hhalf; 
    pre_draw(whalf,hhalf);
    float xpos = posX + ft.offset.x*scale - whalf;
    float ypos = posY + ft.offset.y*scale + hhalf - ft.height*scale;
    ddata4={xpos,                          ypos,                        0.0, 0.0,          
                 xpos + ft.size.x*scale, ypos,                        1.0, 0.0,
                 xpos + ft.size.x*scale, ypos-ft.size.y*scale,  1.0, 1.0,
                 xpos,                          ypos-ft.size.y*scale,  0.0, 1.0};
}

void Container::draw(Mwindow& wdw){
        
       if(need_layout){
             layout->call(*this);
             need_layout=false;
         }
       Rectangle::draw(wdw); 
       for(auto& m:children){
                m->draw(wdw);
         } 
}

void ViewGlyph::draw(Mwindow& wdw){
        wdw.setView(origin._X,origin._Y,width,height);
        custom_draw(wdw);
        wdw.setView(0,0,wdw.getWidth(),wdw.getHeight());
}

void alignHelperH(Container& mc,float offsetx,float startY){
     float limit=offsetx+mc.getWidth();
     if(mc.getPadding(TOP)){
        float offsety = startY - mc.getPadding(TOP);
        for(auto m:mc){
            if(offsetx+m->getWidth()>limit) offsetx=AL_NUM;
            m->setPos(offsetx,offsety);
            offsetx += m->getWidth();
          }
      }    
     else if(mc.getPadding(BOTTOM)){
        float offsety = startY - mc.getHeight() + mc.getPadding(BOTTOM);
        for(auto m:mc){
            if(offsetx+m->getWidth()>limit) offsetx=AL_NUM;
            m->setPos(offsetx,offsety + m->getHeight());
            offsetx += m->getWidth();
          }
      }
     else { 
        float offsety = startY - mc.getHeight()/2;
        for(auto m:mc){
            if(offsetx+m->getWidth()>limit) offsetx=AL_NUM;
            m->setPos(offsetx,offsety + m->getHeight()/2);
            offsetx += m->getWidth();
          }
      }
}


void alignHelperV(Container& mc,float offsety,float startX){
     float limit=offsety-mc.getHeight();
     if(mc.getPadding(LEFT)){
        float offsetx = startX + mc.getPadding(LEFT);
        for(auto m:mc){
           if(offsety-m->getHeight()<limit) offsety=-AL_NUM;
           m->setPos(offsetx,offsety);
           offsety -= (m->getHeight());
          }
      }    
     else if(mc.getPadding(RIGHT)){
        float offsetx = startX + mc.getWidth() - mc.getPadding(RIGHT);
        for(auto m:mc){
            if(offsety-m->getHeight()<limit) offsety=-AL_NUM;
            m->setPos(offsetx-m->getWidth(),offsety);
            offsety -= (m->getHeight());
          }
      }
     else { 
        float offsetx = startX + mc.getWidth()/2;
        for(auto m:mc){
            if(offsety-m->getHeight()<limit) offsety=-AL_NUM;
            m->setPos(offsetx - m->getWidth()/2,offsety);
            offsety -= (m->getHeight());
        }
      }
}
void alignLeft(Container& mc){
     auto [posX,posY]=mc.getPos();
     alignHelperH(mc,posX + mc.getPadding(LEFT),posY);
}

void alignRight(Container& mc){
     auto [posX,posY]=mc.getPos();
     alignHelperH(mc,posX + mc.getWidth() - mc.getPadding(RIGHT)-mc.totalWidth(),posY);
}

void alignCenterH(Container& mc){
     auto [posX,posY]=mc.getPos();
     alignHelperH(mc,posX + (mc.getWidth()-mc.totalWidth()-mc.getPadding(RIGHT))/2 + mc.getPadding(LEFT)/2,posY);
}

void alignTop(Container& mc){
     auto [posX,posY]=mc.getPos();
     alignHelperV(mc,posY - mc.getPadding(TOP),posX);
}

void alignBottom(Container& mc){
     auto [posX,posY]=mc.getPos();
     alignHelperV(mc,posY - mc.getHeight() + mc.getPadding(BOTTOM) + mc.totalHeight(),posX);
}

void alignCenterV(Container& mc){
     auto [posX,posY]=mc.getPos();
     alignHelperV(mc,posY - (mc.getHeight()-mc.totalHeight()-mc.getPadding(BOTTOM))/2 - mc.getPadding(TOP)/2,posX);
}

