#include "cocos2d.h"
#include "ui/CocosGUI.h"

#include "panel_look.h"
#include "panel_think.h"

#include "../system/moonring.h"
#include "../system/configmanager.h"
#include "../system/helpmanager.h"
#include "../system/resolutionmanager.h"
#include "../system/tmemanager.h"

#include "../frontend/layout_id.h"
#include "../frontend/choose_id.h"
#include "../frontend/language.h"

#include "../ui/uihelper.h"
#include "../ui/uicommandwindow.h"

#include "../landscaping/LandscapeSky.h"
#include "../landscaping/LandscapeLand.h"
#include "../landscaping/LandscapeTerrain.h"
#include "../landscaping/LandscapeColour.h"

#include "../tme_interface.h"

#include "../TMEMapBuilder.h"

#include <iostream>
#include <string>
#include <algorithm>

USING_NS_CC;
using namespace tme;
using namespace cocos2d::ui;

enum tagid_t {
    TAG_NONE            = 0,
    TAG_MENU_COLLAPSE   = 1,
    TAG_DELAYED_SAVE    = 2,
    TAG_UNDO            = 3,
    TAG_UNDO_DONE       = 4,
    TAG_ENTER_TUNNEL    = 5,
    TAG_ENTER_TUNNEL_DONE=6,
    TAG_EXIT_TUNNEL     =7,
    TAG_EXIT_TUNNEL_DONE=8,
};


#define SHIELD_WIDTH    RES(192)
#define SHIELD_HEIGHT   RES(224)


#if defined(_LOM_)
#define DESCRIPTION_COLOUR  _clrWhite
#define SHIELD_X        RES(0)
#define SHIELD_Y        RES(16)
#define SHIELD_SCALE    1.0f
#endif

#if defined(_DDR_)
#define DESCRIPTION_COLOUR  _clrRed
#define SHIELD_X        RES(32)
#define SHIELD_Y        RES(-16)
#define SHIELD_SCALE    0.9f
#endif

panel_look::~panel_look()
{
    CC_SAFE_RELEASE_NULL(i_command_window);
}

bool panel_look::init()
{
    if ( !uipanel::init() )
    {
        return false;
    }
    
    // TODO: DDR Tunnel view
    
    // TODO: DDR TunnelGenerator
    options.generator = new LandscapeGenerator();
    options.colour = new LandscapeColour();
    options.showWater = false;
    options.showLand  = false;
    options.showTerrain = true ;
    options.debugMode = 0;
    options.landScaleX = 1.6f;
    options.landScaleY = 2.15f;
    options.debugLand=false;
    options.isMoving=false;
    options.isLooking=false;
    options.programState = mr->glProgramState;
    options.colour->options = &options;

    // Header area
    layHeader = LayerColor::create(Color4B(_clrWhite), getContentSize().width, RES(228) );
    layHeader->setLocalZOrder(ZORDER_FAR+1);
    uihelper::AddTopLeft(this,layHeader);
    
    f32 lblNameAdjust = 0;
    
    // Name Label
#if defined(_LOM_)
    lblName = Label::createWithTTF( uihelper::font_config_big, "" );
    lblName->getFontAtlas()->setAntiAliasTexParameters();
    lblName->setTextColor(Color4B::YELLOW);
    lblName->setLocalZOrder(ZORDER_DEFAULT);
    uihelper::AddTopLeft(safeArea,lblName,RES(32),RES(32));
    lblNameAdjust = DIS(32) ;
#endif

    // Location Desction Label
    lblDescription = Label::createWithTTF( uihelper::font_config_big, "" );
    lblDescription->getFontAtlas()->setAntiAliasTexParameters();
    lblDescription->setTextColor(Color4B(DESCRIPTION_COLOUR));
    lblDescription->setLocalZOrder(ZORDER_DEFAULT);
    lblDescription->setWidth(RES(800-64));
    uihelper::AddTopLeft(safeArea,lblDescription, RES(32),RES(32)+lblNameAdjust);

    // Shield
    imgShield = ImageView::create();
    imgShield->setLocalZOrder(ZORDER_DEFAULT);
    imgShield->setScale(SHIELD_SCALE);
    imgShield->setTouchEnabled(true);
    imgShield->addClickEventListener(clickCallback);
    imgShield->setTag(ID_THINK);
    uihelper::AddTopRight(safeArea, imgShield,SHIELD_X,SHIELD_Y);
    
    // people in front
    
    //auto background = LayerColor::create(Color4B(_clrRed));
    //background->setLocalZOrder(ZORDER_DEFAULT);
    //background->setContentSize( Size(getContentSize().width,RES(PEOPLE_WINDOW_HEIGHT)));
    //uihelper::AddBottomLeft(this, background);
    //uihelper::FillParent(background);
    
    for ( int ii=0; ii<3; ii++ ) {
        people[ii] = LandscapePeople::create(&options);
        addChild(people[ii]);
    }
    
    //people[1]->setVisible(false);
    //people[2]->setVisible(false);
    
    //
    
    current_info = new locationinfo_t();
    follower_info = new locationinfo_t();
        
    // Command Window
    i_command_window = uicommandwindow::create(this);
    i_command_window->retain();
    
    auto choose = uihelper::CreateImageButton("i_actions", ID_ACTIONS, clickCallback);
    uihelper::AddBottomRight(safeArea, choose, RES(10), RES(10) );

    // Character Selection
    auto select = uihelper::CreateImageButton("i_select2", ID_SELECT_ALL, clickCallback);
    uihelper::AddBottomLeft(safeArea, select, RES(10), RES(10) );
    
    
    // Keyboard
    initKeyboard();
    
    // TODO: Popup menu for character selection
    
    // TODO: Shortcut keys
    
    // TODO: Following
    
    // TODO: Debug Menu
    
    // TODO: Compass
    
    // TODO: Direction movement indicators
    setupMovementIndicators();
    
    // mouse events
    auto listener = EventListenerTouchOneByOne::create();
    
    // trigger when you push down
    listener->onTouchBegan = [=](Touch* touch, Event* event){
        bool result=OnMouseEvent(touch,event,true);
        updateMovementIndicators();
        return result;
    };
    
    // trigger when moving touch
    listener->onTouchMoved = [](Touch* touch, Event* event){
    };
    
    // trigger when you let up
    listener->onTouchEnded = [=](Touch* touch, Event* event){
        OnMouseEvent(touch,event,false);
        updateMovementIndicators();
    };
    
    // Add listener
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    
    return true;
}

void panel_look::OnMovementComplete( /*uiview* sender,*/ LANDSCAPE_MOVEMENT type )
{
#if defined(_DDR_)
    if ( type == LM_MOVE_FORWARD_LEAVE_TUNNEL ) {
        OnExitTunnel();
        return;
    }
#endif
    
    draw_arrows=false;
    updateMovementIndicators();
    
    startInactivity();
    
    
    if ( type == LM_DRAG_START ) {
        landscape_dragging=true;
        stopInactivity();
        return;
    }
    
    landscape_dragging=FALSE;
    
    if ( type == LM_SHOW_COMPASS ) {
        stopInactivity();
        
        // TODO:
        //character&    c = TME_CurrentCharacter();
        //i_compass->Show(landscape->mouse_down_pos, c.looking );
        
        return;
    }
    
    Enable();
    
    setObject(characterId);
    
    if ( type ==  LM_ROTATE_LEFT || type == LM_ROTATE_RIGHT || type == LM_NONE ) {
        mr->help->Shown(HELP_LOOKING_AROUND);
    }
    
    if ( type ==  LM_MOVE_FORWARD ) {
        
        mr->help->Shown(HELP_MOVEMENT);
        mr->help->Shown(HELP_MOVING);
        
        Disable();
        
        //
        // did we fight
        // or seek ?
        character&    c = TME_CurrentCharacter();
        if ( c.lastcommand == CMD_FIGHT ) {
            // think!
            mr->showPage(MODE_THINK_FIGHT, c.lastcommandid);
            
        }else if ( c.lastcommand == CMD_SEEK ) {
            mr->showPage(MODE_THINK_SEEK, c.lastcommandid);

        }
        
        this->scheduleOnce( [&](float) {
            delayedSave();
        }, 0.1f, "DelayedSave" );
        
        
    }
    
    // get the current location
    TME_GetCharacterLocationInfo(TME_CurrentCharacter());
    
    if ( location_infront_armies.foes_armies ) {
        if ( !ShowHelpWindow(HELP_BATTLE))
            return;
        if ( !ShowHelpWindow(HELP_ARMIES) )
            return;
    }
    
    int id = GET_ID(location_infront_object) ;
    if ( id >= OB_WOLVES && id<=OB_WILDHORSES ) {
        if ( !ShowHelpWindow(HELP_NASTIES) )
            return;
        return;
    }
    
#if defined(_DDR_)
    if ( Character_IsInTunnel(TME_CurrentCharacter()) ) {
        return;
    }
#endif
    
    
    defaultexport::location_t loc;
    TME_GetMapLocation(loc, location_infrontid );
    
#if defined(_DDR_)
    if ( loc.flags.Is(lf_mist) ) {
        if ( !ShowHelpWindow( HELP_TN_MISTS ) )
            return;
    }
#endif
    if ( (loc.terrain == TN_PLAINS ) || (loc.terrain== TN_PLAINS2) ) {
        TME_GetMapLocation(loc, location_lookingatid );
        if ( (loc.terrain != TN_PLAINS) && (loc.terrain!=TN_PLAINS2) )
            return;
    }
    
#if defined(_DDR_)
    // map plains for ddr plains
    if ( loc.terrain == TN_PLAINS )
        loc.terrain = TN_PLAINS2;
    loc.terrain = (mxterrain_t)((int)loc.terrain - TN_PLAINS2) ;
#endif
    
    if ( !ShowHelpWindow( (helpid_t)(HELP_NONE+1+(int)loc.terrain )) )
        return;
    
    
}

void panel_look::delayedSave()
{
    mr->stories->save();
    Enable();
    setObject(characterId);
    
    if ( (mr->panels->currentmode == this->currentmode)
        && recruitable_characters.Count() ) {
        mr->showPage ( MODE_THINK_APPROACH );
    }
}

//void panel_look::OnActionComplete( uiaction* sender, s32 value )
//{
//  UNDO
//  Enter Tunnel
//  Exit Tunnel
//  Menu Collapse
//  Delayed Save
//}

//void panel_look::OnCompassEvent(uicompass *sender, uicompassevent *event)
//{
//}

void panel_look::getCharacterInfo ( defaultexport::character_t& c, locationinfo_t* info)
{
    info->id = c.id;
    info->time = c.time;
    info->location = c.location;
    info->looking = c.looking;
    
    info->shield = GetCharacterShield(c) ;
    info->person = GetCharacterImage(c);
    info->face = GetCharacterFace(c);
    
#if defined(_DDR_)
    info->tunnel = Character_IsInTunnel(c);
    
    maplocation m;
    TME_GetLocationInDirection(m, c.location, c.looking);

    info->lookingdowntunnel = m.flags.Is(lf_tunnel) && info->tunnel;
    
#endif

    info->name = c.longname ;
    std::transform(info->name.begin(), info->name.end(), info->name.begin(), ::toupper);
    
    //lib::c_strupr ( info->name.c_str() );
    info->locationtext = TME_GetCharacterText ( c, "CharLocation" );
    
}

void panel_look::setObject ( mxid c )
{
    if ( ID_TYPE(c) != IDT_CHARACTER )
        return;
    
    characterId = c ;
    TME_CurrentCharacter(c);
    
    //if ( parent ) {
        getCurrentLocationInfo( );
        setViewForCurrentCharacter();
    //}
    
    //if ( current_view )
    //    current_view->SetCharacter(c);
    
}


void panel_look::getCurrentLocationInfo ( void )
{
    character&    c = TME_CurrentCharacter();
    getCharacterInfo(c, current_info);
    TME_GetCharacterLocationInfo ( c );
    
    
    if ( c.following ) {
        character tempc;
        TME_GetCharacter(tempc, c.following );
        getCharacterInfo(tempc, follower_info);
    }else{
        follower_info->id=0;
    }
    
    OnSetupIcons();
    OnSetupFaces();
    
}



void panel_look::setViewForCurrentCharacter ( void )
{
    character&    c = TME_CurrentCharacter();
    
#if defined (_LOM_)
    lblName->setString(current_info->name);
#endif

    std::string shieldImage = current_info->shield;
    if ( shieldImage.empty() )
        shieldImage = current_info->person ;
    
    lblDescription->setString(current_info->locationtext);
    imgShield->loadTexture(shieldImage, Widget::TextureResType::LOCAL);
    imgShield->setAnchorPoint(uihelper::AnchorTopRight);
    imgShield->setIgnoreAnchorPointForPosition(false);
    
    
    // TODO: Shield
    //((uiimage*)(imgShield->children[0]))->Image(current_info->shield);
    //imgShield->Enable();
    
    // TODO: Following
//    if ( follower_info->id ) {
//        uiimage* img = (uiimage*)i_Following->children[0];
//        img->Image(follower_info->face);
//        i_Following->dimensions = img->dimensions ;
//        i_Following->tag = follower_info->id;
//        i_Following->ShowEnable();
//        //ShowHelpWindow(HELP_GROUPED);
//    }else{
//        i_Following->HideDisable();
//    }

    options.colour->SetLookColour(current_info->time);
    
    options.here = current_info->location;
    options.here.x *= DIR_STEPS;
    options.here.y *= DIR_STEPS;
    options.isMoving = false;
    options.isLooking = false;

#if defined(_DDR_)
    options.isInTunnel = current_info->tunnel;
    options.isLookingDownTunnel = current_info->lookingdowntunnel;
#endif

    options.lookAmount = current_info->looking * 400;
    if ( options.lookAmount >= 3200 )
        options.lookAmount-=3200;
    
    options.currentLocation = current_info->location;
    options.currentDirection = current_info->looking;
    
    options.aheadLocation.x = options.currentLocation.x + mxgridref::DirectionLookTable[options.currentDirection*2];
    options.aheadLocation.y = options.currentLocation.y + mxgridref::DirectionLookTable[(options.currentDirection*2)+1];
    
    options.generator->Build(&options);
    
    // Initialise people
    current_people=people[0];
    next_people=people[1];
    prev_people=people[2];
    
    auto width = getContentSize().width;
    
    if ( current_people ) {
        current_people->Initialise(c);
        current_people->setPositionX(0);
    }
    
    if ( next_people ) {
        next_people->Initialise(NEXT_DIRECTION(current_info->looking));
        next_people->setPositionX(width);
    }
    
    if ( prev_people ) {
        prev_people->Initialise(PREV_DIRECTION(current_info->looking));
        prev_people->setPositionX(-width);
    }
    
    UpdateLandscape();
    
    // Add shield if following
    
    // update actions
    
}

void panel_look::UpdateLandscape()
{
    if ( current_view ) {
        removeChild(current_view);
    }
    
    options.generator->horizontalOffset = options.lookAmount;
    
    if ( current_info->tunnel ) {
        // TODO: Tunnels
        //tunnelView = new TunnelView();
        //current_view = tunnelView;
    }else{
        current_view = LandscapeView::create(&options);
    }

    current_view->setAnchorPoint(Vec2::ZERO);
    current_view->setPosition( Vec2::ZERO);
    current_view->setLocalZOrder(ZORDER_FAR);
    addChild(current_view);
    
    // LoM's header is affectively part of the backgroun
    // so update the colour to the same as the sky
#if defined(_LOM_)
    layHeader->setColor(Color3B(options.colour->CalcCurrentMovementTint(1)));
#endif
    
}

void panel_look::initKeyboard()
{
    auto eventListener = EventListenerKeyboard::create();
    
    eventListener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode, Event* event){
        
        if ( options.isMoving || options.isLooking )
            return;
        
        Vec2 loc = event->getCurrentTarget()->getPosition();
        switch(keyCode){
                
            case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
                startLookLeft();
                break;
            case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
                startLookRight();
                break;
            case EventKeyboard::KeyCode::KEY_UP_ARROW:
                moveForward();
                break;
            case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
                break;
                
            default:
                break;
        }
    };
    
    this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener,this);
}

bool panel_look::startLookLeft ( void )
{
    character&    c = TME_CurrentCharacter();
    Character_LookLeft ( c );
    
    options.colour->SetLookColour(c.time);
    
    f32 originalLooking = options.lookAmount;
    f32 target = -400 ;
    
    options.isLooking = true;
    
    auto actionfloat = ActionFloat::create(0.5, 0, target, [=](float value) {
        options.lookAmount = originalLooking + value;
        
        //UIDEBUG("MovementLeft  %f %f %f", target, options.lookAmount, value);
        
        f32 distance = value / target;
        if ( current_people )
            current_people->adjustMovement( distance );
        if (next_people)
            next_people->adjustMovement( distance );
        
        if ( value <= target) {
            stopRotating(LM_ROTATE_LEFT);
            return;
        }
        UpdateLandscape();
    });
    
    this->runAction(EaseSineInOut::create(actionfloat));
    
    next_people->Initialise(c);
    next_people->startSlideFromLeft();
    
    //current_people->Initialise(c);
    current_people->startSlideOffRight();

    
    
    return TRUE;
}

bool panel_look::startLookRight ( void )
{
    character&    c = TME_CurrentCharacter();
    Character_LookRight ( c );
    
    options.colour->SetLookColour(c.time);
    
    f32 originalLooking = options.lookAmount;
    f32 target = 400 ;
    
    options.isLooking = true;
    
    auto actionfloat = ActionFloat::create(0.5, 0, target, [=](float value) {
        options.lookAmount = originalLooking + value;
        
        //UIDEBUG("MovementRight  %f %f %f", target, options.lookAmount, value);
        
        f32 distance = value / target;
        if ( current_people )
            current_people->adjustMovement( distance );
        if (next_people)
            next_people->adjustMovement( distance );
        
        if ( value >= target) {
            stopRotating(LM_ROTATE_RIGHT);
            return;
        }
        
        UpdateLandscape();
    });
    
    this->runAction(EaseSineInOut::create(actionfloat));
    
    next_people->Initialise(c);
    next_people->startSlideFromRight();
    
    //current_people->Initialise(c);
    current_people->startSlideOffLeft();
    
    return TRUE;
    
}


bool panel_look::moveForward ( void )
{
    character& c = TME_CurrentCharacter();
    
    TME_GetCharacterLocationInfo ( c );
    
    // something is in our way that we must fight
    // so check for auto fight
    if ( location_flags&lif_fight ) {
        if ( mr->config->autofight ) {
            // do we have an army?
            // do we have the correct sword?
            
            mxid fight = Character_Fight(c);
            
            if ( fight != IDT_NONE && Character_IsDead(c)) {
                mr->showPage(MODE_THINK_FIGHT,fight);
                mr->stories->save();
                return FALSE;
            }
            
            //return FALSE;
        }
    }
    
    //
    if ( Character_IsHidden(c) ) {
        if ( mr->config->autounhide ) {
            if ( Character_Hide(c) ) {
            }
        }
    }
    
    if ( !startMoving() ) {
        
        character& c = TME_CurrentCharacter();
        
        TME_GetCharacterLocationInfo(c);
        
        if ( location_stubborn_lord_move!=IDT_NONE) {
            // switch to the stubborn character
            // and think
            setObject(location_stubborn_lord_move);
            return true;
        }
        
        if ( Character_IsNight(c) ) {
            if ( !ShowHelpWindow(HELP_PRESS_NIGHT))
                return TRUE;
        }
        
        if ( c.following != IDT_NONE )
            if (!ShowHelpWindow(HELP_GROUPED))
                return TRUE;
        
        if ( location_infront_armies.foes_armies )
            if (!ShowHelpWindow(HELP_BATTLE, TRUE))
                return TRUE;
        
        // something is in our way that we must fight
        mxid objectid = location_flags&lif_fight ? location_fightthing : Character_LocationObject(c);
        
        mr->showPage(MODE_THINK, objectid);
        
        return TRUE;
    }
    
    return FALSE;
}



bool panel_look::startMoving()
{
    character&    c = TME_CurrentCharacter();
    
    mxtime_t startTime = c.time;
    
    
    mxid            key;
    maplocation        map;
    key = MAKE_LOCID(location_infront.x, location_infront.y);
    TME_GetLocation( map, key );
    
    options.moveLocationHasArmy = map.flags&lf_army ;
    options.moveFrom = c.location;
    
    if ( Character_Move(c) ) {
        
        options.colour->SetMovementColour(startTime,c.time);
        
        options.moveTo = c.location;
        f32 target = DIR_STEPS ;
        
        options.isMoving = true;
   
        auto actionfloat = ActionFloat::create(0.5f, 0, target, [=](float value) {
            
            options.here.x = options.moveFrom.x*(DIR_STEPS - value) + options.moveTo.x*value;
            options.here.y = options.moveFrom.y*(DIR_STEPS - value) + options.moveTo.y*value;
            
            if ( value >= target ) {
                stopMoving();
                return;
            }
        
            f32 result = value / target ;
            options.movementAmount = result;
            
            options.generator->Build(&options);
            UpdateLandscape();
            
            
        });
        
        this->runAction(EaseSineInOut::create(actionfloat));
        
        next_people->Initialise(c);
        next_people->startFadeIn();
        
        //current_people->Initialise(c);
        current_people->startFadeOut();
        
        return true;
    }
    
    return false;
    
}

void panel_look::stopRotating(LANDSCAPE_MOVEMENT type)
{
    options.isLooking = false;
    
    LandscapePeople* temp = next_people;
    next_people=current_people;
    current_people=temp;
    
    next_people->clear();
    current_people->stopAnim();
    prev_people->clear();
    
    setViewForCurrentCharacter();
    
    OnMovementComplete(type);
}

void panel_look::stopMoving()
{
    options.isMoving = false;
    
    // swap info
    LandscapePeople* temp = next_people;
    next_people=current_people;
    current_people=temp;
    
    next_people->clear();
    current_people->stopAnim();
    prev_people->clear();
    
    setViewForCurrentCharacter();
    
    OnMovementComplete(LM_MOVE_FORWARD);
    
}

void panel_look::startInactivity()
{
    this->scheduleOnce( [&](float) {
        showInactivityHelp();
    }, NONE_ACTIVTY_DURATION, "NonActivityTimer" );
}

void panel_look::stopInactivity()
{
    this->unschedule("NonActivityTimer");
}

void panel_look::showInactivityHelp()
{
    static helpid_t help_options[] = { HELP_LOOKING_AROUND, HELP_MOVEMENT, HELP_SELECTING_CHARACTER, HELP_THINKING, HELP_CHOOSE };
    
    if ( isEnabled() ) {
        if ( !isHelpVisible() ) {
            for ( u32 ii=0; ii<NUMELE(help_options); ii++ ) {
                if ( !ShowHelpWindow( help_options[ii] ) )
                    break;
            }
        }
    }
}

void panel_look::hideMenus ( void )
{
    // TODO: Hide Menus
//    if ( actions )
//        actions->Collapse();
//    if ( faces )
//        faces->Collapse();
//    if ( debug )
//        debug->Collapse();
}


void panel_look::fadeIn ( rgb_t colour, f32 initialAlpha,  MXVoidCallback callback )
{
    if ( !mr->config->screentransitions ) {
        if ( callback != nullptr ) {
            callback();
        }
        return;
    }
    
    auto rect = this->getBoundingBox();
    auto fade_panel = DrawNode::create();
    fade_panel->drawSolidRect(Vec2(0,0), Vec2( rect.getMaxX(), rect.getMaxY()), Color4F(colour) );
    fade_panel->setOpacity(ALPHA(initialAlpha));
    fade_panel->setLocalZOrder(ZORDER_POPUP);
    addChild(fade_panel);
    
    auto callbackAction = CallFunc::create([=]() {
        if ( callback != nullptr ) {
            callback();
        }
        this->removeChild(fade_panel);
        //fade_panel = nullptr;
    });
    
    auto action = FadeOut::create(3.0f);
    auto sequence = Sequence::createWithTwoActions(action, callbackAction);
    fade_panel->runAction(sequence);
    
}

void panel_look::fadeOut ( rgb_t colour, f32 initialAlpha,  MXVoidCallback callback )
{
    if ( !mr->config->screentransitions ) {
        if ( callback != nullptr ) {
            callback();
        }
        return;
    }
    
    auto rect = this->getBoundingBox();
    auto fade_panel = DrawNode::create();
    fade_panel->drawSolidRect(Vec2(0,0), Vec2( rect.getMaxX(), rect.getMaxY()), Color4F(colour) );
    fade_panel->setLocalZOrder(ZORDER_POPUP);
    fade_panel->setOpacity(ALPHA(initialAlpha));
    addChild(fade_panel);
    
    auto callbackAction = CallFunc::create([=]() {
        if ( callback != nullptr ) {
            callback();
        }
        this->removeChild(fade_panel);
        //fade_panel = nullptr;
    });
    
    auto action = FadeIn::create(3.0f);
    auto sequence = Sequence::createWithTwoActions(action, callbackAction);
    fade_panel->runAction(sequence);
}

void panel_look::OnShown()
{
    startInactivity();
    
    
    if ( variables::sv_days == 0) {
        if ( !ShowHelpWindow(HELP_DAY1))
            return;
        
        if ( !ShowHelpWindow(HELP_CHOICE_OF_GAMES))
            return;
    }
    
    if ( variables::sv_days == 1) {
        if ( !ShowHelpWindow(HELP_BATTLE))
            return;
#if defined(_LOM_)
        if ( !ShowHelpWindow(HELP_ICEFEAR))
            return;
#endif
    }
    if ( variables::sv_days == 2) {
        if ( !ShowHelpWindow(HELP_VICTORY))
            return;
    }
    
#if defined(_LOM_)
    character& c = TME_CurrentCharacter();
    if ( c.race == RA_FREE || c.race == RA_MORKIN )
        if ( !ShowHelpWindow(HELP_FREE))
            return;
    if ( c.race == RA_FEY )
        if ( !ShowHelpWindow(HELP_FEY))
            return;
    if ( c.race == RA_WISE )
        if ( !ShowHelpWindow(HELP_WISE))
            return;
#endif
}

void panel_look::OnActivate( void )
{
    uipanel::OnActivate();
    
    // make sure we are working with our character
    setObject(characterId);
    
    character&    c = TME_CurrentCharacter();
    
    stopInactivity();
    
    if ( Character_IsDead(c) ) {
        mr->showPage( MODE_THINK );
        return;
    }
    
    OnSetupIcons();
    OnSetupFaces();
}

void panel_look::OnDeActivate( void )
{
    uipanel::OnDeActivate();
}

void panel_look::OnSetupFaces()
{
}

void panel_look::OnNotification( Ref* sender )
{
    stopInactivity();
    
    auto button = static_cast<Widget*>(sender);
    if ( button == nullptr )
        return;
    
    layoutid_t id = static_cast<layoutid_t>(button->getTag());
    
    hideMenus();
    
    switch ( id ) {
            
        case ID_ACTIONS:
            i_command_window->show(nullptr);
            break;
            
        case ID_HOME:
        {
            AreYouSure(CLOSE_STORY_MSG, [&] {
                mr->closeStory();
            });
            
            break;
        }
            
        case ID_UNDO_NIGHT:
        {
            AreYouSure(UNDO_NIGHT_MSG, [&] {
                OnUndo(savemode_night);
            });

            return;
            break;
        }
            
        case ID_UNDO_DAWN:
        {
            AreYouSure(UNDO_DAWN_MSG, [&] {
                OnUndo(savemode_dawn);
            });
            
            return;
            break;
        }
            
        case ID_UNDO:
        {
            OnUndo(savemode_normal);
            return;
        }

        case ID_NIGHT:
        {
            AreYouSure(NIGHT_MSG, [&] {
                mr->night();
            });
            
            return;
        }
            
        case ID_MAP:
        {
            mr->showPage(MODE_MAP_OVERVIEW);
            break;
        }
            
        case ID_SELECT_ALL:
        {
            mr->help->Shown(HELP_SELECTING_CHARACTER);
            mr->showPage( MODE_SELECT );
            break;
        }
            
        case ID_APPROACH:
        {
            if ( mr->approach() )
                return;
        }
            
#if defined(_LOM_)
        case ID_SEEK:
        {
            if ( mr->seek() )
                return;
        }
            
        case ID_HIDE:
        case ID_UNHIDE:
        {
            if ( mr->hideunhide()) {
                return;
            }
            break;
        }
            
        case ID_FIGHT:
        {
            if ( mr->fight() ) {
                return;
            }
            break;
        }
#endif
            
#if defined(_DDR_)
        case ID_GIVE:
        {
            if ( mr->give() )
                return;
            break;
        }
        case ID_TAKE:
        {
            if ( mr->take() )
                return;
            break;
        }
        case ID_USE:
        {
            if ( mr->use() )
                return;
            break;
        }
        case ID_REST:
        {
            if ( mr->rest() )
                return;
            break;
        }
        case ID_ENTER_TUNNEL:
        {
            if ( OnEnterTunnel() )
                return;
        }
#endif
            
        case ID_THINK:
        {
            if ( mr->think() )
                return;
        }
            
        case ID_RECRUITMEN:
        {
            if ( mr->recruitMen() )
                return;
            break;
        }
        case ID_POSTMEN:
        {
            if ( mr->postMen() )
                return;
            break;
        }
            
        case ID_ATTACK:
        {
            if ( mr->attack() )
                return;
            break;
        }
            
            // check for character selection
        case ID_CHAR1:
        case ID_CHAR2:
        case ID_CHAR3:
        case ID_CHAR4:
        case ID_SHOW_LEADER:
        {
            char_data_t* data = static_cast<char_data_t*>(button->getUserData());
            
            if ( mr->selectCharacter(data->id) )
                return;
 
            break;
        }
            
        default:
            break;
    }
    
    hideMenus();
    
    TME_RefreshCurrentCharacter();
    TME_GetCharacterLocationInfo ( TME_CurrentCharacter() );
    uipanel::OnNotification(sender);
}

bool panel_look::OnUndo ( savemode_t mode )
{
    undo_mode=mode;
    fadeOut(_clrBlack, 0.75f, [&](){
        mr->undo(undo_mode);
        setObject( TME_CurrentCharacter().id );
        fadeIn(_clrBlack, 1.0f, [&]{
                // do nothing
        });
    });
    return true;
}


#if defined(_DDR_)

bool panel_look::OnEnterTunnel()
{
    hideMenus();
    fadeOut(_clrBlack, 0.75f, [&](){
        
        mr->enterTunnel();
        
        getCurrentLocationInfo();
        setViewForCurrentCharacter();
       // current_view->SetViewForCurrentCharacter();
        fadeIn(_clrBlack, 1.0f, [&](){
            ShowHelpWindow(HELP_TN_TUNNEL);
        });
    });
    
    
    return true;
}

bool panel_look::OnExitTunnel()
{
    hideMenus();
    fadeOut(_clrWhite, 0.0f, [&]() {
        OnMovementComplete(LM_MOVE_FORWARD);
        fadeIn(_clrBlack, 1.0f, [&](){
        });
    });
    return true;
}

#endif // _DDR_


void panel_look::OnSetupIcons ( void )
{

// TODO: Debug menu
//    if ( debug ) {
//        debug->ClearItems();
//
//        if ( gl->stories->canUndo(savemode_night) )
//            debug->AddItem(i_debug_undo_night);
//
//        if ( gl->stories->canUndo(savemode_dawn) )
//            debug->AddItem(i_debug_undo_dawn);
//
//        if ( gl->stories->canUndo(savemode_normal) )
//            debug->AddItem(i_debug_undo);
//
//        debug->AddItem(i_debug_email);
//        debug->AddItem(i_debug_map);
//    }
    
    
    if ( i_command_window==NULL )
        return;
    
    i_command_window->updateElements();
    
}

bool panel_look::OnMouseEvent( Touch* touch, Event* event, bool pressed )
{
    f32 MOUSE_MOVE_BLEED = 256;
    f32 MOUSE_LOOK_BLEED = 256;
    
    auto size = getContentSize();
    
    bool IsLeftMouseDown = false;
    bool IsLeftMouseUp = false;
    
    if ( event->getType() == Event::Type::MOUSE || event->getType() == Event::Type::TOUCH ) {
        if ( pressed )
            IsLeftMouseDown = true;
        else
            IsLeftMouseUp = true;
    }
    
    auto position =  touch->getLocation();
    
    if ( mr->config->nav_mode!=CF_NAV_SWIPE ) {
        
        int move_press_y = size.height - RES(MOUSE_MOVE_BLEED) ;
        
        // use full centre height if we are only pressing
        if ( mr->config->nav_mode==CF_NAV_SWIPE_MOVE_PRESS_LOOK)
            move_press_y = size.height * 0.75 ;
        
        else if ( mr->config->nav_mode==CF_NAV_PRESS )
            move_press_y = size.height ;
        
        if ( IsLeftMouseDown  ) {
            
            
            if ( mr->config->nav_mode!=CF_NAV_SWIPE_MOVE_PRESS_LOOK) {
                if ( position.y > move_press_y && (position.x>RES(MOUSE_LOOK_BLEED) /*&& event->m_mouse.x < imgShield->location.x*/) ) {
                    draw_arrows=true;
                    current_arrow=LM_MOVE_FORWARD;
                    return true;
                }
            }
            
            if ( position.x > size.width-RES(MOUSE_LOOK_BLEED) ) {
                draw_arrows=true;
                current_arrow=LM_ROTATE_RIGHT;
                return true;
            } else if ( position.x < RES(MOUSE_LOOK_BLEED) ) {
                draw_arrows=true;
                current_arrow=LM_ROTATE_LEFT;
                return true;
            }
            
        }
        
        if ( IsLeftMouseUp  ) {
            
            if ( !draw_arrows )
                return true;
            
            if ( mr->config->nav_mode!=CF_NAV_SWIPE_MOVE_PRESS_LOOK) {
                if ( position.y > move_press_y && (position.x>RES(MOUSE_LOOK_BLEED) /*&&  event->m_mouse.x < imgShield->location.x*/ ) ) {
                    draw_arrows=false;
                    moveForward();
                    return true;
                }
            }
            
            if ( position.x > size.width-RES(MOUSE_LOOK_BLEED) ) {
                draw_arrows=false;
                startLookRight();
                return true;
            } else if ( position.x < RES(MOUSE_LOOK_BLEED) ) {
                draw_arrows=false;
                startLookLeft();
                return true;
            }
        }
    }
    
    return false;
}

void panel_look::setupMovementIndicators()
{
    if ( !mr->config->showmovementindicators )
        return;
    
    f32 scale = 2.5f;
    
    movementIndicators[0] = Sprite::createWithSpriteFrameName("arrow_left");
    movementIndicators[0]->setScale(scale);
    movementIndicators[0]->setLocalZOrder(ZORDER_DEFAULT+1);
    uihelper::AddCenterLeft(this, movementIndicators[0], RES(48),RES(0));

    
    movementIndicators[1] = Sprite::createWithSpriteFrameName("arrow_right");
    movementIndicators[1]->setScale(scale);
    movementIndicators[1]->setLocalZOrder(ZORDER_DEFAULT+1);
    uihelper::AddCenterRight(this, movementIndicators[1], RES(48),RES(0));

    movementIndicators[2] = Sprite::createWithSpriteFrameName("arrow_up");
    movementIndicators[2]->setScale(scale);
    movementIndicators[2]->setLocalZOrder(ZORDER_DEFAULT+1);
    uihelper::AddTopCenter(this, movementIndicators[2],RES(0),RES(256));

    draw_arrows = false;
    updateMovementIndicators();
}

void panel_look::updateMovementIndicators()
{
    for ( int ii=0; ii<NUMELE(movementIndicators); ii++ ) {
        movementIndicators[ii]->setVisible(draw_arrows);
        movementIndicators[ii]->setColor(_clrWhite);
        
        if ( draw_arrows ) {
            if ( current_arrow == (ii+1) ) {
                movementIndicators[ii]->setOpacity(ALPHA(1.0f));
            }else{
                movementIndicators[ii]->setOpacity(ALPHA(0.25f));
            }
        }
    }
}
