//
//  shadermanager.h
//  midnight
//
//  Created by Chris Wild on 17/04/2020.
//

#pragma once

#include "../cocos.h"
#include "../library/inc/mxtypes.h"
#include "ringcontroller.h"

FORWARD_REFERENCE(SimpleShader);

class shadermanager : public ringcontroller
{
    using Node = cocos2d::Node;
    using Vec4 = cocos2d::Vec4;
    using ProgramState = cocos2d::backend::ProgramState;
    using Program = cocos2d::backend::Program;
    using Color4F = cocos2d::Color4F;

private:
    void CreateDayNightShader();
    
public:
    
    void Init();
    
    Node* AttachShader(Node* node, SimpleShader* state);

    //
    // Day and Night shader
    //
    void UpdateSpriteDayNightShader(Node* node, f32 alpha, Color4F& outline, Color4F& body);
    Node* AddDayNightShader(Node* node);
    SimpleShader* GetDayNightShader();

    //
    
protected:
    SimpleShader*         dayNightShader;
};

#include <stdio.h>
