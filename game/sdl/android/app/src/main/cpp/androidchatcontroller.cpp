#include <jni.h>
#include <cctype>
#include "androidchatcontroller.h"
#include "game/sdl/hosthelpers.h"

jclass ChatControllerClass;
jmethodID clearMethod;
jmethodID addChatMethod;
jmethodID showMethod;
jmethodID hideMethod;
jmethodID setTitleMethod;
jmethodID getTitleMethod;

jobject jchatc; // java chat controller

JavaVM *g_jvm;
JNIEnv *g_env;

// This is the game thread callable class for controlling chat
// These get called on the game thread, and get passed on to Java
namespace wi {

IChatControllerCallback *pcccb_;
wi::AndroidChatController *chatc_;

AndroidChatController::AndroidChatController() {
    g_jvm = HostHelpers::GetJavaVM();
    g_env = HostHelpers::GetJNIEnv(); // JNIEnv for this thread

    pcccb_ = NULL;
    chatc_ = this;

    // Make a tmp class then declair the real class as a global reference
    jclass tmp = g_env->FindClass("com/spiffcode/ht/ChatController");
    ChatControllerClass = (jclass)g_env->NewGlobalRef(tmp);

    // Setup an instance of the java chat object
    jmethodID constructor = g_env->GetMethodID(ChatControllerClass, "<init>", "()V");
    jchatc = g_env->NewObject(ChatControllerClass, constructor);

    // Setup the JNI method calls
    clearMethod = g_env->GetMethodID(ChatControllerClass, "clear", "()V");
    addChatMethod = g_env->GetMethodID(ChatControllerClass, "addChat", "(Ljava/lang/String;Ljava/lang/String;I)V");
    showMethod = g_env->GetMethodID(ChatControllerClass, "show", "()V");
    hideMethod = g_env->GetMethodID(ChatControllerClass, "hide", "()V");
    setTitleMethod = g_env->GetMethodID(ChatControllerClass, "setTitle", "(Ljava/lang/String;)V");
    getTitleMethod = g_env->GetMethodID(ChatControllerClass, "getTitle", "()Ljava/lang/String;");
}

void AndroidChatController::Clear() {
    g_env->CallVoidMethod(jchatc, clearMethod);
}

void AndroidChatController::AddChat(const char *player, const char *chat) {

    int sig = 0; // Is player a mod/admin with a sig?
    size_t length;

    // NewStringUTF() doesn't work with weird characters, so just
    // convert any non ASCII characters to "?" in the player and chat

    std::string strPlayer = player;
    length = strPlayer.length();
    for (int i = 0; i < length; i++) {
        if (!isascii(strPlayer[i])) {
            if (strPlayer[i] == '\xa0') {
                // Mods can put \xa0 in their name to prove they are a mod
                // as that is an illegal username character. To get around this,
                // convert it to "+" then Java will convert any "+" back to
                // \xa0 if sig is set to 1
                strPlayer.replace(i, 1, "+");
                sig = 1;
            } else {
                strPlayer.replace(i, 1, "?");
            }

        }
    }

    std::string strChat = chat;
    length = strChat.length();
    for (int i = 0; i < length; i++) {
        if (!isascii(strChat[i])){
            if (strChat[i] == '\xa0' && strPlayer.empty()) {
                // Hack to allow the server to use the '\xa0' character in server chats
                strChat.replace(i, 1, "+");
                sig = 1;
            } else {
                strChat.replace(i, 1, "?");
            }
        }
    }

    // Convert to jstring
    jstring jplayer = g_env->NewStringUTF(strPlayer.c_str());
    jstring jchat = g_env->NewStringUTF(strChat.c_str());

    // Send the data to java
    g_env->CallVoidMethod(jchatc, addChatMethod, jplayer, jchat, (jint)sig);

    // Release the jstrings
    g_env->DeleteLocalRef(jplayer);
    g_env->DeleteLocalRef(jchat);
}

void AndroidChatController::Show(bool fShow) {
    if (fShow) {
        g_env->CallVoidMethod(jchatc, showMethod);
    } else {
        g_env->CallVoidMethod(jchatc, hideMethod);
    }
}

void AndroidChatController::SetTitle(const char *title) {
    jstring jtitle = g_env->NewStringUTF(title);
    g_env->CallVoidMethod(jchatc, setTitleMethod, jtitle);
    g_env->DeleteLocalRef(jtitle);
}

const char *AndroidChatController::GetTitle() {
    jobject title = g_env->CallObjectMethod(jchatc, getTitleMethod);
    return g_env->GetStringUTFChars((jstring)title, NULL);
}

IChatControllerCallback *AndroidChatController::SetCallback(
        IChatControllerCallback *pcccb) {
    IChatControllerCallback *old = pcccb_;
    pcccb_ = pcccb;
    return old;
}

void AndroidChatController::OnMessage(base::Message *pmsg) {
    switch (pmsg->id) {
    case kidmOnDismissed:
        if (pcccb_ != NULL) {
            pcccb_->OnChatDismissed();
        }
        break;

    case kidmOnSend:
        if (pcccb_ != NULL) {
            ChatParams *params = (ChatParams *)pmsg->data;
            pcccb_->OnChatSend(params->chat.c_str());
            delete params;
        }
        break;

    case kidmOnPlayers:
        if (pcccb_ != NULL) {
            pcccb_->OnPlayers();
        }
        break;
    }
}

} // namespace wi

#ifdef __cplusplus
extern "C" {
#endif

// These are the java callable methods for controlling chat

JNIEXPORT void JNICALL
Java_com_spiffcode_ht_ChatController_nativeOnSend(JNIEnv* env, jclass cls, jstring chat) {
    if (wi::chatc_ == NULL) {
        return;
    }

    const char *ccChat = env->GetStringUTFChars((jstring)chat, NULL);
    std::string strChat = ccChat;
    wi::ChatParams *params = new wi::ChatParams(strChat);
    wi::chatc_->thread().Post(kidmOnSend, wi::chatc_, params);
}

JNIEXPORT void JNICALL
Java_com_spiffcode_ht_ChatController_nativeOnDone(JNIEnv* env, jclass cls) {
    if (wi::chatc_ != NULL) {
        wi::chatc_->thread().Post(kidmOnDismissed, wi::chatc_);
    }
}

JNIEXPORT void JNICALL
Java_com_spiffcode_ht_ChatController_nativeOnPlayers(JNIEnv* env, jclass cls) {
    if (wi::chatc_ != NULL) {
        wi::chatc_->thread().Post(kidmOnPlayers, wi::chatc_);
    }
}

#ifdef __cplusplus
} // extern C
#endif
