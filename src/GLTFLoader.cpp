#include "GLTFLoader.h"



void Model::collectVertexAttributes(const Value& attributes, const Value& accessors, const Value& bufferViews, const std::vector<char*>& buffers,
                              Layout& vbLayout, uint32_t& vertexNum,std::vector<LayoutItem>& layouts,Primitive& primitive) {
    //分析primitive[i]里attributes的内容，提取到layouts数组里
    vertexNum = 0;
    for (auto it = attributes.MemberBegin(); it != attributes.MemberEnd(); it++) {
        LayoutItem l;
        std::string attrib = it->name.GetString();
        if (attrib == "POSITION")
            l.type = ARRAY_VERTEX;
        else if (attrib == "TEXCOORD_0")
            l.type = ARRAY_TEX_UV;
        else if (attrib == "TEXCOORD_1")
            l.type = ARRAY_TEX_UV2;
        else if (attrib == "NORMAL")
            l.type = ARRAY_NORMAL;
        else if (attrib == "TANGENT")
            l.type = ARRAY_TANGENT;
        const Value& accessor = accessors[it->value.GetUint()];
        if (vertexNum == 0)
            vertexNum = accessor["count"].GetUint();
        layoutFromAccessor(l, accessor, bufferViews);

        if(l.type == ARRAY_VERTEX)
        {

            float* position_data = new float[l.e_count*l.e_num];
            std::memcpy(position_data, l.data_ptr, l.e_count*l.e_num*l.e_size);
            for(int j=0;j<l.e_count*l.e_num;j++)
            {
//                if(j<10) printf("%02f ",position_data[j]);
                primitive.vertex_pos.push_back(position_data[j]);
            }

        }

        layouts.push_back(l);
    }


}

void Model::layoutFromAccessor(LayoutItem& l, const Value& accessor, const Value& bufferViews) {
    //bufferView
    const Value& bufferView = bufferViews[accessor["bufferView"].GetUint()];

    l.offset = 0;

    //整合数据偏移
    uint32_t bufferVOffset = 0, accessorOffset = 0;
    //bufferVOffset
    auto it = bufferView.FindMember("byteOffset");
    if (it != bufferView.MemberEnd())
        bufferVOffset = it->value.GetUint();
    //accessorOffset
    it = accessor.FindMember("byteOffset");
    if (it != accessor.MemberEnd())
        accessorOffset = it->value.GetUint();
    uint32_t offset = bufferVOffset + accessorOffset;
    l.offset = offset;

    //dataptr数据起始位置

//    std::cout<<"buffer_array[bufferView[\"buffer\"].GetUint()]: " <<  &(buffer_array[bufferView["buffer"].GetUint()])<<"\n" ;
//    l.data_ptr = (char*)&(buffer_array[bufferView["buffer"].GetUint()]) + offset;
//    std::cout<<(void*)buffer_array[bufferView["buffer"].GetUint()]<<"\n";
    l.data_ptr = buffer_array[bufferView["buffer"].GetUint()] + offset;
//    std::cout<<"offset:"<<offset<<"\n";
//    std::cout<<"data_ptr:"<<l.data_ptr<<"\n";

    //buffer_indx数据所在buffer的索引
    l.buffer_indx = bufferView["buffer"].GetUint();
    l.buffer_byteLength = buffer_byteLengths[l.buffer_indx];
    l.buffer_ptr =(void*)buffer_array[bufferView["buffer"].GetUint()];


    //normalized
    it = accessor.FindMember("normalized");
    if (it != accessor.MemberEnd())
        l.normalized = it->value.GetBool() ? GL_TRUE : GL_FALSE;

    //componentType
    uint32_t eType = accessor["componentType"].GetUint();
    // TODO: direct copy element type!
    switch (eType) {
        case ElementType::BYTE:
            l.e_size = 1;
            l.e_format = GL_BYTE; break;
        case ElementType::FLOAT:
            l.e_size = 4;
            l.e_format = GL_FLOAT; break;
        case ElementType::SHORT:
            l.e_size = 2;
            l.e_format = GL_SHORT; break;
        case ElementType::UNSIGNED_BYTE:
            l.e_size = 1;
            l.e_format = GL_UNSIGNED_BYTE; break;
        case ElementType::UNSIGNED_INT:
            l.e_size = 4;
            l.e_format = GL_UNSIGNED_INT; break;
        case ElementType::UNSIGNED_SHORT:
            l.e_size = 2;
            l.e_format = GL_UNSIGNED_SHORT; break;
        default:
            break;
    }

    //type
    std::string type_str = accessor["type"].GetString();
    if (type_str == "SCALAR")
        l.e_count = 1;
    else if (type_str == "VEC2")
        l.e_count = 2;
    else if (type_str == "VEC3")
        l.e_count = 3;
    else if (type_str == "VEC4")
        l.e_count = 4;

    //num 数据元素的数量
    l.e_num= accessor["count"].GetUint();

    //stride
    uint32_t stride;
    it = bufferView.FindMember("byteStride");
    if (it != bufferView.MemberEnd())
        stride = it->value.GetUint();
    else
        stride = l.e_size*l.e_count;
    l.strip = stride;


}

glm::mat4 Model::GetTransformMat(const Value& node){
    glm::mat4 new_mesh2obj;
    auto it = node.FindMember("matrix");
    //if node has matrix
    if (node.HasMember("matrix")){
        for(int i=0;i<4;i++)
            for(int j=0;j<4;j++){
                new_mesh2obj[i][j] = node["matrix"][i*4+j].GetFloat();
            }
    }
    // if node has scale\rotation\translation
    else {
        // use TRS
        glm::mat4 scale_m(1.0f);
        glm::mat4 rotation_m(1.0f);
        glm::mat4 translation_m(1.0f);
        if(node.HasMember("scale")){
            scale_m = glm::scale(scale_m,glm::vec3(node["scale"][0].GetFloat(),node["scale"][1].GetFloat(),node["scale"][2].GetFloat()));
        }
        if(node.HasMember("rotation")){
            //四元数
            glm::fquat temp_q(node["rotation"][0].GetFloat(),node["rotation"][1].GetFloat(),node["rotation"][2].GetFloat(),node["rotation"][3].GetFloat());
            rotation_m = glm::mat4_cast(temp_q);
        }
        if(node.HasMember("translation")){
            translation_m = glm::translate(translation_m,glm::vec3(node["translation"][0].GetFloat(),node["translation"][1].GetFloat(),node["translation"][2].GetFloat()));
        }
        new_mesh2obj = translation_m * rotation_m * scale_m;
    }
    return new_mesh2obj;
}

void Model::AddMesh(const Value& node, glm::mat4 curr_mesh2obj,
                  const Value& node_pool, const Value& mesh_pool,
                  const Value& accessor_pool, const Value& bufferView_pool) {
    // node's transform
    glm::mat4 new_mesh2obj;
    new_mesh2obj = GetTransformMat(node);
    curr_mesh2obj = curr_mesh2obj * new_mesh2obj;


    //node's mesh
    if(node.HasMember("mesh")){
        uint32_t mesh_idx = node["mesh"].GetUint();
        //mesh --> primitives
        const Value& primitive_json = mesh_pool[mesh_idx]["primitives"];
        vector<Primitive> primitives;
        //单个primitive
        for(int j=0;j<primitive_json.Size();j++){
            Primitive primitive;
            //material
            if(primitive_json[j].HasMember("material")){
//                printf("%d",primitive_json[j]["material"].GetUint());
                primitive.material = material_array[primitive_json[j]["material"].GetUint()];
            }
            //mode
            GLenum primitive_mode = GL_TRIANGLES;
            primitive.mode = primitive_mode;
            if(primitive_json[j].HasMember("mode")){
                primitive.mode = primitive_json[j]["mode"].GetUint();
            }
            //index
            if(primitive_json[j].HasMember("indices")){
                const Value& index_accessor = accessor_pool[primitive_json[j]["indices"].GetUint()];
                uint32_t index_count = index_accessor["count"].GetUint(); //数据元素的数量

                LayoutItem indexLayout;
                layoutFromAccessor(indexLayout, index_accessor, bufferView_pool);
                indexLayout.type = ARRAY_INDEX;
                primitive.index = indexLayout;
                primitive.haveIndex = true;


                //加载数据
//                //初始化index_data数组,大小为：单数据字节大小*分量大小\*数据元素的数量
                unsigned int* index_data = new unsigned int[indexLayout.e_count*index_count];
                std::memcpy(index_data, indexLayout.data_ptr, indexLayout.e_size*indexLayout.e_count*index_count);
//                std::memcpy(index_data, buffer_array[indexLayout.buffer_indx], indexLayout.e_size*indexLayout.e_count*index_count);
                for(int j=0;j<indexLayout.e_count*index_count;j++)
                {
//                    if(j<10) printf("%d ",index_data[j]);
                    primitive.indices.push_back(index_data[j]);
                }
            }

            //attributes
            if(primitive_json[j].HasMember("attributes")){
                // construct vertex buffer layout
                Layout vbLayout;
                std::vector<LayoutItem> layouts;
                uint32_t vertexNum;
                collectVertexAttributes(primitive_json[j]["attributes"], accessor_pool, bufferView_pool, buffer_array, vbLayout, vertexNum, layouts,primitive);
                primitive.layouts = layouts;
                // 新增一个primitive
//                meshes.push_back(new TriangleMesh(vertex_data, vbLayout, vertexNum, index_data, indexNum / 3, indexElementT, Transform::Identity(), primitive_mode));
            }
            //新增一个primitive
            primitives.push_back(primitive);


        }
        //新增一个mesh,mesh带有primitives
        Mesh m(primitives,curr_mesh2obj);
        meshes.push_back(m);
//            Primitive3D* p = new Primitive3D(rtms, meshes, Transform(curr_mesh2obj));
//            primitives.push_back(p);

    }
    // add children 递归
    auto it = node.FindMember("children");
    if (it != node.MemberEnd()) {
        const auto& children = it->value;
        for (int i = 0; i < children.Size(); i++) {
            const Value& childNode = node_pool[children[i].GetUint()];
            AddMesh(childNode, curr_mesh2obj, node_pool, mesh_pool, accessor_pool, bufferView_pool);
        }
    }
}

void Model::loadModel(string const &path){
    //gltf文件路径，并解析其所在目录directory
    int dirStart = path.find_last_of('/');
    directory = path.substr(0, dirStart);

//    stbi_set_flip_vertically_on_load(true);

    //打开文件
    std::ifstream gltfFile(path);
    if(!gltfFile){
        printf("未找到gltf文件\n");
        return;
    }

    //读取文件到gltfFileStr
    std::string gltfFileStr;
    std::stringstream gltfFileStream;
    gltfFileStream << gltfFile.rdbuf();
    gltfFileStr = gltfFileStream.str();

    //创建解析对象
    Document d;
    if(d.Parse(gltfFileStr.c_str()).HasParseError()){
        printf("解析错误\n");
        return;
    }

    //buffers加载入buffer_array[]
    const Value& buffers_json = d["buffers"];
    for (int i = 0; i < buffers_json.Size(); i++){
        std::ifstream fbuffer(directory + '/' + buffers_json[i]["uri"].GetString(), std::ios_base::binary);
        if(!fbuffer){
            printf("未找到bin文件\n");
            return;
        }

        uint32_t byteLength = buffers_json[i]["byteLength"].GetUint();
        buffer_byteLengths.push_back(byteLength);
        char* bin_data = new char[byteLength];
        fbuffer.read(bin_data, byteLength);
        buffer_array.push_back(bin_data);
//        std::cout<<"bin_data: " <<  (void*)&bin_data  <<"\n";
//        std::cout<<"bin_data: " <<  (void*)(buffer_array[0])  <<"\n";

    }




    //texture
    if(d.HasMember("textures")){
        const Value& textures_json = d["textures"];
        for (int i = 0; i < textures_json.Size(); i++){
            //加载
            int img_idx = textures_json[i]["source"].GetInt();
            int sampler_idx = -1;
            if(textures_json[i].HasMember("sampler")){
                sampler_idx = textures_json[i]["sampler"].GetInt();
            }

            std::string img_path = directory + '/' + d["images"][img_idx]["uri"].GetString();
            int width, height, nrChannels;
            unsigned char *data = stbi_load(img_path.c_str(), &width, &height, &nrChannels, 0);
            //创建
            unsigned int texture;
            glGenTextures(1, &texture);

            Texture tex(data);
            tex.width=width;
            tex.height=height;
            tex.nrChannels=nrChannels;
            tex.texture_id=texture;

            if (data)
            {
                GLenum format;
                if (nrChannels == 1)
                    format = GL_RED;
                else if (nrChannels == 3)
                    format = GL_RGB;
                else if (nrChannels == 4)
                    format = GL_RGBA;
                //绑定
                glBindTexture(GL_TEXTURE_2D, texture);
                glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);

                //samplers
                auto minFilter = GL_LINEAR;
                auto magFilter = GL_LINEAR;
                auto wrapS = GL_REPEAT;
                auto wrapT = GL_REPEAT;
                if(d.HasMember("samplers")&&sampler_idx!=-1){
                    const Value& sampler_json = d["samplers"][sampler_idx];
                    if(sampler_json.HasMember("magFilter"))
                        magFilter = sampler_json["magFilter"].GetUint();
                    if(sampler_json.HasMember("magFilter"))
                        magFilter = sampler_json["magFilter"].GetUint();
                    if(sampler_json.HasMember("magFilter"))
                        magFilter = sampler_json["magFilter"].GetUint();
                    if(sampler_json.HasMember("magFilter"))
                        magFilter = sampler_json["magFilter"].GetUint();
                }

                // 为当前绑定的纹理对象设置环绕、过滤方式
                // 坐标轴设置S\T
                // 放大(Magnify)和缩小(Minify)操作
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

                stbi_image_free(data);
            }
            else
            {
                std::cout << "Texture failed to load at path: " << path << std::endl;
                stbi_image_free(data);
            }
            printf("texture:%u\n",texture);
            texture_array.push_back(tex);
        }
    }

    //materials
    if(d.HasMember("materials")){
        const Value& materials_json = d["materials"];
        for(int i=0;i<materials_json.Size();i++){
            Material m;
            const Value& material_json = materials_json[i];

            //pbrMetallicRoughness
            const Value& pbr_json = material_json["pbrMetallicRoughness"];
            if(pbr_json.HasMember("baseColorFactor")){
                for(int j=0;j<4;j++)
                    m.baseColorFactor[j]=pbr_json["baseColorFactor"][j].GetFloat();
            }
            if(pbr_json.HasMember("metallicFactor")){
                m.metallicFactor = pbr_json["metallicFactor"].GetFloat();
            }
            if(pbr_json.HasMember("roughnessFactor")){
                m.metallicFactor = pbr_json["roughnessFactor"].GetFloat();
            }
            if(pbr_json.HasMember("baseColorTexture")){
                m.baseColorTexture = texture_array[pbr_json["baseColorTexture"]["index"].GetInt()];
                std::cout<<m.baseColorTexture.texture_id<<" "<<m.baseColorTexture.width<<" "<<m.baseColorTexture.nrChannels<<"\n";
            }
            if(pbr_json.HasMember("metallicRoughnessTexture")){
                m.metallicRoughnessTexture = texture_array[pbr_json["metallicRoughnessTexture"]["index"].GetInt()];
            }

            //other
            if(material_json.HasMember("normalTexture")){
                m.normalTexture = texture_array[material_json["normalTexture"]["index"].GetInt()];
            }
            if(material_json.HasMember("emissiveFactor")){
                for(int j=0;j<3;j++){
                    m.emissiveFactor[j]=material_json["emissiveFactor"][j].GetFloat();
                }
            }
            if(material_json.HasMember("alphaMode")){
                if(std::string("BLEND")==material_json["alphaMode"].GetString())
                    m.alphaMode = 2;
                else if(std::string("MASK")==material_json["alphaMode"].GetString())
                    m.alphaMode = 1;
                else
                    m.alphaMode = 0;
            }
            if(material_json.HasMember("alphaCutoff")){
                m.alphaCutoff = material_json["alphaCutoff"].GetFloat();
            }
            if(material_json.HasMember("doubleSided")){
                m.doubleSided = material_json["doubleSided"].GetBool();
            }
            material_array.push_back(m);
        }
    }
    std::cout<<"size of material_a"<<material_array.size()<<"\n";

    // nodes
    // only the first scene is parsed
    const Value& nodes_array = d["scenes"][0]["nodes"];
    const Value& nodes_pool = d["nodes"];
    const Value& mesh_pool = d["meshes"];
    const Value& accessor_pool = d["accessors"];
    const Value& bufferview_pool = d["bufferViews"];

    for (int i = 0; i < nodes_array.Size(); i++) {
        // each node contains multiple meshes, so here treated as a primitive
        uint32_t node_idx = nodes_array[i].GetUint();
        const Value& node = nodes_pool[node_idx];
        glm::mat4 identity(1.0f);
        AddMesh(node, identity,  nodes_pool, mesh_pool, accessor_pool, bufferview_pool);
    }
//    for (char* p : buffer_array)
//        delete p;
    return;
}