#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include "PouEngine/Types.h"
#include "PouEngine/core/NotificationListener.h"

/** Any object that can be put in a scene : instance of sprite or mesh, ligth source, camera, ambient sound, ... **/

namespace pou
{

class SceneNode;

class SceneObject : public NotificationListener
{
    friend class SceneNode;

    public:
        SceneObject();
        virtual ~SceneObject();

        SceneNode *getParentNode();

        bool isALight();
        bool isAnEntity();
        bool isAShadowCaster();

        void setLocalTime(float time);
        void setLastUpdateTime(float time, bool force = false);
        float getLastUpdateTime();

        virtual void update(const Time &elapsedTime, float localTime = -1);
        virtual void notify(NotificationSender* , NotificationType,
                            size_t dataSize = 0, char* data = nullptr) override;

    protected:

        SceneNode *setParentNode(SceneNode*);
        SceneNode *m_parentNode;

        bool m_isALight;
        bool m_isAnEntity;
        bool m_isAShadowCaster;

        float m_curLocalTime;
        float m_lastUpdateTime;

    private:
};

}

#endif // SCENEOBJECT_H
