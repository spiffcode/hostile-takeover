#include "ht.h"
#include "yajl/wrapper/jsontypes.h"
#include "yajl/wrapper/jsonbuilder.h"

namespace wi {

Font *LoadFont(char *pszFont)
{
	Font *pfnt = new Font;
	Assert(pfnt != NULL, "out of memory!");
	if (pfnt == NULL)
		return NULL;
	if (!pfnt->Load(pszFont)) {
		delete pfnt;
		return NULL;
	}
	return pfnt;
}

Font::Font()
{
    m_nGlyphOverlap = 0;
    m_nLineOverlap = 0;
    m_cxEllipsis = 0;
    m_cy = 0;
    m_ptbmDefault = NULL;
}

Font::~Font()
{
    delete m_ptbmDefault;
}

bool Font::Load(char *pszFont)
{
    // Read json

    FileMap fmap;
	char *pszJson = (char *)gpakr.MapFile(pszFont, &fmap);

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
    gpakr.UnmapFile(&fmap);

    // Font json map

    json::JsonMap *map = (json::JsonMap *)obj;

    // Map of characters to glyph image filenames

    json::JsonMap *glyphMap = (json::JsonMap *)map->GetObject("glyph_map");
    if (glyphMap == NULL) {
        delete map;
        return false;
    }

    // Iterate over all the characters and cache corresponding TImages via std::map

    Enum enm;
    const char *key;
    while ((key = glyphMap->EnumKeys(&enm)) != NULL) {

        json::JsonString *glyph = (json::JsonString *)glyphMap->GetObject(key);
        if (glyph == NULL) {
            delete map;
            return false;
        }

        char szGlyphFn[64];
        strncpyz(szGlyphFn, glyph->GetString(), sizeof(szGlyphFn));

        if (!m_map.insert(std::make_pair(std::string(key), CreateTBitmap(szGlyphFn))).second) {
            LOG() << "Error adding \"" << key << "\" to font map";
            delete map;
            return false;
        }
    }

    // Default character, for when requested character isn't mapped

    char sz[64];
    strncpyz(sz, map->GetString("default"), sizeof(sz));
    m_ptbmDefault = CreateTBitmap(sz);
    if (m_ptbmDefault == NULL) {
        delete map;
        return false;
    }

    m_cy = map->GetInteger("height");
    m_nGlyphOverlap = map->GetInteger("glyph_overlap");
    m_nLineOverlap = map->GetInteger("line_overlap");
    
    if (TBitmapExists('.'))
        m_cxEllipsis = GetTextExtent("...");

    delete map;
    return true;
}

int Font::GetTextExtent(const char *psz)
{
    return GetTextExtent(psz, (int)strlen(psz));
}

int Font::GetTextExtent(const char *psz, int cch)
{
    int width = 0;
    for (int i = 0; i < cch; i++) {
        width += GetTBitmap(psz[i])->Width() - m_nGlyphOverlap;
    }
    return width;
}

int Font::CalcMultilineHeight(char *psz, int cxMultiline)
{
	int cy = 0;
	char *pszNext = psz;
	while (pszNext != NULL) {
		char *pszStart = pszNext;
		CalcBreak(cxMultiline, &pszNext);
		cy += m_cy - m_nLineOverlap;
	}

	return cy;
}

void Font::DrawText(DibBitmap *pbm, char *psz, int x, int y, int cx, int cy,
        bool fEllipsis)
{
    int cyFont = m_cy - m_nLineOverlap;
	int cyT = cyFont;
	char *pszNext = psz;
	while (pszNext != NULL) {
		if (cy != -1 && cyT > cy)
			return;
		char *pszStart = pszNext;
		int cch = CalcBreak(cx, &pszNext);
        if (fEllipsis && pszNext != NULL && cy != -1 && cyT + cyFont > cy) {
            DrawTextWithEllipsis(pbm, pszStart, cch, x, y, cx, true);
        } else if (fEllipsis && pszNext == NULL) {
            DrawTextWithEllipsis(pbm, pszStart, cch, x, y, cx, false);
        } else {
            DrawText(pbm, pszStart, x, y, cch);
        }
		y += cyFont;
		cyT += cyFont;
	}
}

void Font::DrawTextWithEllipsis(DibBitmap *pbm, char *psz, int cch, int x,
        int y, int cx, bool fForce)
{
    if (!fForce) {
        // If not being forced and text fits, draw it without ellipsis
        if (GetTextExtent(psz) < cx) {
            DrawText(pbm, psz, x, y, (int)strlen(psz));
            return;
        }
    }

    // Do a binary search to find out where the ellipsis is needed

    char szT[256];
    int imin = 0;

    // Convert to index and ensure room for ellipsis
    int imax = _min(cch - 1, (int)sizeof(szT) - 3 - 1);
    if (imax < 0) {
        imax = 0;
    }
    int icur = imax;
    int ifit = imax;
    while (true) {
        int icurT = imin + (imax - imin) / 2;
        if (icurT == icur) {
            break;
        }
        icur = icurT;
        int cxT = GetTextExtent(psz, icur + 1);
        if (cxT >= cx - m_cxEllipsis) {
            imax = icur - 1;
        } else {
            ifit = icur;
            imin = icur + 1;
        }
    }
    strncpyz(szT, psz, ifit + 2); // convert to count, add one for 0
    strcat(szT, "..."); // this fits without checks
    DrawText(pbm, szT, x, y, (int)strlen(szT));
}

#define IsBreakingChar(ch) ((ch) == ' ' || (ch) == '\t')

char *Font::FindNextNonBreakingChar(char *psz)
{
	while (*psz != 0) {
		if (!IsBreakingChar(*psz))
			return psz;
		psz++;
	}
	return NULL;
}

int Font::CalcBreak(int cx, char **ppsz, bool fChop)
{
	char *pchBreakAfter = NULL;
	int cchBreak = 0;
	int cxT = 0;
	char *pchT = *ppsz;
	int cch = 0;
	bool fFoundBreak = false;

	while (*pchT != 0) {
		char ch = *pchT;

		// Handle any combo of \n, \r, \n\r, or \r\n

		switch (ch) {
		case '\n':
			pchT++;
			if (*pchT == '\r')
				pchT++;
			*ppsz = FindNextNonBreakingChar(pchT);
			return cch;

		case '\r':
			pchT++;
			if (*pchT == '\n')
				pchT++;
			*ppsz = FindNextNonBreakingChar(pchT);
			return cch;
		}

		// Otherwise if breaking char, remember break point

		if (IsBreakingChar(*pchT)) {
			fFoundBreak = true;
			pchBreakAfter = FindNextNonBreakingChar(pchT);
			cchBreak = cch;
		}

		// At right edge yet?

        int cxChar = GetTBitmap(pchT[0])->Width() - m_nGlyphOverlap;
		if (cxT + cxChar > cx) {
			// If last break exists, use it. Return pointer skips past break char

			if (fFoundBreak) {
				*ppsz = pchBreakAfter;
				return cchBreak;
			}

			// If no last break, sometimes callers want word chopping, sometimes not

			if (!fChop)
				return 0;

			// No last break; Scan forward for a breaking char so we know where to start
			// the next line

			while (*pchT != 0) {
				// Start the next past the break; only draw what fits in cx for this line

				if (IsBreakingChar(*pchT)) {
					*ppsz = FindNextNonBreakingChar(pchT);
					return cch;
				}

				// If carriage return, start at next char

				if (ch == '\n') {
					*ppsz = pchT + 1;
					return cch;
				}

				pchT++;
			}

			// At end of line with no break. Return length that fits in cx, and NULL to indicate done.

			if (*pchT == 0) {
				*ppsz = NULL;
				return cch;
			}
		}

		// Add char; next char

		cxT += cxChar;
		cch++;
		pchT++;
	}

	// Done without hitting edge. Return length and NULL to indicate done

	*ppsz = NULL;
	return cch;
}

int Font::DrawText(DibBitmap *pbm, char *psz, int x, int y, int cch, Color *pclr)
{
    if (cch == -1)
		cch = (int)strlen(psz);

    int pos = 0;
    for (int i = 0; i < cch; i++) {

        // Draw the character
        // If character unavailable, GetTBitmap() will return the default character

        GetTBitmap(psz[i])->BltTo(pbm, x + pos, y);
        pos = pos + GetTBitmap(psz[i])->Width() - m_nGlyphOverlap;
    }

    return 0;
}

TBitmap *Font::GetTBitmap(char sz)
{
    std::string str(1, sz);
    FontMap::iterator it = m_map.find(str);
    if (it == m_map.end()) {
        // LOG() << "Missing character \"" << str << "\"";
        return m_ptbmDefault;
    }
    return it->second;
}

bool Font::TBitmapExists(char sz)
{
    std::string str(1, sz);
    FontMap::iterator it = m_map.find(str);
    return it != m_map.end();
}

} // namespace wi
