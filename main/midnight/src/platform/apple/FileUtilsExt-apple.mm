#import <Foundation/Foundation.h>

#include <ftw.h>

#include <string>
#include <stack>

#include "FileUtilsExt-apple.h"

#define DECLARE_GUARD std::lock_guard<std::recursive_mutex> mutexGuard(_mutex)


std::string FileUtilsExtApple::getApplicationSupportPath()
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    std::string strRet = [documentsDirectory UTF8String];
    strRet.append("/");
    return strRet;
}

