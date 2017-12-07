//
//  uipanel.cpp
//  midnight
//
//  Created by Chris Wild on 02/12/2017.
//

#include "../ui/uipopup.h"
#include "../Extensions/CustomDirector.h"
#include "uipanel.h"
#include "../frontend/resolutionmanager.h"
#include "../system/moonring.h"


USING_NS_CC;


bool uipanel::init()
{
    if ( !Scene::init() )
    {
        return false;
    }
    
    mr = moonring::instance();
    
    return true;
}

void uipanel::OnNotification( Ref* element, u32 id )
{
}

void uipanel::SetBackground( LPCSTR background )
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto p = Sprite::create((LPCSTR)background);
    p->setPosition(Vec2(0,0));
    p->setAnchorPoint(Vec2(0,0));
    
    f32 scale = visibleSize.height / p->getContentSize().height ;
    
    p->setScale(scale, scale );
    addChild(p);

}

void uipanel::FadeExit(f32 delay )
{
    if ( delay == 0 ) {
        CustomDirector *director = (CustomDirector *)Director::getInstance();
        director->popSceneWithTransition<TransitionCrossFade>(1.0);
        return;
    }
    
    
    this->scheduleOnce( [](float) {
        
        CustomDirector *director = (CustomDirector *)Director::getInstance();
        director->popSceneWithTransition<TransitionCrossFade>(1.0);
        
    }, delay, "delayed_fade_exit" );
}

void uipanel::AreYouSure ( LPCSTR text, MXVoidCallback ok )
{
    if ( popupWindow != nullptr ) {
        popupWindow->Close();
    }
    
    popupWindow = new uipopup( this, point(0,0), RES(600), text );
    popupWindow->onCancel = [&] {
        popupWindow = nullptr;
    };
    popupWindow->onOk = [&,ok] {
        popupWindow = nullptr;
        if ( ok != nullptr )
            ok();
    };
    popupWindow->Show();
    
}

void uipanel::OpenPDF(LPCSTR pdf)
{
#if defined(_OS_ANDROID_GOOGLE_DOCS_)
    char buffer[MAX_PATH];
    snprintf(buffer, MAX_PATH, "%s%s", PDF_VIEWER, pdf);
    Application::getInstance()->openURL(buffer);
#else
    Application::getInstance()->openURL(pdf);
#endif
}

void uipanel::FillBackground()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto p = Sprite::createWithSpriteFrameName("background_tile");
    p->setPosition( Vec2::ZERO );
    p->setAnchorPoint( Vec2::ZERO );
    f32 scaleY = visibleSize.height / p->getContentSize().height ;
    f32 scaleX = visibleSize.width / p->getContentSize().width ;
    p->setScale(scaleX, scaleY );
    addChild(p);
}
