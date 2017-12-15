#include <tools/confusionmatrix.h>


ConfusionMatrix::ConfusionMatrix(QObject * p):QObject(p)
{

}

ConfusionMatrix::~ConfusionMatrix(void)
{

}

void ConfusionMatrix::setNames(const QStringList &nl)
{
	int i=1;
	foreach(QString n, nl) 
	{
		_idxToName[i] = n;
		_namesIdx[n] = i++;
		
	}
}

double ConfusionMatrix::overallAccuracyPct(  )
{
	double right=0;
	double all=0;

	for( int i=1; i<=_idxToName.size(); i++)
	{
		for( int j=1; j<=_idxToName.size(); j++)
		{
			all += _matrix[i][j];
			if (i==j) 
				right += _matrix[i][j];			
		}

	}

	return (right / all) * 100;
}

bool ConfusionMatrix::addPair( const QString &expected, const QString & actual, int num )
{
	if ( !_namesIdx.contains( expected ) || !_namesIdx.contains( actual )  )
		return false;
	_matrix[_namesIdx[expected]][_namesIdx[actual]] += num;
	return true;
}

QAbstractItemModel * ConfusionMatrix::model()
{ 
	_model.setColumnCount( _namesIdx.size() + 1  );
	_model.setRowCount( _namesIdx.size() + 1  );
	for( int i=1; i<=_idxToName.size(); i++)
	{
		for( int j=1; j<=_idxToName.size(); j++)
		{
			if (_matrix[i][j])
			{
				_model.setData( _model.index( i, j ), QColor("red"), Qt::BackgroundRole);
				_model.setData( _model.index( i, j ), _matrix[i][j], Qt::DisplayRole );
			}
			
		}

	}

	for( int i=1; i<=_idxToName.size(); i++)
	{
		_model.setData(_model.index( 0, i ), QColor("grey"),Qt::BackgroundRole);
		_model.setData(_model.index( i, 0 ), QColor("grey"),Qt::BackgroundRole);
		_model.setData(_model.index( i, i ), QColor("green"),Qt::BackgroundRole);
		_model.setData(_model.index( 0, i ), i, Qt::DisplayRole );
		_model.setData(_model.index( i, 0 ), i, Qt::DisplayRole);
	}
	return &_model;
}