#ifndef MESHASSET_H
#define MESHASSET_H

#include <vector>

#include "PouEngine/assets/MaterialAsset.h"
#include "PouEngine/vulkanImpl/VulkanImpl.h"


namespace pou
{


struct VertexTriangle
{
    glm::vec3 v1;
    glm::vec3 v2;
    glm::vec3 v3;
};

class MeshAsset : public Asset, public NotificationListener
{
    friend class SceneRenderer;
    friend class MeshesHandler;

    public:
        MeshAsset();
        MeshAsset(const AssetTypeId);
        virtual ~MeshAsset();

        bool loadFromFile(const std::string &filePath);
        virtual void notify(NotificationSender* , int notificationType,
                            void* data = nullptr) override;

        void setMaterial(AssetTypeId materialId);
        void setMaterial(MaterialAsset* material);

        float getScale();
        MaterialAsset*  getMaterial();
        VMesh*          getMesh();

    protected:
        bool loadFromXML(TiXmlHandle *);
        bool loadModelFromObj(const std::string &filePath, bool flipUV);


        bool generateModel(const std::vector<glm::vec3> &vertexList,
                           const std::vector<glm::vec2> &uvList,
                           const std::vector<glm::vec3> &normalList,
                           const std::vector<VertexTriangle> &indexList);

        bool generateModel(std::vector<MeshVertex> &vertexList,
                           std::vector<uint16_t>   &indexList);


    private:
        std::atomic<bool> m_materialsLoaded;
        MaterialAsset  *m_material; //Could be vector

        bool    m_meshLoading;
        bool    m_meshLoaded;
        VMesh  *m_mesh;

        float m_scale;
};


}

#endif // MESHASSET_H
