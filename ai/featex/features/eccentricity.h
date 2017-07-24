#ifndef FEATURE_ECCENTRICITY_HG
#define FEATURE_ECCENTRICITY_HG

#include "feature.h"

//Implementation of the Feature Eccentricity
// Eccentricity is measure of aspect ratio - the ratio of the length of major axis to the length of minor axis. 
// Using minimum bounding rectangle method


class FeatureEccentricity : public Feature
{
public:
	FeatureEccentricity();
	virtual ~FeatureEccentricity();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};

#endif