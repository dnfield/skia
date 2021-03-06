/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCDrawPathsOp_DEFINED
#define GrCCDrawPathsOp_DEFINED

#include "SkTInternalLList.h"
#include "ccpr/GrCCPathParser.h"
#include "ccpr/GrCCPathProcessor.h"
#include "ccpr/GrCCSTLList.h"
#include "ops/GrDrawOp.h"

class GrCCAtlas;
class GrCCPerFlushResources;
struct GrCCRTPendingPaths;
class GrCoverageCountingPathRenderer;

/**
 * This is the Op that draws paths to the actual canvas, using atlases generated by CCPR.
 */
class GrCCDrawPathsOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrCCDrawPathsOp);

    GrCCDrawPathsOp(GrCoverageCountingPathRenderer*, GrPaint&&, const SkIRect& clipIBounds,
                    const SkMatrix&, const SkPath&, const SkRect& devBounds);
    ~GrCCDrawPathsOp() override;

    const char* name() const override { return "GrCCDrawOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*,
                                GrPixelConfigIsClamped) override;
    void wasRecorded(GrRenderTargetOpList*) override;
    bool onCombineIfPossible(GrOp* other, const GrCaps& caps) override;
    void visitProxies(const VisitProxyFunc& func) const override {
        fProcessors.visitProxies(func);
    }
    void onPrepare(GrOpFlushState*) override {}

    int countPaths(GrCCPathParser::PathStats*) const;
    void setupResources(GrCCPerFlushResources*, GrOnFlushResourceProvider*);
    SkDEBUGCODE(int numSkippedInstances_debugOnly() const { return fNumSkippedInstances; })

    void onExecute(GrOpFlushState*) override;

private:
    SkPath::FillType getFillType() const {
        SkASSERT(fNumDraws >= 1);
        return fDraws.head().fPath.getFillType();
    }

    struct AtlasBatch {
        const GrCCAtlas* fAtlas;
        int fEndInstanceIdx;
    };

    void addAtlasBatch(const GrCCAtlas* atlas, int endInstanceIdx) {
        SkASSERT(endInstanceIdx > fBaseInstance);
        SkASSERT(fAtlasBatches.empty() ||
                 endInstanceIdx > fAtlasBatches.back().fEndInstanceIdx);
        fAtlasBatches.push_back() = {atlas, endInstanceIdx};
    }

    GrCoverageCountingPathRenderer* const fCCPR;
    const uint32_t fSRGBFlags;

    struct SingleDraw {
        SkIRect fClipIBounds;
        SkMatrix fMatrix;
        SkPath fPath;
        GrColor fColor;
        SingleDraw* fNext;
    };

    GrCCSTLList<SingleDraw> fDraws;
    SkDEBUGCODE(int fNumDraws = 1);

    GrProcessorSet fProcessors;
    GrCCRTPendingPaths* fOwningRTPendingPaths = nullptr;

    int fBaseInstance;
    SkSTArray<1, AtlasBatch, true> fAtlasBatches;
    SkDEBUGCODE(int fNumSkippedInstances = 0);
};

#endif
