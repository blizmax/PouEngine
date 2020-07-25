#ifndef TERRAINGENERATOR_H
#define TERRAINGENERATOR_H

#include "world/WorldNode.h"
#include "world/GameWorld_Sync.h"
#include "assets/TerrainLayerModelAsset.h"

#include "PouEngine/tools/RNGesus.h"

#include <stack>

struct TerrainGenerator_GroundLayer
{
    TerrainGenerator_GroundLayer *parentLayer;
    TerrainLayerModelAsset *layerModel;

    bool    occulting;
    size_t  depth;
    int     spawnPointSparsity;
    float   spawnProbability;
    float   expandProbability;
};

class TerrainGenerator
{
    public:
        TerrainGenerator();
        virtual ~TerrainGenerator();

        ///Add Load From XML, etc

        bool loadFromFile(const std::string &filePath);

        void generatesOnNode(std::shared_ptr<WorldNode> targetNode, int seed, GameWorld_Sync *syncComponent = nullptr);

        const std::string &getFilePath();
        int getGeneratingSeed();

    protected:
        bool loadFromXML(TiXmlHandle *hdl);
        bool loadParameters(TiXmlElement *element);
        bool loadGroundLayer(TiXmlElement *element, TerrainGenerator_GroundLayer *parentLayer);

        TerrainGenerator_GroundLayer* getGridValue(int x, int y);
        size_t getGridDepth(int x, int y);
        bool lookForNonOccludedLayer(int x, int y, TerrainGenerator_GroundLayer *layer);

        void generateGrid();
        //void spawnLayerElement(int x, int y, size_t layer, float spawnProbability, float expandProbability);
        void spawnLayerElement(int x, int y, TerrainGenerator_GroundLayer *groundLayer);

        void generateSprites(int x, int y,
                             std::shared_ptr<WorldNode> targetNode, GameWorld_Sync *syncComponent);
        //void generateSprite(std::list<TerrainGenerator_LayerModelElement> *listModel,
        //                    std::shared_ptr<WorldNode> targetNode, GameWorld_Sync *syncComponent);


    private:
        pou::RNGenerator m_rng;

        std::string m_filePath;
        std::string m_fileDirectory;

        int m_generatingSeed;

        glm::vec2 m_tileSize;
        glm::vec2 m_terrainSize;
        glm::vec2 m_gridSize;

        //int m_nbrLayers;
        //std::list<TerrainLayerModelAsset*> m_layerModels;
        std::list<TerrainGenerator_GroundLayer> m_groundLayers;

        //std::vector<size_t> m_generatingGrid;
        std::vector<TerrainGenerator_GroundLayer*> m_generatingGrid;

};

#endif // TERRAINGENERATOR_H
