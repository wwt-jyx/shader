#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"
#include "camera.h"

#include <string>
#include <vector>
using namespace std;

enum ArrayType {
    ARRAY_VERTEX = 0,
    ARRAY_NORMAL = 1,
    ARRAY_TANGENT = 2,
    ARRAY_COLOR = 3,
    ARRAY_TEX_UV = 4,
    ARRAY_TEX_UV2 = 5,
    ARRAY_BONES = 6,
    ARRAY_WEIGHTS = 7,
    ARRAY_INDEX = 8,
    ARRAY_MAX = 9
};

struct LayoutItem {
    uint32_t buffer_indx;
    void* buffer_ptr;
    uint32_t buffer_byteLength;
    uint32_t strip;   // same in all items, size of the item in byte两个数据元素之间的字节距离
    ArrayType type;
    GLenum e_format;    // type of component in one item 数据类型
    GLint e_size;     // size of one component in byte  单数据占比特大小
    uint16_t e_count; // component count in one item  数据分量的基础类型
    uint32_t e_num;      //数据元素的数量
    GLenum normalized = GL_FALSE;
    GLint offset;     // 数据偏移
    void* data_ptr;  //数据起始位置（指定buffer起始位置+偏移量）
    bool valid = true;
    LayoutItem(uint16_t strip, ArrayType type, GLenum e_format, GLint e_size,
               uint16_t e_count, bool normalized, GLint offset, void* data_ptr) : strip(strip), type(type),
                                                                                  e_format(e_format), e_size(e_size), e_count(e_count), normalized(normalized), offset(offset), data_ptr(data_ptr) {}
    LayoutItem() = default;

    uint32_t GetByteLength(){return e_size*e_count*e_num;};
};



unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);


struct Layout {
    std::vector<LayoutItem> _data;
    Layout() = default;
    Layout(const std::vector<LayoutItem> data) :_data(data) {
        // fix strip and offset
        uint16_t curr_offset = 0;
        for(auto i = _data.begin();i<_data.end();i++) {
            i->offset = curr_offset;
            curr_offset += i->e_size * i->e_count;
        }
        for(auto i = _data.begin();i<_data.end();i++)
            i->strip = curr_offset;
    }//??

    Layout(const std::initializer_list<LayoutItem> item_list) : Layout(std::vector<LayoutItem>(item_list)) {}
    // TODO
    void emplace_back(const ArrayType& type) {}
    void emplace_back(ArrayType type, GLenum e_format, GLint e_size,
                      uint16_t e_count, bool normalized) {
        uint16_t newStrip = 0;
        uint16_t currOffset = 0;
        if (_data.size() == 0)
            newStrip = e_size * e_count;
        else {
            newStrip = _data.back().strip + e_size * e_count;
            currOffset = _data.back().strip;
            for (auto& i : _data)
                i.strip = newStrip;
        }
        _data.emplace_back(newStrip, type, e_format, e_size, e_count, normalized,
                           currOffset, nullptr);
    }
    void push_back(const LayoutItem& l) {
        uint16_t newStrip = 0;
        uint16_t currOffset = 0;
        if (_data.size() == 0)
            newStrip = l.e_size * l.e_count;
        else {
            newStrip = _data.back().strip + l.e_size * l.e_count;
            currOffset = _data.back().strip;
            for (auto& i : _data)
                i.strip = newStrip;
        }
        _data.push_back(l);
    }
    uint16_t strip() const { return _data.front().strip; }
    size_t size() const { return _data.size(); }
    LayoutItem& operator[](unsigned int idx) { return _data[idx]; }
    LayoutItem* Find(ArrayType t) {
        for (auto it = _data.begin(); it < _data.end(); it++)
            if (it->type == t)
                return &*it;
        return nullptr;
    }
    const LayoutItem* getLayoutItem(ArrayType t) const;
};

enum ElementType {
    BYTE = 5120,
    UNSIGNED_BYTE = 5121,
    SHORT = 5122,
    UNSIGNED_SHORT = 5123,
    UNSIGNED_INT = 5125,
    FLOAT = 5126
};

struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent 切线
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
};

struct Texture {
    unsigned int texture_id;
    unsigned char *image;
    int width, height, nrChannels;
    Texture(){
        width=0;height=0;
    };
    Texture(unsigned char *image):image(image){};
};

class Material{
public:
    //pbrMetallicRoughness
    glm::vec4 baseColorFactor;  //default: [1,1,1,1]
    float metallicFactor;   //金属感强度 default: 1
    float roughnessFactor;  //粗糙感强度 default: 1
    Texture baseColorTexture;

    Texture metallicRoughnessTexture;
    Texture normalTexture;
    glm::vec3 emissiveFactor;  //自发光强度 default: [0,0,0]

    int alphaMode;//0:OPAQUE 完全不透明  1:MASK  2:BLEND
    float alphaCutoff;//MASK模式中,如果小于alphaCutoff的值则为完全透明，否则为完全不透明
    bool doubleSided;//是否为双面贴图

    Material(){
        glm::vec4 temp_b(1.0,1.0,1.0,1.0);
        baseColorFactor = temp_b;
        metallicFactor = 1.0;
        roughnessFactor = 1.0;
        baseColorTexture = NULL;
        metallicRoughnessTexture = NULL;
        normalTexture = NULL;
        glm::vec3 temp_e(1.0,1.0,1.0);
        emissiveFactor = temp_e;

        alphaMode = 0;
        alphaCutoff = 0.5;
        doubleSided = false;
    }
};

struct Primitive{
public:
    Material material;         //材质
    GLenum mode;               //mode属性的值为0表示点，4表示三角形
    LayoutItem index;          //index的数据存储状态
    bool haveIndex = false;
    vector<LayoutItem> layouts;//attributes
    unsigned int VAO;

    vector<float> vertex_pos;
    vector<unsigned int> indices;

    void setupPrimitive()
    {

        // create buffers/arrays
        // 创建 VBO 顶点缓冲对象 VAO顶点数组对象 EBO索引缓冲对象
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);


        if(haveIndex){
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,index.e_size*index.e_count*index.e_num,index.data_ptr, GL_STATIC_DRAW);
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        //先假定只有一个buffer
        glBufferData(GL_ARRAY_BUFFER,layouts[0].buffer_byteLength,layouts[0].buffer_ptr,GL_STATIC_DRAW);



        printf("primitive:%u\n",VAO);


        for(auto it = layouts.begin();it!=layouts.end();it++)
        {
            //POSITION
            if(it->type==ARRAY_VERTEX){
                glVertexAttribPointer(0, it->e_count, GL_FLOAT, GL_FALSE, it->strip, (const void*)it->offset);
                glEnableVertexAttribArray(0);
            }
            //NORMAL
            if(it->type==ARRAY_NORMAL){
                glVertexAttribPointer(1, it->e_count, GL_FLOAT, GL_FALSE, it->strip, (const void*)it->offset);
                glEnableVertexAttribArray(1);
            }
            //TANGENT
            if(it->type==ARRAY_TANGENT){
                glVertexAttribPointer(2, it->e_count, GL_FLOAT, GL_FALSE, it->strip, (const void*)it->offset);
                glEnableVertexAttribArray(2);
            }
            //TEXCOORD_0 ARRAY_TEX_UV
            if(it->type==ARRAY_TEX_UV){
                glVertexAttribPointer(3, it->e_count, GL_FLOAT, GL_FALSE, it->strip, (const void*)it->offset);
                glEnableVertexAttribArray(3);

//                float* position_data = new float[it->e_count*it->e_num];
//                std::memcpy(position_data, it->data_ptr, it->e_count*it->e_num*it->e_size);
//                for(int j=0;j<it->e_count*it->e_num;j++)
//                {
//                    std::cout<<position_data[j]<<" ";
//                    if((j+1)%it->e_count==0) printf("\n");
//                }
            }
            //TEXCOORD_1 ARRAY_TEX_UV2
            if(it->type==ARRAY_TEX_UV2){
                glVertexAttribPointer(4, it->e_count, GL_FLOAT, GL_FALSE, it->strip, (const void*)it->offset);
                glEnableVertexAttribArray(4);
            }
        }
        glBindVertexArray(0);

    }

    void Draw(Shader &shader)
    {
        //生成纹理
        if(material.baseColorTexture.height!=0)
        {
            glActiveTexture(GL_TEXTURE0); // 在绑定纹理之前先激活纹理单元
            glBindTexture(GL_TEXTURE_2D, material.baseColorTexture.texture_id);
//            printf("primitive:%u and texture:%u\n",VAO,material.baseColorTexture.texture_id);
        }
        if(material.metallicRoughnessTexture.height!=0)
        {
            glActiveTexture(GL_TEXTURE0+1); // 在绑定纹理之前先激活纹理单元
            glBindTexture(GL_TEXTURE_2D, material.metallicRoughnessTexture.texture_id);
        }
        if(material.normalTexture.height!=0)
        {
            glActiveTexture(GL_TEXTURE0+2); // 在绑定纹理之前先激活纹理单元
            glBindTexture(GL_TEXTURE_2D, material.normalTexture.texture_id);
        }
        shader.setInt("alphaMode",material.alphaMode);



        // draw mesh
        // 绘制网格
        glBindVertexArray(VAO);
//        glDrawArrays(GL_TRIANGLES, 0, vertex_pos.size());
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // always good practice to set everything back to defaults once configured.
//        glActiveTexture(GL_TEXTURE0);
    }
private:
    // render data
    unsigned int VBO, EBO;


};

class Mesh {
public:
    vector<Primitive> primitives;
    glm::mat4 transform_mat;

    // mesh Data
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;


    Material material;
    GLenum mode;

    Mesh(){};

    Mesh(vector<Primitive> primitives,glm::mat4 transform_mat) : primitives(primitives),transform_mat(transform_mat)
    {
        this->primitives=primitives;
        for(int i = 0;i<this->primitives.size();i++)
        {
            this->primitives[i].setupPrimitive();
        }

    };

    // constructor
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
    }

    // render the mesh
    void Draw(Shader &shader)
    {
        // render the loaded model
        // 模型空间--世界空间矩阵
        shader.setMat4("model",transform_mat);

        for(unsigned int i = 0; i < primitives.size(); i++)
            primitives[i].Draw(shader);
    }

private:
    // render data

    // initializes all the buffer objects/arrays
    void setupMesh()
    {

    }
};
#endif