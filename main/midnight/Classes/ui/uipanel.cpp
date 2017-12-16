//
//  uipanel.cpp
//  midnight
//
//  Created by Chris Wild on 02/12/2017.
//

#include "../ui/uipopup.h"
#include "../ui/uihelpwindow.h"
#include "../Extensions/CustomDirector.h"
#include "uipanel.h"
#include "../frontend/resolutionmanager.h"
#include "../system/moonring.h"
#include "../system/helpmanager.h"
#include "../system/configmanager.h"
#include "../ui/uihelper.h"

USING_NS_CC;


bool uipanel::init()
{
    if ( !Scene::init() )
    {
        return false;
    }
    
    mr = moonring::mikesingleton();
    
    return true;
}

void uipanel::OnNotification( Ref* element )
{
}

void uipanel::SetBackground( LPCSTR background )
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto p = Sprite::create((LPCSTR)background);
    f32 scale = visibleSize.height / p->getContentSize().height ;
    p->setScale(scale, scale );
    uihelper::AddCenter(this, p);

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
    f32 scaleY = visibleSize.height / p->getContentSize().height ;
    f32 scaleX = visibleSize.width / p->getContentSize().width ;
    p->setScale(scaleX, scaleY );
    uihelper::AddCenter(this, p);
    
}

BOOL uipanel::ShowHelpWindow ( helpid_t id, BOOL forceImportant, MXVoidCallback callback )
{
    helpmanager* help = mr->help;
    
    if ( !help->isAlways(id) ) {
        
        if ( !mr->config->tutorial )
            return TRUE;
        
        if ( help->isShown(id) )
            return TRUE;
        
        if ( help_pending && !help->isImportant(id) && !forceImportant )
            return FALSE;
    }
    
    if ( help->isImportant(id) || forceImportant ) {
        PopupHelpWindow(id,callback);
        return FALSE;
    }
    
    // show the tutorial help
    if ( !help->isShown(HELP_TUTORIAL) ) {
        PopupHelpWindow(HELP_TUTORIAL,callback);
    }
    
    // and set the required help to pending
    help_pending_id = id;
    help_pending=TRUE;
    
    HelpPending();

    return FALSE;
}

void uipanel::HelpPending()
{
    //    i_help->ShowEnable();
    //    uiaction* flash = new uiaFlash(300,10*1000);
    //    flash->delegate=this;
    //    flash->tag=9999;
    //    i_help->AddAction ( flash) ;
    //    a_help_timer->start(30*1000);
}

void uipanel::PopupHelpWindow ( helpid_t id, MXVoidCallback callback )
{
    help_pending = FALSE;
    help_pending_id = HELP_NONE ;
    
    //if ( a_help_timer ) {
    //    a_help_timer->stop();
    //}
    help_visible=id;
    
    if ( help_window != nullptr ) {
        help_window->Close();
    }
    
    help_window = new uihelpwindow( this, id );
    help_window->Show( [&,callback] {
        help_window = nullptr;
        if ( callback != nullptr )
            callback();
    });
    
}




