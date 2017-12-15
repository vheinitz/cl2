#ifndef _CONFUSION_MATR_HG_
#define _CONFUSION_MATR_HG_

#include <pddefines.h>
#include <QObject>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QStandardItemModel>
#include <QAbstractItemModel>
#include <QMap>

class CORELIB_EXPORT ConfusionMatrix : public QObject
{
	Q_OBJECT


	QMap<QString, int> _namesIdx;
	QMap<int, QString> _idxToName;
	QMap< int, QMap<int, int> > _matrix;
	QStandardItemModel _model;
	
public:
	ConfusionMatrix(QObject * p=0);
	virtual ~ConfusionMatrix(void);

	const QMap<int, QString> & names() const { return _idxToName;}
	void setNames(const QStringList &n);
	bool addPair( const QString &expected, const QString & actual, int num=1 );
	QAbstractItemModel * model() ;
	void clear()
	{
		_namesIdx.clear();
		_idxToName.clear();
		_matrix.clear();
		_model.clear();
	}
	double overallAccuracyPct(  );


};

#endif // _CONFUSION_MATR_HG_