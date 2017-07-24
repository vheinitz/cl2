#ifndef FEATURE_Convexity_HG
#define FEATURE_Convexity_HG

#include "feature.h"

//Implementation of the Feature Convexity
// Convexity is defined as the ratio of perimeters of the convex hull over the shape contour


class FeatureConvexity : public Feature
{
public:
	FeatureConvexity();
	virtual ~FeatureConvexity();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};

#endif