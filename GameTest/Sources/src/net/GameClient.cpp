#include "net/GameClient.h"

#include "net/NetMessageTypes.h"
#include "PouEngine/Types.h"
#include "PouEngine/tools/Profiler.h"
#include "PouEngine/core/MessageBus.h"

#include "net/GameServer.h"
#include "logic/GameMessageTypes.h"

//const float GameClient::CLIENTWORLD_SYNC_DELAY = 0.1;

const int       GameClient::TICKRATE    = 60;
const pou::Time GameClient::TICKDELAY(1.0f/GameClient::TICKRATE);
const int       GameClient::SYNCRATE    = 30;
const pou::Time GameClient::SYNCDELAY(1.0f/GameClient::SYNCRATE);
const float     GameClient::INTERPOLATIONDELAY = 0.0f;
const uint32_t  GameClient::MAX_PLAYER_REWIND = 200;

GameClient::GameClient() :
    //m_world(true, false),
    m_curWorldId(0),
    m_isWaitingForWorldSync(false),
    m_curCursorPos(0),
    m_remainingTime(0)
{
    initializeNetMessages();
    pou::NetEngine::setSyncDelay(GameServer::TICKRATE/GameServer::SYNCRATE);
    pou::NetEngine::setMaxRewindAmount(200);

    m_lastPlayerWalkDirection = glm::vec2(0);
    m_lastServerAckTime = (uint32_t)(-1);

    pou::MessageBus::addListener(this, GameMessageType_CharacterDamaged);
}

GameClient::~GameClient()
{
    this->cleanup();
}

bool GameClient::create(unsigned short port)
{
    m_client = std::move(pou::NetEngine::createClient());
    m_client->create(port);

    return (true);
}

void GameClient::cleanup()
{
    this->disconnectFromServer();

    if(m_client)
    {
        m_client->destroy();
        m_client.reset();
    }
}

void GameClient::disconnectionCleanup()
{
    m_isWaitingForWorldSync = false;
    m_curWorldId = 0;

    m_world.reset();
    //m_world.destroy();
}

bool GameClient::connectToServer(const pou::NetAddress &address)
{
    if(!m_client)
        return (false);

    this->disconnectFromServer();

    return m_client->connectToServer(address);
}

bool GameClient::disconnectFromServer()
{
    bool r = true;

    if(m_client)
        r = r & m_client->disconnectFromServer();

    this->disconnectionCleanup();

    return r;
}

const pou::NetAddress &GameClient::getServerAddress() const
{
    return m_client->getServerAddress();
}

Player* GameClient::getPlayer()
{
    if(!m_client || m_curWorldId == 0)
        return (nullptr);

    if(!m_world)
        return (nullptr);

    return m_world->getSyncPlayer(m_curPlayerId);
}


void GameClient::update(const pou::Time &elapsedTime)
{
    if(!m_client)
        return;

    m_remainingTime += elapsedTime;

    if(m_remainingTime < GameClient::TICKDELAY)
        return;

    float nbrTicks = (int)(m_remainingTime/GameClient::TICKDELAY);
    auto tickedElapsedTime = nbrTicks*GameClient::TICKDELAY;

    m_client->update(tickedElapsedTime);

    std::list<std::shared_ptr<pou::NetMessage> > netMessages;
    m_client->receivePackets(netMessages);

    for(auto &msg : netMessages)
        this->processMessage(msg);

    pou::Profiler::pushClock("Update client world");
    if(m_client->isConnected())
        this->updateWorld(tickedElapsedTime);
    else
        m_remainingTime -= tickedElapsedTime;
    pou::Profiler::popClock();
}

void GameClient::render(pou::RenderWindow *renderWindow)
{
    if(m_world)
        m_world->render(renderWindow);
}

void GameClient::sendMsgTest(bool reliable, bool forceSend)
{
    auto testMsg = std::dynamic_pointer_cast<NetMessage_Test>(pou::NetEngine::createNetMessage(NetMessageType_Test));
    testMsg->test_value = glm::linearRand(0,100);
    testMsg->isReliable = reliable;

    m_client->sendMessage(testMsg, forceSend);
    pou::Logger::write("Client send test message with value:"+std::to_string(testMsg->test_value)+" and id: "+std::to_string(testMsg->id));
}



void GameClient::addPlayerAction(const PlayerAction &action)
{
    if(!m_client || m_curWorldId == 0)
        return;

    /**if(action.actionType == PlayerActionType_CursorMove)
    {
        m_curCursorPos = action.direction;
        auto player = m_world.getPlayer(m_curPlayerId);
        if(player)
            player->processAction(action);
        return;
    }**/

    if(action.actionType == PlayerActionType_Walk)
        m_lastPlayerWalkDirection = action.direction;

    if(m_world)
        m_world->addPlayerAction(m_curPlayerId, action);
}

/*void GameClient::playerCursor(glm::vec2 direction)
{
    m_curCursorPos = direction;

    if(!m_client || m_curWorldId == 0)
        return;

    auto player = m_world.getPlayer(m_curPlayerId);
    if(player)
    {
        PlayerAction action;
        action.actionType = PlayerActionType_CursorMove;
        action.direction = direction;
        player->processAction(action);
        ///player->lookAt(direction);
    }
}

void GameClient::playerLook(glm::vec2 direction)
{
    if(!m_client || m_curWorldId == 0)
        return;

    PlayerAction playerAction;
    playerAction.actionType = PlayerActionType_Look;
    playerAction.direction  = direction;

    m_world.addPlayerAction(m_curPlayerId, playerAction);
}

void GameClient::playerWalk(glm::vec2 direction)
{
    if(!m_client || m_curWorldId == 0)
        return;

    if(m_lastPlayerWalkDirection != direction || !GameServer::USEREWIND)
    {
        m_lastPlayerWalkDirection = direction;
        PlayerAction playerAction;
        playerAction.actionType = PlayerActionType_Walk;
        playerAction.direction  = direction;

        m_world.addPlayerAction(m_curPlayerId, playerAction);
    }
}

void GameClient::playerDash(glm::vec2 direction)
{
    if(!m_client || m_curWorldId == 0)
        return;

    PlayerAction playerAction;
    playerAction.actionType = PlayerActionType_Dash;
    playerAction.direction  = direction;

    m_world.addPlayerAction(m_curPlayerId, playerAction);
}

void GameClient::playerAttack(glm::vec2 direction)
{
    if(!m_client || m_curWorldId == 0)
        return;

    PlayerAction playerAction;
    playerAction.actionType = PlayerActionType_Attack;
    playerAction.direction  = direction;

    m_world.addPlayerAction(m_curPlayerId, playerAction);
}

void GameClient::playerUseItem(size_t itemNbr)
{
    if(!m_client || m_curWorldId == 0)
        return;

    PlayerAction playerAction;
    playerAction.actionType = PlayerActionType_UseItem;
    playerAction.value      = itemNbr;

    m_world.addPlayerAction(m_curPlayerId, playerAction);
}*/

///Protected

void GameClient::processMessage(std::shared_ptr<pou::NetMessage> msg)
{
    if(!msg)
        return;

    switch(msg->type)
    {
        case NetMessageType_Test:{
            auto castMsg = std::dynamic_pointer_cast<NetMessage_Test>(msg);
            pou::Logger::write("Client received test message with value: "+std::to_string(castMsg->test_value)+" and id: "+std::to_string(castMsg->id));
        }break;

        case NetMessageType_ConnectionStatus:{
            auto castMsg = std::dynamic_pointer_cast<pou::NetMessage_ConnectionStatus>(msg);
            if(castMsg->connectionStatus == pou::ConnectionStatus_Connected)
            {
                // Do something, probably
            }
            else if(castMsg->connectionStatus == pou::ConnectionStatus_Disconnected)
                this->disconnectionCleanup();
        }break;


        case NetMessageType_WorldInit:{
            auto castMsg = std::dynamic_pointer_cast<NetMessage_WorldInit>(msg);
            m_curWorldId = castMsg->world_id;
            m_curPlayerId = castMsg->player_id;
            m_isWaitingForWorldSync = false;

            m_world = std::make_unique<GameWorld>(true);
            m_world->generateFromMsg(castMsg);

            pou::Logger::write("Received world #"+std::to_string(m_curWorldId));
        }break;

        case NetMessageType_WorldSync:{
            auto castMsg = std::dynamic_pointer_cast<NetMessage_WorldSync>(msg);
            if(m_world)
                m_world->getSyncComponent()->syncWorldFromMsg(castMsg, m_curPlayerId, m_client->getRTT());

            if(uint32less(m_lastServerAckTime, castMsg->clientTime))
                m_lastServerAckTime = castMsg->clientTime;
        }break;
    }
}

void GameClient::updateWorld(const pou::Time &elapsedTime)
{
    if(m_world)
    {
        while(m_remainingTime > GameClient::TICKDELAY)
        {
            m_world->update(GameClient::TICKDELAY);
            m_remainingTime -= GameClient::TICKDELAY;
        }
    } else
        m_remainingTime = pou::Time(0);


    if(m_curWorldId == 0 && !m_isWaitingForWorldSync)
    {
        m_isWaitingForWorldSync = true;
        //auto msg = std::dynamic_pointer_cast<NetMessage_AskForWorldSync>(pou::NetEngine::createNetMessage(NetMessageType_AskForWorldSync));
        auto msg = std::dynamic_pointer_cast<NetMessage_PlayerSync>(pou::NetEngine::createNetMessage(NetMessageType_PlayerSync));
        msg->isReliable = true;
        msg->lastSyncTime = -1;
        msg->localTime = 0;
        m_client->sendMessage(msg, true);

        return;
    } else if(m_curWorldId != 0 && m_world) {
        if(m_syncTimer.update(elapsedTime) || !m_syncTimer.isActive())
        {
            //auto msg = std::dynamic_pointer_cast<NetMessage_AskForWorldSync>(pou::NetEngine::createNetMessage(NetMessageType_AskForWorldSync));
            auto msg = std::dynamic_pointer_cast<NetMessage_PlayerSync>(pou::NetEngine::createNetMessage(NetMessageType_PlayerSync));

            /*if(m_lastPlayerWalkDirection != glm::vec2(0))
            {
                PlayerAction playerAction;
                playerAction.actionType = PlayerActionType_Walk;
                playerAction.direction  = m_lastPlayerWalkDirection;
                m_world.addPlayerAction(m_curPlayerId, playerAction);
            }*/

            //Could add condition to do this only if attackMode is on
            /**{
                PlayerAction playerAction;
                playerAction.actionType = PlayerActionType_CursorMove;
                playerAction.direction  = glm::normalize(m_curCursorPos);
                m_world.addPlayerAction(m_curPlayerId, playerAction);
            }**/

            //std::cout<<"Last Server Ack:"<<m_lastServerAckTime<<" vs cur time:"<<m_world.getLocalTime()<<std::endl;
            m_world->getSyncComponent()->createPlayerSyncMsg(msg, m_curPlayerId, m_lastServerAckTime);
            m_client->sendMessage(msg, true);
            m_syncTimer.reset(GameClient::SYNCDELAY);
        }
    }
}

void GameClient::notify(pou::NotificationSender*, int notificationType, void* data)
{
    if(!m_world)
        return;

    auto player = m_world->getSyncPlayer(m_curPlayerId);
    if(!player)
        return;

    auto playerEventMsg = std::dynamic_pointer_cast<NetMessage_PlayerEvent>(pou::NetEngine::createNetMessage(NetMessageType_PlayerEvent));

    if(notificationType == GameMessageType_CharacterDamaged)
    {
        GameMessage_CharacterDamaged *gameMsg = static_cast<GameMessage_CharacterDamaged*>(data);

        if(gameMsg->character == player)
            return;

        playerEventMsg->isReliable  = true;
        playerEventMsg->eventType   = PlayerEventType_CharacterDamaged;
        playerEventMsg->syncId      = gameMsg->character->getCharacterSyncId();
        playerEventMsg->direction   = gameMsg->direction;
        playerEventMsg->amount      = gameMsg->damages;

        m_client->sendMessage(playerEventMsg);
    }
}
