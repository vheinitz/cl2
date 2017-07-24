#ifndef TESTING_HG
#define TESTING_HG

#include "feature.h"

//TODO Implement Features:
// http://www.math.uci.edu/icamp/summer/research_11/park/shape_descriptors_survey.pdf
// circularity
// Eccentricity
// Rectangularity
// Convexity
// Solidity: Shape / Convex Hull
// Euler Number: Connected Regions - Holes
// Hole Area Ratio: Area of Shape / Area of holes
// Mingqiang Yang, Kidiyo Kpalma, Joseph Ronsin. A Survey of Shape Feature Extraction Techniques. Peng-Yeng Yin. Pattern Recognition, IN-TECH, pp.43-90, 2008. <hal-00446037>


class FeatureLocalMax : public Feature
{
public:
	FeatureLocalMax();
	virtual ~FeatureLocalMax();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};

class FeaturePeaks : public Feature
{
public:
	FeaturePeaks();
	virtual ~FeaturePeaks();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};

class FeaturePeaksPerLine : public Feature
{
public:
	FeaturePeaksPerLine();
	virtual ~FeaturePeaksPerLine();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};

class FeatureEllipses : public Feature
{
public:
	FeatureEllipses();
	virtual ~FeatureEllipses();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};


class FeatureBlobs : public Feature
{
public:
	FeatureBlobs();
	virtual ~FeatureBlobs();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};

class FeatureNdnaIndex : public Feature
{
public:
	FeatureNdnaIndex();
	virtual ~FeatureNdnaIndex();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};

//TODO
class FeatureExposure : public Feature
{
public:
	FeatureExposure();
	virtual ~FeatureExposure();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};

class FeatureImgParam : public Feature
{
public:
	FeatureImgParam();
	virtual ~FeatureImgParam();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};

class FeatureNumOfStroks : public Feature
{
public:
	FeatureNumOfStroks();
	virtual ~FeatureNumOfStroks();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};

class FeatureSpinsPerLine : public Feature
{
public:
	FeatureSpinsPerLine();
	virtual ~FeatureSpinsPerLine();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};

class FeatureMatchHistogram : public Feature
{
public:
	FeatureMatchHistogram();
	virtual ~FeatureMatchHistogram();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};

class FeatureSequence : public Feature
{
public:
	FeatureSequence();
	virtual ~FeatureSequence();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};

class FeatureHuMean : public Feature
{
public:
	FeatureHuMean();
	virtual ~FeatureHuMean();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};

class FeatureCellMetrics : public Feature
{
public:
	FeatureCellMetrics();
	virtual ~FeatureCellMetrics();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};

class FeatureExif : public Feature
{
public:
	FeatureExif();
	virtual ~FeatureExif();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};

class FeatureCellVariance : public Feature
{
public:
	FeatureCellVariance();
	virtual ~FeatureCellVariance();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};

class FeatureCellTexture : public Feature
{
public:
	FeatureCellTexture();
	virtual ~FeatureCellTexture();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};

class FeaturePosMitosis : public Feature
{
public:
	FeaturePosMitosis();
	virtual ~FeaturePosMitosis();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};

class FeatureCellShape : public Feature
{
public:
	FeatureCellShape();
	virtual ~FeatureCellShape();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};

class FeatureFindTempl : public Feature
{
public:
	FeatureFindTempl();
	virtual ~FeatureFindTempl();
	virtual QString version();
	virtual TPFeatureResults extract( QImage & img, TFeatParams params );
};

#endif