#ifndef __HOSTHELPERS_H__
#define __HOSTHELPERS_H__

#include <stdarg.h>
#include "game/ht.h"
#include "game/chatcontroller.h"

#ifdef __ANDROID__
#include <jni.h>
#endif

namespace wi {

struct SurfaceProperties {
    int cxWidth;
    int cyHeight;
    int cbxPitch;
    int cbyPitch;
    float density;
    unsigned short ffFormat;
};

const unsigned short kfDirect565 = 0x0001;
const unsigned short kfDirect888 = 0x0002;
const unsigned short kfDirect8 = 0x0004;

class FormMgr;

class HostHelpers
{
public:
    static void OpenUrl(const char *pszUrl);
    static void Log(const char *pszFormat, ...);
    static void Log(const char *pszFormat, va_list va);
    static void MessageBox(const char *pszFormat, va_list va);
    static void Break();
    static void GetSurfaceProperties(SurfaceProperties *pprops);
    static void FrameStart();
    static void FrameComplete(int cfrmm, UpdateMap **apupd, Rect *arc,
            bool fScrolled);
    static void ResetScrollOffset();
    static void SetFormMgrs(FormMgr *pfrmmSimUI, FormMgr *pfrmmInput);
    static DibBitmap *CreateFrontDib(int cx, int cy, int nDegreeOrientation);
    static const char *GetUdid();
    static void InitiateAsk(const char *title, int max, const char *def,
            int keyboard, bool secure);
    static void GetAskString(char *psz, int cb);
    static IChatController *GetChatController();
    static void InitiateWebView(const char *title, const char *url);
    static bool IsExiting();
    static void GameThreadStart(void *pv);
    static void DisplayInitComplete();
    static const char *GetPlatformString();
    static bool DirExists(char *psz);
    static bool EnumFiles(Enum *penm, int key, char *pszFn, int cbFn);

    // TODO(darrinm): unused?
    static int main(int argc, char **argv);

    static bool Init();
    static void Cleanup();
    static const char *GetMainDataDir();
    static const char *GetMissionPacksDir();
    static const char *GetMissionPackInfosDir();
    static const char *GetTempDir();
    static const char *GetCompletesDir();
    static const char *GetSaveGamesDir();
    static const char *GetPrefsFilename();

    #ifdef __ANDROID__
    static JavaVM *GetJavaVM();
    static JNIEnv *GetJNIEnv();
    #endif
};

} // namespace wi

#endif // __HOSTHELPERS_H__
