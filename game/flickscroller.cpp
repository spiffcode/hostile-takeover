#include "ht.h"
#include <math.h>

namespace wi {

FlickScroller::FlickScroller()
{
    m_fHasMagnitude = false;
}
    
bool FlickScroller::Init(int nPen, float flMultiplier,
                    float flDecayPercent, float cmsDecaySpan, bool fChoose)
{
    // Get the best flick vector.
    
    if (!fChoose) {    
        gevm.GetFlickVector(nPen, &m_fliv);
    } else {
        FlickVector flivPreferred;
        FlickVector flivAlternate;    
        gevm.GetFlickVector(nPen, &flivPreferred);        
        gevm.GetFlickVector(nPen ^ 3, &flivAlternate);
        if (flivAlternate.GetMagnitude() > flivPreferred.GetMagnitude()) {
            m_fliv = flivAlternate;
        } else {
            m_fliv = flivPreferred;
        }
    }
       
    //Trace("FlickVector: dx=%d dy=%d ms=%ld", m_fliv.dx, m_fliv.dy, m_fliv.cms);
    m_msStart = HostGetMillisecondCount();
    m_flMultiplier = flMultiplier;
    m_flDecayPercent = flDecayPercent;
    m_cmsDecaySpan = cmsDecaySpan;
    return CheckMagnitude(m_fliv.dx, m_fliv.dy);
}
    
void FlickScroller::Clear()
{
    //Trace("FlickVector cleared!");
    m_fHasMagnitude = false;
}
    
bool FlickScroller::GetPosition(Point *ppt)
{
    if (!m_fHasMagnitude)
        return false;
    
    float dx = 0.0f;
    float dy = 0.0f;
    
#if 0
    float flDecayMultiplier = 1.0f;
    
    // Not exact, but close enough.
    float flDecayPercent = ((float)m_fliv.cms / (float)m_cmsDecaySpan) * m_flDecayPercent;

    dword cmsDelta = HostGetMillisecondCount() - m_msStart;        
    int count = ((float)cmsDelta / (float)m_fliv.cms + 0.5f);
    while (count-- != 0 && m_fHasMagnitude) {
        float dxT = m_fliv.dx * flDecayMultiplier;
        float dyT = m_fliv.dy * flDecayMultiplier;
        flDecayMultiplier = flDecayMultiplier * (1.0f - flDecayPercent);
        dx += dxT;
        dy += dyT;
        CheckMagnitude(dxT, dyT);
    }
    ppt->x = (int)(dx + 0.5f);
    ppt->y = (int)(dy + 0.5f);
#else
    
    float flDecayMultiplier = 1.0f * ((float)m_cmsDecaySpan / (float)m_fliv.cms);
    float flDecayPercent = m_flDecayPercent;
    
    dword cmsDelta = HostGetMillisecondCount() - m_msStart;        
    int count = ((float)cmsDelta / (float)m_cmsDecaySpan + 0.5f);
    while (count-- != 0 && m_fHasMagnitude) {
        float dxT = m_fliv.dx * flDecayMultiplier;
        float dyT = m_fliv.dy * flDecayMultiplier;
        flDecayMultiplier = flDecayMultiplier * (1.0f - flDecayPercent);
        dx += dxT;
        dy += dyT;
        CheckMagnitude(dxT, dyT);
    }
    ppt->x = (int)(dx + 0.5f);
    ppt->y = (int)(dy + 0.5f);
#endif    
    
    //Trace("FS::GetPosition()-> x=%d y=%d ms=%ld", ppt->x, ppt->y, HostGetMillisecondCount());
    
    return true;
}

bool FlickScroller::HasMagnitude()
{
    return m_fHasMagnitude;
}

bool FlickScroller::CheckMagnitude(float dx, float dy)
{
    m_fHasMagnitude = (fabsf(dx) > 0.1f || fabsf(dy) > 0.1f);
    return m_fHasMagnitude;
}

}; // namespace wi
