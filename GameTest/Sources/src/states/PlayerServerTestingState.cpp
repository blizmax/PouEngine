#include "states/PlayerServerTestingState.h"


#include "PouEngine/Types.h"
#include "PouEngine/utils/Clock.h"

#include "PouEngine/assets/AssetHandler.h"
#include "PouEngine/assets/SoundBankAsset.h"

#include "PouEngine/renderers/UiRenderer.h"

PlayerServerTestingState::PlayerServerTestingState() :
    m_firstEntering(true)
{
    //ctor
}

PlayerServerTestingState::~PlayerServerTestingState()
{
    this->leaving();
}


void PlayerServerTestingState::init()
{
    m_firstEntering = false;

    pou::SoundBanksHandler::loadAssetFromFile("../data/MasterSoundBank.bank");

    m_gameServer.create(46969,true);
    m_gameServer.generateWorld();

    int localClientNbr = m_gameServer.addLocalPlayer();
    m_localClientNbr = (size_t)localClientNbr;
    std::cout<<m_localClientNbr<<std::endl;

    m_gameUi.init();
}

void PlayerServerTestingState::entered()
{
    if(m_firstEntering)
        this->init();
}

void PlayerServerTestingState::leaving()
{
    m_gameServer.shutdown();
}

void PlayerServerTestingState::revealed()
{

}

void PlayerServerTestingState::obscuring()
{

}


void PlayerServerTestingState::handleEvents(const EventsManager *eventsManager)
{
    m_gameUi.handleEvents(eventsManager);

    if(eventsManager->keyReleased(GLFW_KEY_ESCAPE))
        m_manager->stop();

    if(eventsManager->isAskingToClose())
        m_manager->stop();

    glm::vec2 charDirection = {0,0};
    if(eventsManager->keyIsPressed(GLFW_KEY_S))
        charDirection.y = 1;
    if(eventsManager->keyIsPressed(GLFW_KEY_W))
        charDirection.y = -1;
    if(eventsManager->keyIsPressed(GLFW_KEY_A))
        charDirection.x = -1;
    if(eventsManager->keyIsPressed(GLFW_KEY_D))
        charDirection.x = 1;
    m_gameServer.playerWalk(m_localClientNbr, charDirection);
    //if(eventsManager->keyPressed(GLFW_KEY_LEFT_ALT))
    //    m_character->askToDash(charDirection);
    //m_character->walk(charDirection);


    if(eventsManager->keyIsPressed(GLFW_KEY_R))
        m_gameServer.rewindWorld(1,0);
}


void PlayerServerTestingState::update(const pou::Time &elapsedTime)
{
    //m_gameUi.updateCharacterLife(m_character->getAttributes().life,
    //                             m_character->getAttributes().maxLife);

    m_gameUi.update(elapsedTime);
    m_gameServer.update(elapsedTime);
}

void PlayerServerTestingState::draw(pou::RenderWindow *renderWindow)
{
    m_gameServer.render(renderWindow, m_localClientNbr);

    /*if(renderWindow->getRenderer(pou::Renderer_Scene) != nullptr)
    {
        pou::SceneRenderer *renderer = dynamic_cast<pou::SceneRenderer*>(renderWindow->getRenderer(pou::Renderer_Scene));
        if(m_scene)
            m_scene->render(renderer, m_camera);
    }*/

    if(renderWindow->getRenderer(pou::Renderer_Ui) != nullptr)
    {
        pou::UiRenderer *renderer = dynamic_cast<pou::UiRenderer*>(renderWindow->getRenderer(pou::Renderer_Ui));
        m_gameUi.render(renderer);
    }

}
