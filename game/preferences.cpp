#include "ht.h"
#include "yajl/wrapper/jsonbuilder.h"

namespace wi {

Preferences::Preferences() {
    m_pmap = NULL;
    m_pszJson = NULL;
}

Preferences::~Preferences() {
    delete m_pmap;
    delete[] m_pszJson;
}


Preferences *PrefsFromFile() {
    Preferences *pprefs = new Preferences();
    Assert(pprefs != NULL, "out of memory!");
    if (pprefs == NULL)
        return NULL;

    if (!pprefs->InitFromFile()) {
        delete pprefs;
		return NULL;
	}

    return pprefs;
}

Preferences *PrefsFromDefaults() {
    Preferences *pprefs = new Preferences();
    Assert(pprefs != NULL, "out of memory!");
    if (pprefs == NULL)
        return NULL;

    if (!pprefs->InitFromDeafults()) {
        delete pprefs;
		return NULL;
	}

    return pprefs;
}

bool Preferences::InitFromFile() {
    FileHandle hf = HostOpenFile(HostGetPrefsFilename(), kfOfRead);
    if (hf == NULL)
        return NULL;

    // Read length
    
    dword cb;
    HostSeekFile(hf, 0, kfSeekEnd);
    cb = HostTellFile(hf);
    HostSeekFile(hf, 0, kfSeekSet);

    // Read prefs

    byte *pb = new byte[cb];
    if (HostReadFile(pb, cb, 1, hf) != 1) {
        HostCloseFile(hf);
        return false;
    }
    HostCloseFile(hf);

    m_pszJson = (char *)pb;
    if (m_pszJson == NULL)
        return false;

    json::JsonBuilder builder;
    builder.Start();

    if (!builder.Update(m_pszJson, (int)strlen(m_pszJson)))
        return false;

    json::JsonObject *obj = builder.End();
    if (obj == NULL)
        return false;

    m_pmap = (json::JsonMap *)obj;
    return true;
}

bool Preferences::InitFromDeafults() {
    m_pmap = new json::JsonMap();

    Date date;
    HostGetCurrentDate(&date);
    Set(knPrefYearLastRun, date.nYear);
    Set(knPrefMonthLastRun, date.nMonth);
    Set(knPrefDayLastRun, date.nDay);

    Set(kfPrefAnonymous, 0);
    Set(knPrefVolume, 100);
    Set(kfPrefSoundMuted, false);
    Set(kwfPrefPerfOptions, kfPerfMax);
    Set(knPrefGameSpeed, kctUpdate / 2);
    Set(kwfPrefHandicap, kfHcapDefault);
    Set(knPrefScrollSpeed, 1.0f);
    Set(kszPrefAskUrl, "http://");
    Set(kszPrefDeviceId, HostGenerateDeviceId());
    Set(knPrefUpdateDisplay, 8); // 125 FPS

    return true;
}

bool Preferences::Save() {
    FileHandle hf = HostOpenFile(HostGetPrefsFilename(), kfOfWrite);
    if (hf == NULL)
        return false;
    const char *psz = m_pmap->ToJson();
    dword cb = (dword)strlen(psz);
    if (HostWriteFile((void *)psz, sizeof(char), cb, hf) < cb) {
        HostCloseFile(hf);
        return false;
    }
    HostCloseFile(hf);
    return true;
}

const char *Preferences::GetString(const char *key) {
    return m_pmap->GetString(key);
}

int Preferences::GetInteger(const char *key) {
    return m_pmap->GetInteger(key);
}

float Preferences::GetFloat(const char *key) {
    return m_pmap->GetFloat(key);
}

bool Preferences::GetBool(const char *key) {
    return m_pmap->GetBool(key);
}

void Preferences::Set(const char *key, const char *psz) {
    m_pmap->SetObject(key, json::NewJsonString(psz, (int)strlen(psz)));
}

void Preferences::Set(const char *key, int n) {
    m_pmap->SetObject(key, new json::JsonNumber(n));
}

void Preferences::Set(const char *key, float n) {
    m_pmap->SetObject(key, new json::JsonNumber(n));
}

void Preferences::Set(const char *key, bool f) {
    m_pmap->SetObject(key, new json::JsonBool(f));
}

} // namespace wi
