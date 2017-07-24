#ifndef FEATURE_Circularity_HG
#define FEATURE_Circularity_HG

#include "feature.h"

//Implementation of the Feature Circularity
// Circularity is measure of aspect ratio - the ratio of the length of major axis to the length of minor axis. 
// Using minimum bounding rectangle method


class FeatureCircularity : public Feature
{
public:
	FeatureCircularity();
	virtual ~FeatureCircularity();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};

#endif