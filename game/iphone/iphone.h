#ifndef __IPHONE_H__
#define __IPHONE_H__

#include <stdarg.h>
#include "game/ht.h"
#include "game/chatcontroller.h"

namespace wi {

struct SurfaceProperties {
    int cxWidth;
    int cyHeight;
    int cbxPitch;
    int cbyPitch;
    unsigned short ffFormat;
};

const unsigned short kfDirect565 = 0x0001;
const unsigned short kfDirect888 = 0x0002;

class SpriteManager;
class FormMgr;

class IPhone
{
public:
    static void OpenUrl(const char *pszUrl);
    static void Log(char *pszFormat, ...);
    static void Log(char *pszFormat, va_list va);
    static void MessageBox(char *pszFormat, va_list va);
    static void Break();
    static void GetSurfaceProperties(SurfaceProperties *pprops);
    static void FrameStart();
    static void FrameComplete(int cfrmm, UpdateMap **apupd, Rect *arc,
            bool fScrolled);
    static void ResetScrollOffset();
    static SpriteManager *GetSpriteManager();
    static void SetFormMgrs(FormMgr *pfrmmSimUI, FormMgr *pfrmmInput);
    static DibBitmap *CreateFrontDib(int cx, int cy, int nDegreeOrientation);
    static void SetPalette(Palette *ppal);
    static const char *GetMissionPacksDir();
    static const char *GetMissionPackInfosDir();
    static const char *GetSaveGamesDir();
    static const char *GetPrefsFilename();
    static const char *GetMainDataDir();
    static const char *GetTempDir();
    static const char *GetCompletesDir();
    static const char *GetStaticUUID();
    static void InitiateAsk(const char *title, int max, const char *def,
            int keyboard, bool secure);
    static void GetAskString(char *psz, int cb);
    static IChatController *GetChatController();
    static void InitiateWebView(const char *title, const char *url);
    static bool IsExiting();
    static void GameThreadStart(void *pv);
    static int main(int argc, char **argv);

private:
    static void PrepareDirs();
};

} // namespace wi

#endif // __IPHONE_H__
