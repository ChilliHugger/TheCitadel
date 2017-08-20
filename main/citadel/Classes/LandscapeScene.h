#ifndef __LANDSCAPE_SCENE_H__
#define __LANDSCAPE_SCENE_H__

#include "cocos2d.h"

#include "tme_interface.h"

#include "LandscapeGenerator.h"

USING_NS_CC;

typedef enum GESTURE_MODE {
    none=0,
    horizontal=1,
    vertical=2,
} GESTURE_MODE ;



typedef enum SNAP_MODE {
    sm_back_right=1,
    sm_forward_right=3,
} SNAP_MODE ;

enum LANDSCAPE_MOVEMENT
{
    LM_NONE=0,
    LM_ROTATE_LEFT=1,
    LM_ROTATE_RIGHT=2,
    LM_MOVE_FORWARD=3,
    LM_MOVE_BACKWARD=4,
#if defined(_DDR_)
    LM_MOVE_FORWARD_LEAVE_TUNNEL=5,
#endif
    LM_DRAG_START=10,
    LM_SHOW_COMPASS=11,
    LM_HIDE_COMPASS=12,
};


class Landscape : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
    // a selector callback
    void menuCloseCallback(Ref* pSender);
    
    // implement the "static create()" method manually
    CREATE_FUNC(Landscape);

    
    Sprite* GetTerrainImage( mxterrain_t terrain );
    Sprite* GetFloorImage( floor_t floor );
    

    bool onTouchBegan(Touch* touch, Event* event);
    void onTouchEnded(Touch* touch, Event* event);
    void onTouchMoved(Touch* touch, Event* event);
    void onTouchCancelled(Touch* touch, Event* event);

    void update(float delta);
    
    void Moving();
    void Rotating();
    
    bool StartMoving();
    void StopMoving();
    
    bool StartLookRight ( void );
    bool StartLookLeft ( void );
    void StopRotating(LANDSCAPE_MOVEMENT type);

    void SetViewForCurrentCharacter ( void );
    
    void BuildFloors( LandscapeItems* items );
    void BuildTerrain( LandscapeItems* items );
    void BuildDebug( LandscapeItems* items );
    void UpdateScreenCoordinates();

    
    void UpdateLandscape();
    void InitKeyboard();
    
    
private:
    GLProgramState* glProgramState;
    
    
    f32				dragged;
    Vec2            drag_start;
    Vec2            drag_current;
    Vec2            drag_stop;
    
    
    /* landscaping */
    LandscapeGenerator*  landscapeGen;
    
    f32				rotate_look ;
    f32				voffset ;
    f32				looking ;
    f32				turning ;
    
    loc_t			here ;
    loc_t           currentLocation;
    loc_t           aheadLocation;
    mxdir_t         currentDirection;
    
    loc_t			moveFrom ;
    loc_t			moveTo ;
    f32             movement_amount ;
    s32         right ;
    s32         left ;
    GESTURE_MODE fingermode;
    
    s32				moveCount ;
    s32				UseHalfHeight ;
    s32				snapback;
    
    s32             location_infront_y;
    

    BOOL            bMoveLocationHasArmy;
    s32             compass_delay;
    
    GLProgram*      glShaderProgram;
    
public:
    u32             mouse_down_time;
    BOOL            mouse_down;
    point           mouse_down_pos;


    Node*           current_landscape_node;
    f32             masterScale;
 
    bool            showWater;
    bool            showLand;
    bool            showTerrain;
    int             debugMode;
    bool            debugLand;
    f32             landScaleX;
    f32             landScaleY;
    
    
};

#endif // __LANDSCAPE_SCENE_H__
