#include "ht.h"
#include "yajl/wrapper/jsonbuilder.h"
#include "base/thread.h"

namespace wi {

TexAtlasMgr::TexAtlasMgr()
{
    m_json = NULL;
    m_pbmAtlases = NULL;
    m_natlases = 0;
}
TexAtlasMgr::~TexAtlasMgr()
{
    delete m_json;
    m_json = NULL;
    for (int n = 0; n < m_natlases; n++) {
        if (m_pbmAtlases[n] != NULL)
            delete m_pbmAtlases[n];
    }
	delete[] m_pbmAtlases;
    m_pbmAtlases = NULL;
}

char *apszatlases[] = {
    "animations.png",
    "fonts.png",
    "bitmaps.png",
    "bitmaps0.png",
    "bitmaps1.png",
    "bitmaps2.png",
    "bitmaps3.png",
    "bitmaps4.png",
    "units0.png",
    "units1.png",
    "units2.png",
    "units3.png",
    "units4.png"
};

bool TexAtlasMgr::Init()
{
    m_natlases = ARRAYSIZE(apszatlases);
    m_pbmAtlases = new DibBitmap*[m_natlases];

    // Load atlases

    for (int i = 0; i < m_natlases; i++) {
        m_pbmAtlases[i] = LoadDibBitmap(apszatlases[i]);
        if (!m_pbmAtlases[i])
            return false;
    }

    // Read json

    FileMap fmap;
	char *pszJson = (char *)gpakr.MapFile("atlasmap.json", &fmap);
    if (!pszJson) {
        return false;
    }

    // Parse json

    json::JsonBuilder builder;
    builder.Start();
    if (!builder.Update(pszJson, (int)strlen(pszJson))) {
        gpakr.UnmapFile(&fmap);
        return false;
    }
    json::JsonObject *obj = builder.End();
    if (obj == NULL) {
        gpakr.UnmapFile(&fmap);
        return false;
    }

    m_json = (json::JsonMap *)obj;
    gpakr.UnmapFile(&fmap);
    return true;
}

void TexAtlasMgr::BltTo(TBitmap *ptbmSrc, DibBitmap *pbmDst, int xDst, int yDst, Side side, Rect *prcSrc)
{
    Size siz;
    ptbmSrc->GetTextureSize(&siz);

    Point pt;
    ptbmSrc->GetPosition(&pt);

    // Where do we want to blt from?

    Rect rc;
    if (prcSrc != NULL) {

        // NOTE: the clipping rect calculated in this block has not been
        // tested with an image that was was cropped by the atlas packer

        // prcSrc data needs to be converted to work within the texture atlas

        rc.left = pt.x + prcSrc->left - ptbmSrc->ClippedLeft();
        rc.top = pt.y + prcSrc->top - ptbmSrc->ClippedTop();
        rc.right = rc.left + prcSrc->Width();
        rc.bottom = rc.top + prcSrc->Height();

        // Set to image edge if anything is past image edge
        // to prevent other images in the texture from being
        // in the blt

        if (rc.left < pt.x)
            rc.left = pt.x;

        if (rc.top < pt.y)
            rc.top = pt.y;

        if (rc.right > pt.x + siz.cx)
            rc.right = pt.x + siz.cx;

        if (rc.bottom > pt.y + siz.cy)
            rc.bottom = pt.y + siz.cy;

    } else {
        // No prcSrc, the blt rect is just the location and size of the image

        rc.Set(pt.x, pt.y, pt.x + siz.cx , pt.y + siz.cy);
    }

    if (m_pbmAtlases[ptbmSrc->GetAtlas(side)]) {
        pbmDst->Blt(m_pbmAtlases[ptbmSrc->GetAtlas(side)],
            &rc, xDst + ptbmSrc->ClippedLeft(), yDst + ptbmSrc->ClippedTop());
    }
}

TBitmap *TexAtlasMgr::CreateTBitmap(char *pszName)
{
    json::JsonMap *entry = (json::JsonMap *)m_json->GetObject(pszName);
    if (entry == NULL) {

        // Scenery gobs and cut scene images are "hardcoded" into the levels
        // files as .tbm and .rbm. If pszName is a .tbm or .rbm, load as .png

        char *pszSuffix = strrchr(pszName, '.');
        if (pszSuffix) {
            if ((strcmp(pszSuffix, ".tbm") == 0) || (strcmp(pszSuffix, ".rbm") == 0)) {

                // Dump the filename, remove the suffix, then append the .png suffix

                char *psz = strdup(pszName);
                psz[strlen(pszName) - strlen(pszSuffix)] = '\0';
                char sz[MAX_PATH];
                sprintf(sz, "%s.png", psz);
                free(psz);

                return CreateTBitmap(sz);
            }
        }

        // Wasnt a .tbm or .rbm, loading failed

        LOG() << "couldn't find TBitmap in atlas json: " << pszName;
        return NULL;
    }

    int *anSideMap = new int[kcSides];
    if (anSideMap == NULL) {
        return NULL;
    }

    json::JsonArray *atlases = (json::JsonArray *)entry->GetObject("atlases");
    for (int i = 0; i < kcSides; i++) {
        anSideMap[i] = ((json::JsonNumber *)atlases->GetObject(i))->GetInteger();
    }

    TBitmap *ptbm = new TBitmap();
    Assert(ptbm != NULL, "out of memory!");
	if (ptbm == NULL)
		return NULL;
    if (!ptbm->Init(
        pszName,
        entry->GetInteger("x"),
        entry->GetInteger("y"),
        entry->GetInteger("cx"),
        entry->GetInteger("cy"),
        entry->GetInteger("cx_orig"),
        entry->GetInteger("cy_orig"),
        entry->GetInteger("cc_left"),
        entry->GetInteger("cc_top"),
        anSideMap)) {

		delete ptbm;
		return NULL;
	}
	return ptbm;
}

} // namespace wi
