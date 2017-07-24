#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <sys/stat.h>
#include "game/sdl/hosthelpers.h"
#include "base/thread.h"
#include "game/sdl/sdlhttpservice.h"
#include "androidchatcontroller.h"

#define LOG_TAG "HT"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

static JavaVM *g_jvm;
static JNIEnv *g_env; // Only use this in the SDL/HT thread

// Java class and methods that we will call from C++
static jclass NativeLibClass;
static jmethodID getDataDirMethod;
static jmethodID openUrlMethod;
static jmethodID screenWidthMethod;
static jmethodID screenHeightMethod;
static jmethodID screenDPIMethod;
static jmethodID getAndroidIDMethod;
static jmethodID initiateAskMethod;
static jmethodID initiateWebViewMethod;
static jmethodID getAskStringMethod;
static jmethodID getPlatformStringMethod;
static jmethodID getAssetManagerMethod;

namespace wi {

//char gszMainDataDir[PATH_MAX];        // main data is archived into Android assets
char gszTempDir[PATH_MAX];              // temp file directory
char gszMissionPacksDir[PATH_MAX];      // downloaded mission packs
char gszMissionPackInfosDir[PATH_MAX];  // info about downloaded mission packs
char gszSaveGamesDir[PATH_MAX];         // saved games
char gszPrefsFilename[PATH_MAX];        // game prefs
char gszCompletesDir[PATH_MAX];         // for "mission completed" tracking
HttpService *gphttp;
AAssetManager *gassetmgr;

IChatController *g_pchat;

#ifdef __cplusplus
extern "C" {
#endif
// This gets called from the Java thread when JNI is loaded
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv *env;
	if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) >= 0) {
        // LOG() << "Success getting env";

        // Lets get the vm and save it globally for later
        int status = env->GetJavaVM(&g_jvm);
        if (status != 0) {
            LOG() << "Failed getting JavaVM";
        } else {
            // LOG() << "Success getting JavaVM";
        }
	} else {
        LOG() << "Error getting env";
    }

    // Make a tmp class then declair the real class as a global reference
    jclass tmp = env->FindClass("com/spiffcode/ht/NativeLib");
    NativeLibClass = (jclass)env->NewGlobalRef(tmp);

    // Setup the JNI method calls
    getDataDirMethod = env->GetStaticMethodID(NativeLibClass, "getDataDir", "()Ljava/lang/String;");
    screenWidthMethod = env->GetStaticMethodID(NativeLibClass, "screenWidth", "()I");
    screenHeightMethod = env->GetStaticMethodID(NativeLibClass, "screenHeight", "()I");
    screenDPIMethod = env->GetStaticMethodID(NativeLibClass, "screenDPI", "()I");
    getAndroidIDMethod = env->GetStaticMethodID(NativeLibClass, "getAndroidID", "()Ljava/lang/String;");
    openUrlMethod = env->GetStaticMethodID(NativeLibClass, "openUrl", "(Ljava/lang/String;)V");
    initiateWebViewMethod = env->GetStaticMethodID(NativeLibClass, "initiateWebView", "(Ljava/lang/String;)V");
    getAskStringMethod = env->GetStaticMethodID(NativeLibClass, "getAskString", "()Ljava/lang/String;");
    initiateAskMethod = env->GetStaticMethodID(NativeLibClass, "initiateAsk", "(Ljava/lang/String;ILjava/lang/String;II)V");
    getPlatformStringMethod = env->GetStaticMethodID(NativeLibClass, "getPlatformString", "()Ljava/lang/String;");
    getAssetManagerMethod = env->GetStaticMethodID(NativeLibClass, "getAssetManager", "()Landroid/content/res/AssetManager;");

    return JNI_VERSION_1_4;
}

// This gets called from the Java thread when the user is done with the string input dialog
JNIEXPORT void JNICALL
Java_com_spiffcode_ht_NativeLib_nativePostAskStringEvent(JNIEnv* env, jclass cls) {
    // Inform the game thread that it's time to get the string
    HostGetGameThread().Post(wi::kidmAskStringEvent, NULL);
}
#ifdef __cplusplus
} // extern C
#endif

bool HostHelpers::Init() {
    // Get the env for this thread using the global JavaVM
    int getEnvStat = g_jvm->GetEnv((void**)&g_env, JNI_VERSION_1_4);
    if (getEnvStat == JNI_EDETACHED) {
        // This shouldn't happen... SDL should have already atteched this thread
        LOG() << "Env not attached";
        LOG() << "Attempting to attach current thread";
        if (g_jvm->AttachCurrentThread(&g_env, NULL) != 0) {
            LOG() << "Failed attaching current thread";
            return false;
        }
    } else if (getEnvStat == JNI_EVERSION) {
        // Lets hope this doesn't happen
        LOG() << "Env version not supported";
        return false;
    } else if (getEnvStat == JNI_OK) {
        // LOG() << "Success getting env";
    }

    // Get the app's data path from Java
    jobject path = g_env->CallStaticObjectMethod(NativeLibClass, getDataDirMethod);
    const char* dataDir = g_env->GetStringUTFChars((jstring)path, NULL);

    // Get the asset manager
    jobject jam = g_env->CallStaticObjectMethod(NativeLibClass, getAssetManagerMethod);
    gassetmgr = AAssetManager_fromJava(g_env, jam);

    // Set the directory paths
    sprintf(gszTempDir, "%s/tmp", dataDir);
    sprintf(gszMissionPacksDir, "%s/MissionPacks", dataDir);
    sprintf(gszMissionPackInfosDir, "%s/MissionPackInfos", dataDir);
    sprintf(gszSaveGamesDir, "%s/SaveGames", dataDir);
    sprintf(gszCompletesDir, "%s/Completes", dataDir);
    sprintf(gszPrefsFilename, "%s/prefs.json", dataDir);

    // Make the directories
    mkdir(gszTempDir, 0755);
    mkdir(gszMissionPacksDir, 0755);
    mkdir(gszMissionPackInfosDir, 0755);
    mkdir(gszSaveGamesDir, 0755);
    mkdir(gszCompletesDir, 0755);

    // SDL HttpService with Curl
    extern HttpService *gphttp;
    gphttp = (HttpService *)new SdlHttpService();

    // Android specific chat controller
    g_pchat = new wi::AndroidChatController();

    return true;
}

void HostHelpers::Cleanup() {
    delete gphttp;
}

const char *HostHelpers::GetMainDataDir() {
    return NULL;
}

const char *HostHelpers::GetTempDir() {
	return gszTempDir;
}

const char *HostHelpers::GetMissionPacksDir() {
	return gszMissionPacksDir;
}

const char *HostHelpers::GetMissionPackInfosDir() {
	return gszMissionPackInfosDir;
}

const char *HostHelpers::GetSaveGamesDir() {
	return gszSaveGamesDir;
}

const char *HostHelpers::GetCompletesDir() {
	return gszCompletesDir;
}

const char *HostHelpers::GetPrefsFilename() {
	return gszPrefsFilename;
}

void HostHelpers::OpenUrl(const char *pszUrl) {
    jstring url = g_env->NewStringUTF(pszUrl);
    g_env->CallStaticVoidMethod(NativeLibClass, openUrlMethod, url);
    g_env->DeleteLocalRef(url);
}

void HostHelpers::Log(const char *pszFormat, va_list va)
{
#if 0
    LOGX() << base::Log::vFormat(pszFormat, va);
#endif

    char sz[512];
    vsnprintf(sz, sizeof(sz), pszFormat, va);

    LOGD("%s\n", sz);
}

void HostHelpers::Log(const char *pszFormat, ...)
{
    va_list va;
    va_start(va, pszFormat);
    Log(pszFormat, va);
    va_end(va);
}

void HostHelpers::MessageBox(const char *pszFormat, va_list va)
{
    Log(pszFormat, va);
}

void HostHelpers::Break()
{
    Log("BREAK!!");
}

void HostHelpers::GetSurfaceProperties(SurfaceProperties *pprops)
{
    int cxScreen, cyScreen;

    // Get the screen size from Java
    int screenWidth = g_env->CallStaticIntMethod(NativeLibClass, screenWidthMethod);
    int screenHeight = g_env->CallStaticIntMethod(NativeLibClass, screenHeightMethod);
    int screenDPI = g_env->CallStaticIntMethod(NativeLibClass, screenDPIMethod);

    // HT wants screen points not screen pixels...
    float density = (float)screenDPI / 160;
    cxScreen = screenWidth / density;
    cyScreen = screenHeight / density;

    pprops->cxWidth = cxScreen;
    pprops->cyHeight = cyScreen;
    pprops->cbxPitch = 1;
    pprops->cbyPitch = pprops->cxWidth;
    pprops->ffFormat = wi::kfDirect8;
    pprops->density = density;
}

bool HostHelpers::DirExists(char *psz)
{
    AAssetDir *assetDir = AAssetManager_openDir(gassetmgr, psz);
    bool exists = AAssetDir_getNextFileName(assetDir) != NULL;
    AAssetDir_close(assetDir);
    return exists;
}

bool HostHelpers::EnumFiles(Enum *penm, int key, char *pszFn, int cbFn)
{
    if (penm->m_pvNext == (void *)kEnmFirst) {
        AAssetDir *assetDir = AAssetManager_openDir(gassetmgr, gpakr.BottomDir());
        penm->m_pvNext = (void *)assetDir;
        penm->m_dwUser = 0;
    }

    AAssetDir *assetDir = (AAssetDir *)penm->m_pvNext;
    const char *ent = AAssetDir_getNextFileName(assetDir);
    if (ent != NULL) {
        strncpyz(pszFn, ent, cbFn);
        penm->m_dwUser++;
        return true;
    }
    AAssetDir_close(assetDir);
    return false;
}

/*
void HostHelpers::FrameStart()
{
#if 0
    [g_appDelegate frameStart];
#else
    Log("HostHelpers::FrameStart not implemented yet");
#endif
}

void HostHelpers::FrameComplete(int cfrmm, UpdateMap **apupd, Rect *arc,
        bool fScrolled)
{
#if 0
    [g_appDelegate frameComplete:cfrmm maps:apupd rects:arc scrolled:fScrolled];
#else
    Log("HostHelpers::FrameComplete not implemented yet");
#endif
}

void HostHelpers::ResetScrollOffset()
{
#if 0
    [g_appDelegate resetScrollOffset];
#else
    Log("HostHelpers::ResetScrollOffset not implemented yet");
#endif
}

void HostHelpers::SetFormMgrs(FormMgr *pfrmmSim, FormMgr *pfrmmInput)
{
#if 0
    return [g_appDelegate setFormMgrs:pfrmmSim input:pfrmmInput];
#else
    Log("HostHelpers::SetFormMgrs not implemented yet");
#endif
}

DibBitmap *HostHelpers::CreateFrontDib(int cx, int cy, int nDegreeOrientation)
{
#if 0
    return [g_appDelegate createFrontDibWithOrientation:nDegreeOrientation width:cx height:cy];
#else
    Log("HostHelpers::CreateFrontDib not implemented yet");
    return NULL;
#endif
}

void HostHelpers::SetPalette(Palette *ppal)
{
#if 0
    [g_appDelegate setPalette:ppal];
#else
    Log("HostHelpers::SetPalette not implemented yet");
#endif
}

bool HostHelpers::IsExiting()
{
#if 0
    return [g_appDelegate isExiting];
#else
    Log("HostHelpers::IsExitig not implemented yet");
    return false;
#endif
}
*/

const char *HostHelpers::GetUdid()
{
    jobject androidID = g_env->CallStaticObjectMethod(NativeLibClass, getAndroidIDMethod);
    return g_env->GetStringUTFChars((jstring)androidID, NULL);
}

void HostHelpers::InitiateAsk(const char *title, int max, const char *def,
        int keyboard, bool secure)
{
    jstring jtitle = g_env->NewStringUTF(title);
    jstring jdef = g_env->NewStringUTF(def);

    g_env->CallStaticVoidMethod(NativeLibClass, initiateAskMethod, jtitle, (jint)max, jdef, (jint)keyboard, secure ? 1 : 0);

    g_env->DeleteLocalRef(jtitle);
    g_env->DeleteLocalRef(jdef);
}

void HostHelpers::GetAskString(char *psz, int cb)
{
    jobject jstr = g_env->CallStaticObjectMethod(NativeLibClass, getAskStringMethod);
    const char* cstr = g_env->GetStringUTFChars((jstring)jstr, NULL);
    wi::strncpyz(psz, cstr, cb);
}

IChatController *HostHelpers::GetChatController()
{
    if (g_pchat == NULL) {
        g_pchat = new wi::AndroidChatController();
    }
    return g_pchat;
}

void HostHelpers::InitiateWebView(const char *title, const char *url) {
    jstring jstrUrl = g_env->NewStringUTF(url);
    g_env->CallStaticVoidMethod(NativeLibClass, initiateWebViewMethod, jstrUrl);
}

const char *HostHelpers::GetPlatformString() {
    jobject jstr = g_env->CallStaticObjectMethod(NativeLibClass, getPlatformStringMethod);
    return g_env->GetStringUTFChars((jstring)jstr, NULL);
}

/*
void HostHelpers::GameThreadStart(void *pv) {
    Log("Starting game...");
    wi::GameMain((char *)""); 
}

int HostHelpers::main(int argc, char **argv)
{

}
*/

JavaVM *HostHelpers::GetJavaVM() {
    return g_jvm;
}

JNIEnv *HostHelpers::GetJNIEnv() {
    return g_env;
}

} // namespace wi
