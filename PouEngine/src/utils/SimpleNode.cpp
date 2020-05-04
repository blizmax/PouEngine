#include "PouEngine/utils/SimpleNode.h"

#include "PouEngine/utils/Logger.h"

namespace pou
{

SimpleNode::SimpleNode(const NodeTypeId id) :
    m_globalPosition(0.0,0.0,0.0),
    m_position(0.0,0.0,0.0),
    m_eulerRotations(0.0,0.0,0.0),
    m_scale(1.0,1.0,1.0),
    m_rigidity(1.0),
    m_curFlexibleLength(0.0),
    m_curFlexibleRotation(0.0),
    m_modelMatrix(1.0),
    m_invModelMatrix(1.0)
{
    m_parent = nullptr;
    m_id = id;
    m_curNewId = 0;
    m_needToUpdateModelMat = true;
}

SimpleNode::~SimpleNode()
{
    this->removeAndDestroyAllChilds();
}


void SimpleNode::addChildNode(SimpleNode* node)
{
    NodeTypeId id = this->generateId();
    this->addChildNode(id, node);
    if(node != nullptr)
        node->setId(id);
}

void SimpleNode::addChildNode(const NodeTypeId id, SimpleNode* node)
{
    auto childsIt = m_childs.find(id);
    if(childsIt != m_childs.end())
   // if(this->getChild(id) == nullptr)
    {
        std::ostringstream warn_report;
        warn_report << "Adding child node of same id as another one (Id="<<id<<")";
        Logger::warning(warn_report);
    }

    m_childs.insert_or_assign(childsIt,id,node);
    node->setParent(this);
}

void SimpleNode::addCreatedChildNode(SimpleNode* node)
{
    NodeTypeId id = this->generateId();
    this->addChildNode(id, node);
    if(node != nullptr)
    {
        node->setId(id);
        m_createdChildsList.insert(id);
    }
}

void SimpleNode::moveChildNode(SimpleNode* node, SimpleNode* target)
{
    if(node != nullptr && node->getParent() == this)
        this->moveChildNode(node->getId(), target);
}

void SimpleNode::moveChildNode(const NodeTypeId id, SimpleNode* target)
{
    auto node = this->getChild(id);
    auto createdChildsIt = m_createdChildsList.find(id);

    if(createdChildsIt != m_createdChildsList.end())
    {
        m_createdChildsList.erase(createdChildsIt);
        target->addCreatedChildNode(node);
    } else
        target->addChildNode(node);
}

SimpleNode* SimpleNode::removeChildNode(const NodeTypeId id)
{
    SimpleNode* node = nullptr;

    auto childsIt = m_childs.find(id);

    if(childsIt == m_childs.end())
    {
        std::ostringstream error_report;
        error_report << "Cannot remove child node (Id="<<id<<")";
        Logger::error(error_report);

        return (nullptr);
    }

    node = childsIt->second;
    node->setParent(nullptr);

    m_childs.erase(childsIt);

    return node;
}

SimpleNode* SimpleNode::removeChildNode(SimpleNode* node)
{
    if(node != nullptr && node->getParent() == this)
        return this->removeChildNode(node->getId());
    return (nullptr);
}

SimpleNode* SimpleNode::createChildNode()
{
    return this->createChildNode(this->generateId());
}

SimpleNode* SimpleNode::createChildNode(float x, float y, float z)
{
    SimpleNode* newNode = this->createChildNode();
    if(newNode != nullptr)
        newNode->setPosition(x,y,z);
    return newNode;
}


SimpleNode* SimpleNode::createChildNode(float x, float y)
{
    return this->createChildNode(x,y,0);
}

SimpleNode* SimpleNode::createChildNode(glm::vec2 p)
{
    return this->createChildNode(p.x, p.y);
}

SimpleNode* SimpleNode::createChildNode(glm::vec3 p)
{
    return this->createChildNode(p.x, p.y, p.z);
}

SimpleNode* SimpleNode::createChildNode(const NodeTypeId id)
{
    if(static_cast<unsigned int>(m_curNewId) <= id)
        m_curNewId = id+1;

    if(this->getChild(id) != nullptr)
    {
        std::ostringstream error_report;
        error_report << "Cannot create new child node with the same Id as an existing child node (Id="<<id<<")";
        Logger::error(error_report);

        return (nullptr); //childsIt->second;
    }

    SimpleNode* newNode = this->nodeAllocator(id);
    m_createdChildsList.insert(id);

    this->addChildNode(id, newNode);

    return newNode;
}

bool SimpleNode::destroyChildNode(SimpleNode* node)
{
    if(node != nullptr && node->getParent() == this)
        return this->destroyChildNode(node->getId());
    return (false);
}

bool SimpleNode::destroyChildNode(const NodeTypeId id)
{
    auto createdChildsIt = m_createdChildsList.find(id);

    if(createdChildsIt == m_createdChildsList.end())
        Logger::warning("Destroying non-created child");
    else
        m_createdChildsList.erase(createdChildsIt);


    //SimpleNode* child = this->getChild(id);
    auto childsIt = m_childs.find(id);

    if(childsIt == m_childs.end())
    //if(child == nullptr)
    {
        std::ostringstream error_report;
        error_report << "Cannot destroy child (Id="<<id<<")";
        Logger::error(error_report);

        return (false);
    }

    if(childsIt->second != nullptr)
        delete childsIt->second;
    m_childs.erase(childsIt);

    return (true);
}

void SimpleNode::removeAndDestroyAllChilds(bool destroyNonCreatedChilds)
{
    if(!destroyNonCreatedChilds)
        while(!m_createdChildsList.empty())
            this->destroyChildNode(*m_createdChildsList.begin());

    if(destroyNonCreatedChilds)
    {
        for(auto childsIt : m_childs)
        {
            if(childsIt.second != nullptr)
            {
                childsIt.second->removeAndDestroyAllChilds(destroyNonCreatedChilds);
                delete childsIt.second;
            }
        }
    }

    m_childs.clear();
    m_createdChildsList.clear();
}

void SimpleNode::copyFrom(const SimpleNode* srcNode)
{
    if(srcNode == nullptr)
        return;

    this->setPosition(srcNode->getPosition());
    this->setRotation(srcNode->getEulerRotation());
    this->setScale(srcNode->getScale());
    this->setName(srcNode->getName());
    this->setRigidity(srcNode->getRigidity());

    for(auto child : srcNode->m_childs)
    {
        auto newNode = this->createChildNode();
        if(newNode != nullptr)
            newNode->copyFrom(child.second);
    }
}

void SimpleNode::move(float x, float y)
{
    this->move(x,y,0);
}

void SimpleNode::move(float x, float y, float z)
{
    this->move(glm::vec3(x,y,z));
}

void SimpleNode::move(glm::vec2 p)
{
    this->move(glm::vec3(p.x,p.y,0));
}

void SimpleNode::move(glm::vec3 p)
{
    /*for(auto child : m_childs)
        child.second->move((child.second->getRigidity()-1.0f)*p);*/

    for(auto child : m_childs)
        child.second->computeFlexibleMove(p);

    this->setPosition(this->getPosition()+p);
}

void SimpleNode::setPosition(float x, float y)
{
    this->setPosition(glm::vec2(x,y));
}

void SimpleNode::setPosition(float x, float y, float z)
{
    this->setPosition(glm::vec3(x, y, z));
}

void SimpleNode::setPosition(glm::vec2 xyPos)
{
    this->setPosition(glm::vec3(xyPos.x, xyPos.y,this->getPosition().z));
}

void SimpleNode::setPosition(glm::vec3 pos)
{
    m_position = pos;

    this->updateGlobalPosition();
}


void SimpleNode::setGlobalPosition(float x, float y)
{
    this->setGlobalPosition(glm::vec2(x,y));
}

void SimpleNode::setGlobalPosition(float x, float y, float z)
{
    this->setGlobalPosition(glm::vec3(x, y, z));
}

void SimpleNode::setGlobalPosition(glm::vec2 xyPos)
{
    this->setGlobalPosition(glm::vec3(xyPos.x, xyPos.y,this->getGlobalPosition().z));
}

void SimpleNode::setGlobalPosition(glm::vec3 pos)
{
    if(m_parent != nullptr)
    {
        glm::vec4 newPos = m_parent->getInvModelMatrix() * glm::vec4(pos,1.0);
        this->setPosition({newPos.x, newPos.y, newPos.z});
    }
    else
        m_position = pos;

    this->updateGlobalPosition();
}

void SimpleNode::scale(float scale)
{
    this->scale({scale, scale, scale});
}

void SimpleNode::scale(glm::vec3 scale)
{
    this->setScale(m_scale*scale);
}

void SimpleNode::linearScale(float x, float y, float z)
{
    this->linearScale(glm::vec3(x,y,z));
}

void SimpleNode::linearScale(glm::vec3 scale)
{
    this->setScale(m_scale+scale);
}

void SimpleNode::setScale(float scale)
{
    this->setScale({scale, scale, scale});
}

void SimpleNode::setScale(glm::vec3 scale)
{
    m_scale = scale;
    this->askForUpdateModelMatrix();
}

void SimpleNode::rotate(float value, glm::vec3 axis, bool inRadians)
{
    this->rotate(value*axis, inRadians);
}

void SimpleNode::rotate(glm::vec3 values, bool inRadians)
{
    if(!inRadians)
        values = glm::pi<float>()/180.0f*values;

    /*for(auto child : m_childs)
        child.second->computeFlexibleRotate(values.z);*/

    this->setRotation(m_eulerRotations+values);
}


void SimpleNode::setRotation(glm::vec3 rotation, bool inRadians)
{
    if(!inRadians)
    {
        rotation.x = rotation.x*glm::pi<float>()/180.0;
        rotation.y = rotation.y*glm::pi<float>()/180.0;
        rotation.z = rotation.z*glm::pi<float>()/180.0;
    }

    m_eulerRotations = glm::mod(rotation+glm::pi<float>()*glm::vec3(1.0),glm::pi<float>()*2)-glm::pi<float>()*glm::vec3(1.0);

    //this->updateModelMatrix();
    this->askForUpdateModelMatrix();
}

void SimpleNode::setName(const std::string &name)
{
    m_name = name;
}

void SimpleNode::setRigidity(float rigidity)
{
    m_rigidity = rigidity;
}

glm::vec3 SimpleNode::getPosition() const
{
    return m_position;
}

glm::vec2 SimpleNode::getXYPosition() const
{
    return glm::vec2(m_position.x, m_position.y);
}

glm::vec3 SimpleNode::getGlobalPosition() const
{
    return m_globalPosition;
}

glm::vec2 SimpleNode::getGlobalXYPosition() const
{
    return glm::vec2(m_globalPosition.x, m_globalPosition.y);
}


glm::vec3 SimpleNode::getScale() const
{
    return m_scale;
}

glm::vec3 SimpleNode::getEulerRotation() const
{
    return m_eulerRotations;
}

const glm::mat4 &SimpleNode::getModelMatrix() const
{
    return m_modelMatrix;
}

const glm::mat4 &SimpleNode::getInvModelMatrix() const
{
    return m_invModelMatrix;
}

NodeTypeId SimpleNode::getId() const
{
    return m_id;
}

const std::string &SimpleNode::getName() const
{
    return m_name;
}

float SimpleNode::getRigidity() const
{
    return m_rigidity;
}

SimpleNode* SimpleNode::getParent()
{
    return m_parent;
}

SimpleNode* SimpleNode::getChild(const NodeTypeId id)
{
    auto childsIt = m_childs.find(id);
    if(childsIt == m_childs.end())
        return (nullptr);
    return childsIt->second;
}

SimpleNode* SimpleNode::getChildByName(const std::string &name, bool recursiveSearch)
{
    for(auto child : m_childs)
    {
        if(child.second->getName() == name)
            return child.second;

        if(recursiveSearch)
        {
            SimpleNode* res = child.second->getChildByName(name,true);
            if(res != nullptr)
                return res;
        }
    }
    return (nullptr);
}


void SimpleNode::getNodesByName(std::map<std::string, SimpleNode*> &namesAndResMap)
{
    if(!m_name.empty())
    {
        auto res = namesAndResMap.find(m_name);
        if(res != namesAndResMap.end())
            namesAndResMap.insert_or_assign(res, m_name, this);
    }

    for(auto child : m_childs)
        child.second->getNodesByName(namesAndResMap);
}

/*std::list<SimpleNode*> SimpleNode::getAllChilds()
{
    return m_childs.
}*/


void SimpleNode::setId(const NodeTypeId id)
{
    m_id = id;
}

void SimpleNode::setParent(SimpleNode *p)
{
    if(m_parent != p)
    {
        this->stopListeningTo(m_parent);
        m_parent = p;
        if(m_parent != nullptr)
        {
            ///this->setScene(m_parent->getScene());
            this->startListeningTo(m_parent);
        }
        this->updateGlobalPosition();
    }
}

SimpleNode* SimpleNode::nodeAllocator(NodeTypeId id)
{
    return new SimpleNode(id);
}

NodeTypeId SimpleNode::generateId()
{
    return m_curNewId++;
}


void SimpleNode::update(const Time &elapsedTime)
{
    if(m_needToUpdateModelMat)
        this->updateModelMatrix();

    for(auto node : m_childs)
        node.second->update(elapsedTime);
}

void SimpleNode::updateGlobalPosition()
{
    if(m_parent != nullptr)
    {
        if(m_rigidity != 1.0)
        {
            m_globalPosition = m_parent->getGlobalPosition() + m_position;
        }
        else
        {
            glm::vec4 pos    = m_parent->getModelMatrix() * glm::vec4(m_position,1.0);
            m_globalPosition = {pos.x,pos.y,pos.z};
        }
    }
    else
        m_globalPosition = m_position;

    //this->updateModelMatrix();
    this->askForUpdateModelMatrix();
}

void SimpleNode::askForUpdateModelMatrix()
{
    m_needToUpdateModelMat = true;

}

void SimpleNode::updateModelMatrix()
{
    if(m_rigidity != 1.0 && m_parent != nullptr)
        m_modelMatrix = glm::translate(glm::mat4(1.0), m_parent->getGlobalPosition());
    else
        m_modelMatrix = glm::mat4(1.0);


    m_modelMatrix = glm::translate(m_modelMatrix, m_position);
    m_modelMatrix = glm::rotate(m_modelMatrix, m_eulerRotations.x, glm::vec3(1.0,0.0,0.0));
    m_modelMatrix = glm::rotate(m_modelMatrix, m_eulerRotations.y, glm::vec3(0.0,1.0,0.0));
    m_modelMatrix = glm::rotate(m_modelMatrix, m_eulerRotations.z, glm::vec3(0.0,0.0,1.0));
    m_modelMatrix = glm::scale(m_modelMatrix, m_scale);

    if(m_rigidity == 1.0 && m_parent != nullptr)
    {
        m_modelMatrix       = m_parent->getModelMatrix() * m_modelMatrix;
      //  m_invModelMatrix    = m_parent->getInvModelMatrix();
    }// else
        m_invModelMatrix = glm::mat4(1.0);

    m_invModelMatrix = glm::scale(m_invModelMatrix, 1.0f/m_scale);
    m_invModelMatrix = glm::rotate(m_invModelMatrix, -m_eulerRotations.z, glm::vec3(0.0,0.0,1.0));
    m_invModelMatrix = glm::rotate(m_invModelMatrix, -m_eulerRotations.y, glm::vec3(0.0,1.0,0.0));
    m_invModelMatrix = glm::rotate(m_invModelMatrix, -m_eulerRotations.x, glm::vec3(1.0,0.0,0.0));
    m_invModelMatrix = glm::translate(m_invModelMatrix, -m_position);

    if(m_rigidity != 1.0 && m_parent != nullptr)
        m_invModelMatrix = glm::translate(m_invModelMatrix, -m_parent->getGlobalPosition());

    if(m_rigidity == 1.0 && m_parent != nullptr)
        m_invModelMatrix = m_invModelMatrix * m_parent->getInvModelMatrix();

    m_needToUpdateModelMat = false;
    this->sendNotification(Notification_NodeMoved);
}


/*{
if(m_rigidity != 1.0 && m_parent != nullptr)
        m_modelMatrix = glm::translate(glm::mat4(1.0), m_parent->getGlobalPosition());
    else
        m_modelMatrix = glm::mat4(1.0);


    m_modelMatrix = glm::translate(m_modelMatrix, m_position);
    m_modelMatrix = glm::rotate(m_modelMatrix, m_eulerRotations.x, glm::vec3(1.0,0.0,0.0));
    m_modelMatrix = glm::rotate(m_modelMatrix, m_eulerRotations.y, glm::vec3(0.0,1.0,0.0));
    m_modelMatrix = glm::rotate(m_modelMatrix, m_eulerRotations.z, glm::vec3(0.0,0.0,1.0));
    m_modelMatrix = glm::scale(m_modelMatrix, m_scale);

    if(m_rigidity == 1.0 && m_parent != nullptr)
    {
        m_modelMatrix       = m_parent->getModelMatrix() * m_modelMatrix;
        m_invModelMatrix    = m_parent->getInvModelMatrix();
    } else
        m_invModelMatrix = glm::mat4(1.0);

    m_invModelMatrix = glm::scale(m_invModelMatrix, 1.0f/m_scale);
    m_invModelMatrix = glm::rotate(m_invModelMatrix, -m_eulerRotations.z, glm::vec3(0.0,0.0,1.0));
    m_invModelMatrix = glm::rotate(m_invModelMatrix, -m_eulerRotations.y, glm::vec3(0.0,1.0,0.0));
    m_invModelMatrix = glm::rotate(m_invModelMatrix, -m_eulerRotations.x, glm::vec3(1.0,0.0,0.0));
    m_invModelMatrix = glm::translate(m_invModelMatrix, -m_position);

    if(m_rigidity != 1.0 && m_parent != nullptr)
        m_invModelMatrix = glm::translate(m_invModelMatrix, -m_parent->getGlobalPosition());

    m_needToUpdateModelMat = false;
    this->sendNotification(Notification_NodeMoved);
}*/


void SimpleNode::computeFlexibleMove(glm::vec3 m)
{
    ///Need to add partial rigidity
    if(m_rigidity != 1.0)
    {
        glm::vec2 p(m_position.x,m_position.y);
        float l = glm::length(p);
        glm::vec2 np = l*glm::normalize(p-glm::vec2(m.x,m.y));
        float dot = glm::dot(p,np);

        float wantedRotation = glm::atan(np.y,np.x)-glm::pi<float>()*0.5;
        this->rotate(wantedRotation-m_curFlexibleRotation,{0,0,1});
        m_curFlexibleRotation = wantedRotation;

        glm::vec3 mp = m+glm::vec3(np.x-p.x,np.y-p.y,0);
        for(auto child : m_childs)
            child.second->computeFlexibleMove(mp);

        this->setPosition(np);

    } else
        for(auto child : m_childs)
            child.second->computeFlexibleMove(m);
}

/*void SimpleNode::computeFlexibleRotate(float rot)
{
    if(m_rigidity != 1.0)
    {
        float c = glm::cos(rot);
        float s = glm::sin(rot);

        glm::vec2 p(m_position.x,m_position.y);
        glm::vec2 np(p.x*c - p.y*s, p.x*s + p.y*c);
        auto v = np-p;

        this->computeFlexibleMove({v.x,v.y,0.0f});
    } else
        for(auto child : m_childs)
            child.second->computeFlexibleRotate(rot);
}*/

void SimpleNode::notify(NotificationSender* sender, NotificationType type,
                       size_t dataSize, char* data)
{
    if(sender == m_parent)
    {
        if(type == Notification_NodeMoved)
            this->updateGlobalPosition();
        if(type == Notification_SenderDestroyed)
        {
            m_parent = nullptr;
            this->updateGlobalPosition();
        }
    }
}

}
