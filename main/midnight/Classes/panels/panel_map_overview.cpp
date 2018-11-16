
/*
 *  panel_map_overview.cpp
 *  midnight
 *
 *  Created by Chris Wild on 10/10/2018.
 *  Copyright (c) 2018 Chilli Hugger Software. All rights reserved.
 *
 */

#include "panel_map_overview.h"

#include "../system/moonring.h"
//#include "../system/configmanager.h"
#include "../system/resolutionmanager.h"
//#include "../system/panelmanager.h"
#include "../ui/uihelper.h"

#include "../frontend/layout_id.h"

USING_NS_CC;
using namespace tme;
using namespace cocos2d::ui;

bool panel_map_overview::init()
{
    if ( !uipanel::init() )
    {
        return false;
    }
    
    auto backgroundColour = Color3B(245,238,228);
    
    setBackground(backgroundColour);
    
    f32 layout_padding = RES(20);
    
    auto scrollView = ui::ScrollView::create();
    scrollView->setDirection(ui::ScrollView::Direction::VERTICAL);
    scrollView->setBounceEnabled(true);
    scrollView->setScrollBarEnabled(true);
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto image = Sprite::create("screens/misc/overview_map.png");
    f32 scale = visibleSize.width / image->getContentSize().width ;
    image->setScale(scale, scale );

    f32 width = visibleSize.width;
    
    scrollView->setInnerContainerSize( Size(width,image->getContentSize().height+(2*layout_padding)) );
    scrollView->setContentSize(visibleSize);
    
    image->setAnchorPoint(uihelper::AnchorBottomLeft);
    image->setPosition(0,layout_padding);
    
    scrollView->addChild(image);
    addChild(scrollView);
    
    uihelper::FillParent(scrollView);
    
    // bottom gradient
    auto gradientB = LayerGradient::create( Color4B(backgroundColour,ALPHA(0.0f)), Color4B(backgroundColour,ALPHA(1.0f)) );
    gradientB->setContentSize(Size(width, layout_padding));
    uihelper::AddBottomLeft(this, gradientB);
    
    // top gradient
    auto gradientT = LayerGradient::create( Color4B(backgroundColour,ALPHA(1.0f)), Color4B(backgroundColour,ALPHA(0.0f)) );
    gradientT->setContentSize(Size(width, layout_padding));
    uihelper::AddTopLeft(this, gradientT);
    
    
    // Command Window
    // Look Icon
    auto look = uihelper::CreateImageButton("i_look", ID_LOOK, clickCallback);
    uihelper::AddBottomLeft(safeArea, look, RES(10), RES(10) );
  
    auto map = uihelper::CreateImageButton("i_small_map", ID_MAP_DISCOVERY, clickCallback);
    uihelper::AddBottomRight(safeArea, map, RES(10), RES(10) );
    
    return true;
}


void panel_map_overview::OnNotification( Ref* sender )
{
    auto button = static_cast<Button*>(sender);
    if ( button == nullptr )
        return;
    
    layoutid_t id = static_cast<layoutid_t>(button->getTag());
    
    switch ( id  )
    {
        case ID_LOOK:
            mr->look();
            break;
            
        case ID_MAP_DISCOVERY:
            mr->showPage(MODE_MAP_DETAILED);
            break;
        
        default:
            break;
    }
}