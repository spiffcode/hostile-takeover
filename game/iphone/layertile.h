#ifndef __LAYERTILE_H__
#define __LAYERTILE_H__

#include <CoreGraphics/CGLayer.h>

namespace wi {

#define USE_IMAGE
//#define USE_LAYER

class LayerTile {
public:
    LayerTile(CGContextRef ctx) {
#ifdef USE_LAYER
        CGSize size;
        size.width = gcxTile;
        size.height = gcyTile;
        layer_ = CGLayerCreateWithContext(ctx, size, NULL);
#endif
#ifdef USE_IMAGE
        image_ = NULL;
#endif
        updated_ = true;
    }
    ~LayerTile() {
#ifdef USE_LAYER
        if (layer_ != NULL) {
            CGLayerRelease(layer_);
        }
#endif
#ifdef USE_IMAGE
        if (image_ != NULL) {
            CGImageRelease(image_);
        }
#endif
    }

    void UpdateImage(CGImageRef image) {
#ifdef USE_LAYER
        if (layer_ != NULL) {
            CGContextRef ctx = CGLayerGetContext(layer_);
            CGRect rc = CGRectMake(0, 0, gcxTile, gcyTile);
            CGContextDrawImage(ctx, rc, image);
        }
#endif
#ifdef USE_IMAGE
        if (image_ != NULL) {
            CGImageRelease(image_);
        }
        image_ = image;
        if (image_ != NULL) {
            CGImageRetain(image_);
        }
#endif
        MarkUpdated();
    }

    void Draw(CGContextRef ctx, const CGRect& rc) {
#ifdef USE_LAYER
        if (layer_ != NULL) {
            CGContextDrawLayerInRect(ctx, rc, layer_);
        }
#endif
#ifdef USE_IMAGE
        if (image_ != NULL) {
            CGContextDrawImage(ctx, rc, image_);
        }
#endif
        ClearUpdated();
    }

    bool IsUpdated() {
#ifdef UPDATE_ALL
        return true;
#else
        return updated_;
#endif
    }

    void MarkUpdated() {
        updated_ = true;
    }

    void ClearUpdated() {
        updated_ = false;
    }

private:
#ifdef USE_LAYER
    CGLayerRef layer_;
#endif
#ifdef USE_IMAGE
    CGImageRef image_;
#endif
    bool updated_;

    friend class LayerMap;
};

} // namespace wi

#endif // __LAYERTILE_H__
