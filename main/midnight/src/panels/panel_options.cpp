//
//  panel_options
//  midnight
//
//  Created by Chris Wild on 02/12/2017.
//

#include "../extensions/CustomDirector.h"
#include "../cocos.h"

#include "panel_options.h"
#include "panel_mainmenu.h"


#include "../system/resolutionmanager.h"
#include "../system/moonring.h"
#include "../system/settingsmanager.h"
#include "../system/keyboardmanager.h"
#include "../system/panelmanager.h"

#include "../ui/uihelper.h"
#include "../ui/uitextmenuitem.h"
#include "../ui/uitextmenu.h"
#include "../ui/uioptionitem.h"

#include "../frontend/language.h"

USING_NS_CC;
USING_NS_CC_UI;

static const char* values_onoff[]               = {"OFF","ON"};
static const char* values_yesno[]               = {"NO","YES"};
static const char* values_movement[]            = {"SWIPE & PUSH","PUSH","SWIPE","PRESS LOOK/SWIPE MOVE"};
static const char* values_think[]               = {"SWIPE LEFT/RIGHT","PUSH","ARROWS ONLY"};
static const char* values_compass_delay[]       = {"OFF","NORMAL PRESS","SHORT PRESS","LONG PRESS"};
static const char* values_compass_feedback[]    = {"OFF","LOW","MEDIUM","HIGH"};
static const char* values_slowfast[]            = {"SLOW","FAST"};
static const char* values_fullbrief[]           = {"BRIEF","FULL"};
static const char* values_keyboard[]            = {"CLASSIC","NEW"};
static const char* values_novella[]             = {"EBOOK","PDF"};
static const char* values_screen[]              = {"FULLSCREEN","SMALL","MEDIUM","LARGE"};
static const char* values_screen2[]             = {"FULLSCREEN - NOT SUPPORTED","SMALL","MEDIUM","LARGE"};



/*
 DISPLAY
    MODE                FULL, Window1, Window2, Window3
    TRANSITIONS         OFF, ON
 GAME
    AUTO FIGHT          ON, OFF
    AUTO HIDE           ON, OFF
    BATTLE REPORT       SLOW, FAST
    NIGHT DISPLAY       BRIEF, FULL
 CONTROL
    COMPASS DELAY
    COMPASS VIBRATE
    THINK PAGING
    NAVIGATION STYLE    SWIPE

 
 HELP
    TUTORIAL
    MOVEMENT INDICATORS
    NOVELLA
 
 MAIN MENU
 
 */
static uitextmenuitem items_main[] = {
    { ID_MENU_DISPLAY,                  {"DISPLAY"},            KEYCODE(D), "D" },
    { ID_MENU_GAME,                     {"GAME"},               KEYCODE(G), "G" },
    { ID_MENU_CONTROL,                  {"CONTROL"},            KEYCODE(C), "C" },
    { ID_MENU_HELP,                     {"HELP"},               KEYCODE(H), "H" },
    { ID_HOME,                          {"MAIN MENU"},          KEYCODE(ESCAPE), "ESC" }
};

static uitextmenuitem items_display[] = {
#if defined(_OS_DESKTOP_)
    { ID_OPTION_SCREENMODE,             {"SCREEN MODE"},        KEYCODE(1), "1", TB_DOUBLE, },
#endif
    { ID_OPTION_TRANSITIONS,            {"SCREEN TRANSITIONS"}, KEYCODE(2), "2", TB_DOUBLE },
    { ID_OPTION_FLIPSCREEN,             {"FLIP SCREEN"},        KEYCODE(3), "3", TB_DOUBLE },
};

static uitextmenuitem items_game[] = {
    { ID_OPTION_AUTO_FIGHT,             {"AUTO FIGHT"},         KEYCODE(1), "1", TB_DOUBLE },
    { ID_OPTION_AUTO_UNHIDE,            {"AUTO UNHIDE"},        KEYCODE(2), "2", TB_DOUBLE },
    { ID_OPTION_NIGHT_DISPLAY,          {"NIGHT DISPLAY"},      KEYCODE(3), "3", TB_DOUBLE },
    { ID_OPTION_BATTLE_FULL,            {"BATTLE REPORT "},     KEYCODE(4), "4", TB_DOUBLE },
};

static uitextmenuitem items_control[] = {
    { ID_OPTION_COMPASS_DELAY,          {"COMPASS DELAY"},      KEYCODE(1), "1", TB_DOUBLE },
    { ID_OPTION_COMPASS_FEEDBACK,       {"COMPASS VIBRATE"},    KEYCODE(2), "2", TB_DOUBLE },
    { ID_OPTION_NAVIGATION,             {"NAVIGATION STYLE"},   KEYCODE(3), "3", TB_DOUBLE },
    { ID_OPTION_KEYBOARD_STYLE,         {"KEYBOARD STYLE"},     KEYCODE(4), "4", TB_DOUBLE },
};

static uitextmenuitem items_help[] = {
    { ID_OPTION_TUTORIAL,               {"TUTORIAL"},           KEYCODE(1), "1", TB_DOUBLE },
#if !defined(_OS_DESKTOP_)
    { ID_OPTION_MOVE_INDICATORS,        {"MOVEMENT INDICATORS"},KEYCODE(2), "2", TB_DOUBLE },
#endif
#if defined(_OS_IOS_) || defined(_OS_OSX_)
    { ID_OPTION_NOVELLA,                {"NOVELLA"},            KEYCODE(3), "3", TB_DOUBLE },
#endif
};

static option_t options[] = {
    
    {   ID_OPTION_AUTO_FIGHT,       OPT_BOOL,    0, values_onoff,               nullptr, false },
    {   ID_OPTION_AUTO_UNHIDE,      OPT_BOOL,    0, values_onoff,               nullptr, false },
    {   ID_OPTION_MOVE_INDICATORS,  OPT_BOOL,    0, values_yesno,               nullptr, false },
    {   ID_OPTION_TUTORIAL,         OPT_BOOL,    0, values_onoff,               nullptr, false },
    {   ID_OPTION_NAVIGATION,       OPT_NUMBER,  4, values_movement,            nullptr, false },
    {   ID_OPTION_TRANSITIONS,      OPT_BOOL,    0, values_onoff,               nullptr, false },
    {   ID_OPTION_FLIPSCREEN,       OPT_BOOL,    0, values_onoff,               nullptr, false },
    
    {   ID_OPTION_NOVELLA,          OPT_BOOL,    0, values_novella,             nullptr, false },
    
    {   ID_OPTION_COMPASS_DELAY,    OPT_NUMBER,  4, values_compass_delay,       nullptr, false },
    {   ID_OPTION_COMPASS_FEEDBACK, OPT_NUMBER,  4, values_compass_feedback,    nullptr, false },
    {   ID_OPTION_NIGHT_DISPLAY,    OPT_BOOL,    0, values_slowfast,            nullptr, false },
    {   ID_OPTION_BATTLE_FULL,      OPT_BOOL,    0, values_fullbrief,           nullptr, false },
    
    {   ID_OPTION_SCREENMODE,       OPT_NUMBER,  4, values_screen,              nullptr, false },

    {   ID_OPTION_KEYBOARD_STYLE,   OPT_NUMBER,  2, values_keyboard,            nullptr, false },
    
    {   ID_HOME,                    OPT_NONE,    0, nullptr,                    nullptr, false },
};

bool panel_options::init()
{
    if ( !uipanel::init() )
    {
        return false;
    }

    menu2_background = NULL;
    fields.clear();
    
    setBackgroundToHeight("screens/misc/main_menu.png");

    auto logo = ImageView::create(IMAGE_LOGO,Widget::TextureResType::LOCAL);
    logo->setLocalZOrder(ZORDER_FAR+1);
    uihelper::AddTopCenter(safeArea,logo,0,RES(32));
    
    CreateMenu1();
    
    SET_OPTION(ID_OPTION_AUTO_FIGHT,autofight);
    SET_OPTION(ID_OPTION_AUTO_UNHIDE,autounhide);
    SET_OPTION(ID_OPTION_MOVE_INDICATORS,showmovementindicators);
    SET_OPTION(ID_OPTION_TUTORIAL,tutorial);
    SET_OPTION(ID_OPTION_NAVIGATION,nav_mode);
    SET_OPTION(ID_OPTION_TRANSITIONS,screentransitions);
    SET_OPTION(ID_OPTION_FLIPSCREEN,flipscreen);
    
    SET_OPTION(ID_OPTION_NOVELLA,novella_pdf);
    
    SET_OPTION(ID_OPTION_COMPASS_DELAY,compass_delay);
    SET_OPTION(ID_OPTION_COMPASS_FEEDBACK,compass_feedback);
    SET_OPTION(ID_OPTION_NIGHT_DISPLAY,night_display_fast);
    SET_OPTION(ID_OPTION_BATTLE_FULL,night_battle_full);
    
    SET_OPTION(ID_OPTION_SCREENMODE, screen_mode);
    SET_OPTION(ID_OPTION_KEYBOARD_STYLE, keyboard_mode);
    
    if ( !mr->settings->fullscreensupported )
    {
        findOption(ID_OPTION_SCREENMODE)->text = values_screen2;
    }
    
    SetMenu(ID_MENU_DISPLAY);
    
    initialScreenMode = mr->settings->screen_mode ;
  
    return true;
}

void panel_options::OnMenuNotification( const uinotificationinterface* sender, menueventargs* args )
{
    int tag = args->menuitem->id;
    
    if ( tag == ID_HOME )
    {
#if defined(_OS_DESKTOP_)
        if(initialScreenMode != mr->settings->screen_mode)
        {
            changeDisplayMode();
            return;
        }
#endif
        Exit();
        mr->settings->Save();
        return;
    } else if ( tag == ID_MENU_GAME ) {
        SetMenu(tag);
        return;
    }else if ( tag == ID_MENU_DISPLAY ) {
        SetMenu(tag);
        return;
    } else if ( tag == ID_MENU_CONTROL ) {
        SetMenu(tag);
        return;
    } else if ( tag == ID_MENU_HELP ) {
        SetMenu(tag);
        return;
    } else if ( tag == ID_CLOSE || tag == ID_HELP_CLOSE ) {
        return;
    }
    
    auto option = findOption(tag);
    if(option==nullptr)
        return;;

    if ( option->type == OPT_BOOL ) {
        BOOL* value = (BOOL*) option->var;
        *value = ! *value;
    }
    
    else if ( option->type == OPT_NUMBER ) {
        int* value = (int*) option->var;
        *value = *value +1;
        if ( *value >= option->max )
            *value=0;
    }
    
    SetValues();
    
    if ( option->id == ID_OPTION_TUTORIAL ) {
        if ( mr->settings->tutorial )
            showHelpWindow(HELP_TUTORIAL_ON);
        else
            showHelpWindow(HELP_TUTORIAL_OFF);
    }
    
}

option_t* panel_options::findOption(int id)
{
    for ( int ii=0; ii<NUMELE(options); ii++ )
    {
        if ( options[ii].id == id )
        {
            return &options[ii];
        }
    }
    return nullptr;
}

void panel_options::disableOption(int id)
{
    auto option = findOption(id);
    if(option!=nullptr)
    {
        option->disabled = true;
    }
}

void panel_options::SetMenu ( int id )
{
    if ( id == ID_MENU_GAME ) {
        SetMenu(items_game,NUMELE(items_game));
        SetValues();
        return;
    }else if ( id == ID_MENU_DISPLAY ) {
        SetMenu(items_display,NUMELE(items_display));
        SetValues();
        return;
    } else if ( id == ID_MENU_CONTROL ) {

        // check if keyboard available
        //if ( !ui->IsKeyboardAvailable() )
        {
            disableOption(ID_OPTION_KEYBOARD_STYLE);
        }

        // check if compass feedback available
//        if ( !ui->IsVibrationAvailable()) {
//            menu2->DisableButton(ID_OPTION_COMPASS_FEEDBACK);
//            gl->settings.compass_feedback=CF_COMPASS_OFF;
//        }
//

        SetMenu(items_control,NUMELE(items_control));
        
#if defined(_OS_DESKTOP_)
        mr->settings->showmovementindicators=FALSE;
#endif
        
        SetValues();
        return;
    } else if ( id == ID_MENU_HELP ) {
        SetMenu(items_help,NUMELE(items_help));
        
#if !defined(_OS_IOS_) && !defined(_OS_OSX_)
        mr->settings->novella_pdf=TRUE;
#endif
        
        SetValues();
        return;
    }
}

void panel_options::SetValues ( void )
{
    for ( u32 ii=0; ii<NUMELE(options); ii++ ) {

        int item=0;
        if ( options[ii].type == OPT_BOOL ) {
            BOOL* value = (BOOL*) options[ii].var ;
            item = *value ? 1 : 0 ;
        }
        if ( options[ii].type == OPT_NUMBER ) {
            int* value = (int*) options[ii].var ;
            item = *value ;
        }
        
        if ( options[ii].type != OPT_NONE ) {

            auto field = optionControls.at(options[ii].id);
            if ( field != nullptr  )
                field->setValue(options[ii].text[item]);
        }
        
    }
}

void panel_options::CreateMenu1()
{
    f32 width = RES(512-32);
    
    f32 contentWidth = safeArea->getContentSize().width;
    f32 contentHeight = safeArea->getContentSize().height;
    
    auto menu = uitextmenu::create(width, items_main, NUMELE(items_main) );
    menu->setAnchorPoint(uihelper::AnchorCenter);
    menu->setPosition(Vec2(contentWidth/4,contentHeight/2));
    menu->setLocalZOrder(ZORDER_UI);
    safeArea->addChild(menu);

    menu->setNotificationCallback ( [&](const uinotificationinterface* s, uieventargs* e) {
        this->OnMenuNotification( s, (menueventargs*)e );
    });
    
}

void panel_options::SetMenu( uitextmenuitem items[], int elements )
{
    f32 width = RES(512-32);
    if(isPhoneScaleEnabled())
    {
        width+=RES(128+64);
    }
    
    f32 contentWidth = safeArea->getContentSize().width;
    
    if ( menu2 != nullptr )  {
        menu2->removeAllChildren();
        menu2->removeFromParent();
    }

    menu2 = Menu::create();
    menu2->setLocalZOrder(ZORDER_UI);
    
    u32 gapY = PHONE_SCALE(RES(5));
    
    this->addChild(menu2);
    
    fields.clear();
    optionControls.clear();
    
    f32 height=0;
    for ( int ii=0; ii<elements; ii++ ) {
        
        uitextmenuitem* item = &items[ii];
        
        auto option = findOption(item->id);
        if(option->disabled)
        {
            continue;
        }
        
        auto button = uioptionitem::create( width, item );
        button->setTag(item->id);
        
        // find the options
        optionControls.insert(item->id, button);
        
        auto menuItem = MenuItemNode::create(button);
        auto r = menuItem->getBoundingBox();
        height += r.size.height + gapY;

        menuItem->setCallback([&,item](cocos2d::Ref *sender) {
            menueventargs args;
            args.menuitem = item;
            OnMenuNotification( nullptr, &args );
        } );
        
        menu2->addChild(menuItem);
        fields.pushBack(menuItem);
    }
    
    menu2->setContentSize(Size(width,height) );
  
    uihelper::PositionParentCenterLeft(menu2,contentWidth/2);
    
    // refresh positions
    auto offset = Vec2( width/2, height );
    for ( auto item : fields ) {
        f32 height = item->getBoundingBox().size.height;
        offset.y -= height/2;
        item->setAnchorPoint( uihelper::AnchorCenter );
        item->setPosition(offset);
        offset.y -=  (height/2) + gapY;
    }
    
}

#if defined(_OS_DESKTOP_)
void panel_options::changeDisplayMode()
{
    AreYouSure(CHANGE_DISPLAY_MSG,
        [&] {
            // yes
            mr->changeDisplayMode();
        },
        [&] {
            // no
            Exit();
            mr->settings->Save();
        });
}
#endif


