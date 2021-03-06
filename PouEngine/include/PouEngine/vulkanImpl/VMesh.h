#ifndef VMESH_H
#define VMESH_H

#include <glm/glm.hpp>
#include <Vulkan/vulkan.h>
#include <array>
#include <vector>

#include "PouEngine/Types.h"
#include "PouEngine/VulkanImpl/VBuffersAllocator.h"

namespace pou
{

struct MeshVertex
{
    glm::vec3 pos;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;

    /*glm::vec4 albedo_color;
    glm::vec3 rme_color;

    glm::uvec2 albedo_texId;
    glm::uvec2 height_texId;
    glm::uvec2 normal_texId;
    glm::uvec2 rme_texId;*/


    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions();
};


class VMesh
{
    public:
        VMesh();
        virtual ~VMesh();

        VMesh( const VMesh& ) = delete;
        VMesh& operator=( const VMesh& ) = delete;

        bool generateMesh(  std::vector<MeshVertex> &vertexList,
                            std::vector<uint16_t> &indexList,
                            CommandPoolName poolName = COMMANDPOOL_SHORTLIVED);

        VBuffer getVertexBuffer();
        VBuffer getIndexBuffer();
        size_t  getIndexCount();

    protected:
         VBuffer m_vertexBuffer;
         VBuffer m_indexBuffer;
         size_t  m_indexCount;

};

}

#endif // VMESH_H
