#ifndef _scriptfeature_HG
#define _scriptfeature_HG

#include "feature.h"

class FeatureScript : public Feature
{
public:
	FeatureScript();
	virtual ~FeatureScript();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};
#endif