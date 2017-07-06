#include "ht.h"
#include <libgen.h>
#include <SDL.h>

namespace wi {

TBitmap::TBitmap()
{
    m_pszFn = NULL;
    m_x = 0;
    m_y = 0;
    m_cx = 0;
    m_cy = 0;
    m_anSideMap = NULL;
}

TBitmap::~TBitmap()
{
    if (m_anSideMap != NULL)
        delete[] m_anSideMap;
    m_anSideMap = NULL;
    if (m_pszFn != NULL)
        delete[] m_pszFn;
    m_pszFn = NULL;
}

TBitmap *CreateTBitmap(char *pszName) {
    return gptam->CreateTBitmap(pszName);
}

bool TBitmap::Init(char *pszFn, int x, int y, int cx, int cy, int cxOrig, int cyOrig,
    int ccLeft, int ccTop, int *anSideMap)
{
    m_pszFn = AllocString(pszFn);
    m_x = x;
    m_y = y;
    m_cx = cx;
    m_cy = cy;
    m_cxOrig = cxOrig;
    m_cyOrig = cyOrig;
    m_ccLeft = ccLeft;
    m_ccTop = ccTop;
    m_anSideMap = anSideMap;

    return true;
}

void TBitmap::BltTo(class DibBitmap *pbmDst, int xDst, int yDst, Rect *prcSrc)
{
    gptam->BltTo(this, pbmDst, xDst, yDst, (Side)1, prcSrc);
}

void TBitmap::BltTo(class DibBitmap *pbmDst, int xDst, int yDst, Side side, Rect *prcSrc)
{
    // Flash unit?

    if (side == (Side)-1) {
        DibBitmap *pbm = Flash();
        pbmDst->Blt(pbm, prcSrc, xDst, yDst);
        delete pbm;
        return;
    }

    // Normal blt

    gptam->BltTo(this, pbmDst, xDst, yDst, side, prcSrc);
}

void TBitmap::GetTextureSize(Size *psiz)
{
    psiz->cx = m_cx;
    psiz->cy = m_cy;
}

void TBitmap::GetSize(Size *psiz)
{
    psiz->cx = m_cxOrig;
    psiz->cy = m_cyOrig;
}

void TBitmap::GetPosition(Point *ppos)
{
    ppos->x = m_x;
    ppos->y = m_y;
}

int TBitmap::GetAtlas(Side side)
{
    return m_anSideMap[(int)side];
}

int TBitmap::GetBaseline()
{
    // Todo
    return 0;
}

void TBitmap::FillTo(class DibBitmap *pbmDst, int xDst, int yDst,
        int cxDst, int cyDst, int xOrigin, int yOrigin)
{
    Rect rcclp;

    // For every y point that is disivible by m_cy

    for (int y = 0; y < cyDst; y++) {
        if (!(y % m_cy)) {

            // For every x point that is disivible by m_cx

            for (int x = 0; x < cxDst; x++) {
                if (!(x % m_cx)) {

                    // Blt and clip appropriately

                    rcclp.Set(xDst + x, yDst + y, cxDst, cyDst);
                    BltTo(pbmDst, xDst + x, yDst + y);
                }
            }
        }
    }
}

DibBitmap *TBitmap::Flash() {
    // Returns a DibBitmap of this TBitmap where all
    // the pixels have been changed to (255, 255, 255)
    // while maintaining their original alpha value

#if defined(SDL)

    // Blt from the texture atlas into a dib

    DibBitmap *pbm = CreateDibBitmap(NULL, m_cxOrig, m_cyOrig);
    BltTo(pbm, 0, 0, ksidmSide1);

    // Read pixels from dib

    int cpixels = m_cxOrig * m_cyOrig;
    dword *pixels = new dword[cpixels];
    int pitch = m_cxOrig * sizeof(dword);

    SDL_Renderer *renderer = gpdisp->Renderer();
    if (SDL_SetRenderTarget(renderer, pbm->Texture()) != 0)
        return NULL;

    // SDL_RenderReadPixels() is a slow operation

    if (SDL_RenderReadPixels(renderer, NULL, 0, pixels, pitch) != 0)
        return NULL;

    // Loop through the pixels and convert to (255, 255, 255, a)

    byte r, g, b, a;
    for (int i = 0; i < cpixels; i++) {
        SDL_GetRGBA(pixels[i], pbm->m_ppfmt, &r, &g, &b, &a);
        pixels[i] = SDL_MapRGBA(pbm->m_ppfmt, 255, 255, 255, a);
    }

    delete pbm;
    return CreateDibBitmap(pixels, m_cxOrig, m_cyOrig);
#endif // SDL
}

} // namespace wi
