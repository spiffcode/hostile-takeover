#ifndef __APPDELEGATE_H__
#define __APPDELEGATE_H__

#import <Foundation/Foundation.h>

#include "base/thread.h"

@interface AppDelegate : NSObject <NSApplicationDelegate> {
    bool m_fExiting;
}

@end

#endif // __APPDELEGATE_H__
