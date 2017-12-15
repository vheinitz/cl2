#include "valuecontrols.h"
#include "pddatapool.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>

ValueControl::ValueControl( QWidget *parent ): QWidget(parent)
{
}

void ValueControl::attachTo( QString pvname )
{
    _pvName=pvname;
    DataPool::instance().subscribe( _pvName, this, "value", false, this );	
}

void ValueControl::ci_stdValueSet( const QVariant & v )
{
	setConstraint("Std",v);
}

IntSliderControl::IntSliderControl( QString pvName, QString label, QWidget *parent ): ValueControl(parent), _ignoreChanges(false)
{
    _slider=new QSlider(Qt::Horizontal);
    _spinbox=new QSpinBox;
	_spinbox->setMinimum( 0 );
    _slider->setMinimum( 0 );
    _spinbox->setMaximum( 1000000 );
    _slider->setMaximum( 1000000 );
    setLayout(  new QHBoxLayout );
    layout()->setMargin(1);
    QLabel * text = new QLabel(label.isEmpty()?pvName:label);
	_standardValue = new QLabel("");
    text->setStyleSheet("color: rgb(255, 250, 255); font: 10pt;");
    layout()->addWidget ( text );
    layout()->addWidget ( _slider );
    layout()->addWidget ( _spinbox );
	layout()->addWidget ( _standardValue );
    connect( _slider, SIGNAL(valueChanged(int)), this, SLOT(processSliderChange( int)) );
    connect( _slider, SIGNAL(sliderReleased()), this, SLOT(processSliderReleased( )) );
	connect( _slider, SIGNAL(sliderPressed()), this, SLOT(processSliderPressed( )) );
    connect( _spinbox, SIGNAL(valueChanged(int)), this, SLOT(processSpinnerChange( int)) );
    attachTo( pvName );
}

void IntSliderControl::processSliderChange( int v)
{
    _spinbox->setValue( v );
}

void IntSliderControl::processSliderReleased( )
{
	DataPool::instance().set(_pvName, _spinbox->value(), this );
    _ignoreChanges = false;
}

void IntSliderControl::processSliderPressed( )
{    
	_ignoreChanges = true;
}


void IntSliderControl::processSpinnerChange( int v)
{
	_slider->setValue( v );
	if ( ! _ignoreChanges )
		DataPool::instance().set(_pvName, _spinbox->value(), this );
}

QVariant IntSliderControl::value() const
{
    return _slider->value();
}

void IntSliderControl::setValue( QVariant v )
{
	_ignoreChanges = true;
	int val = v.toInt();
	QString sval = v.toString();
    _spinbox->setValue( val );
    _slider->setValue( val );
	_ignoreChanges = false;
}

void IntSliderControl::setConstraint(QString constrName, QVariant val)
{
	_ignoreChanges=true;
    if ( constrName == "Min" )
    {
        _spinbox->setMinimum( val.toInt() );
        _slider->setMinimum( val.toInt() );
    }
    else if ( constrName == "Max" )
    {
        _spinbox->setMaximum( val.toInt() );
        _slider->setMaximum( val.toInt() );
    }
	else if ( constrName == "Std" )
    {
        _standardValue->setText( QString("(%1)").arg(val.toString()) );
    }
	_ignoreChanges=false;
}







LineEditControl::LineEditControl( QString pvName, QString label, QWidget *parent ): ValueControl(parent), _ignoreChanges(false)
{
    _value=new QLineEdit();
    setLayout(  new QHBoxLayout );
    layout()->setMargin(1);
    QLabel * text = new QLabel(label.isEmpty()?pvName:label);
	_standardValue = new QLabel("");
    text->setStyleSheet("color: rgb(255, 250, 255); font: 10pt;");
    layout()->addWidget ( text );
    layout()->addWidget ( _value );
	layout()->addWidget ( _standardValue );
	_standardValue->hide(); //TODO make configurable for all
    connect( _value, SIGNAL(textChanged(QString)), this, SLOT(processChange( QString)) );
    attachTo( pvName );
}

void LineEditControl::processChange( QString v)
{
	DataPool::instance().set(_pvName, _value->text(), this );
    _ignoreChanges = false;
}

QVariant LineEditControl::value() const
{
    return _value->text();
}

void LineEditControl::setValue( QVariant v )
{
	_ignoreChanges = true;
	_value->setText( v.toString() );
	_ignoreChanges = false;
}

void LineEditControl::setConstraint(QString constrName, QVariant val)
{
	
}




PasswordInputControl::PasswordInputControl( QString pvName, QString label, QWidget *parent ): ValueControl(parent), _ignoreChanges(false)
{
    _value=new QLineEdit();
    setLayout(  new QHBoxLayout );
    layout()->setMargin(1);
    QLabel * text = new QLabel(label.isEmpty()?pvName:label);
	_value->setEchoMode( QLineEdit::PasswordEchoOnEdit );
	_standardValue = new QLabel("");
    text->setStyleSheet("color: rgb(255, 250, 255); font: 10pt;");
    layout()->addWidget ( text );
    layout()->addWidget ( _value );
	layout()->addWidget ( _standardValue );
	_standardValue->hide(); //TODO make configurable for all
    connect( _value, SIGNAL(textChanged(QString)), this, SLOT(processChange( QString)) );
    attachTo( pvName );
}

void PasswordInputControl::processChange( QString v)
{
	DataPool::instance().set(_pvName, _value->text(), this );
    _ignoreChanges = false;
}

QVariant PasswordInputControl::value() const
{
    return _value->text();
}

void PasswordInputControl::setValue( QVariant v )
{
	_ignoreChanges = true;
	_value->setText( v.toString() );
	_ignoreChanges = false;
}

void PasswordInputControl::setConstraint(QString constrName, QVariant val)
{
	
}



CheckBoxControl::CheckBoxControl( QString pvName, QString label, QWidget *parent ): ValueControl(parent)
{
    _checkBox=new QCheckBox;
    setLayout(  new QHBoxLayout );
    layout()->setMargin(1);    
    _checkBox->setText( "" );
    QLabel * text = new QLabel(label.isEmpty()?pvName:label);
	_standardValue = new QLabel("");
    text->setStyleSheet("color: rgb(255, 250, 255); font: 10pt;");
    layout()->addWidget ( text );
    layout()->addWidget ( _checkBox );
	layout()->addWidget ( _standardValue );
    connect( _checkBox, SIGNAL(clicked(bool)), this, SLOT(processChecked( bool)) );
    attachTo( pvName );
}

void CheckBoxControl::processChecked( bool v)
{
	DataPool::instance().set(_pvName, v?1:0, this );
}

QVariant CheckBoxControl::value() const
{
    return _checkBox->isChecked();
}

void CheckBoxControl::setValue( QVariant v )
{
     _checkBox->setChecked( v.toInt() );
}

void CheckBoxControl::setConstraint(QString constrName, QVariant val)
{
    if ( constrName == "Std" )
    {
		_standardValue->setText( QString("(%1)").arg(val.toBool()?tr("set"):tr("not set")) );
    }
}

TextBoxControl::TextBoxControl( QString pvName, QString label, int minHeight, QWidget *parent ): ValueControl(parent)
{
    _textBox=new QTextEdit;
	//_textBox->setFisetMinsetMinimalHeight(minHeight);
    setLayout(  new QVBoxLayout );
    layout()->setMargin(1);    
    QLabel * text = new QLabel(label.isEmpty()?pvName:label);
    text->setStyleSheet("color: rgb(255, 250, 255); font: 10pt;");
    layout()->addWidget ( text );
    layout()->addWidget ( _textBox );
    attachTo( pvName );
	connect( _textBox, SIGNAL(textChanged ()), this, SLOT(acceptValue( )) );
}

QVariant TextBoxControl::value() const
{
	return _textBox->toPlainText();
}

void TextBoxControl::setValue( QVariant v )
{
    _textBox->setText( v.toString() );
}

void TextBoxControl::acceptValue()
{
	DataPool::instance().set(_pvName, value(), this );
}

ComboBoxControl::ComboBoxControl( QString pvName, QString label, QWidget *parent ):
ValueControl(parent),
_min(0),
_max(0)
{
    _comboBox=new QComboBox;
    setLayout(  new QHBoxLayout );
    layout()->setMargin(1);
    QLabel * text = new QLabel(label.isEmpty()?pvName:label);
	_standardValue = new QLabel("");
    text->setStyleSheet("color: rgb(255, 250, 255); font: 10pt;");
    layout()->addWidget ( text );
    layout()->addWidget ( _comboBox );
    connect( _comboBox, SIGNAL( activated(int) ), this, SLOT( processValueChange( int) ) );
    attachTo( pvName );
}

void ComboBoxControl::processValueChange( int v)
{
    DataPool::instance().set( _pvName, v, this );
}

QVariant ComboBoxControl::value() const
{
    return _comboBox->currentIndex() + _min;
}

void ComboBoxControl::setValue( QVariant v )
{
    _comboBox->setCurrentIndex( v.toInt() - _min );
}

void ComboBoxControl::setConstraint(QString constrName, QVariant val)
{
    if ( constrName == "Min" )
    {
        _min = val.toInt();
        if ( _max > _min )
        {
            int cnt = _max - _min;
            QStringList items;
            for (int i=0; i<= cnt; ++i) items << QString::number(i);
            _comboBox->clear();
            _comboBox->addItems( items );
        }       
    }
    else if ( constrName == "Max" )
    {
        _max = val.toInt();
        if ( _max > _min )
        {
            int cnt = _max - _min;
            QStringList items;
            for (int i=0; i<= cnt; ++i) items << QString::number(i);
            _comboBox->clear();
            _comboBox->addItems( items );
        }       
    }
	else if ( constrName == "Std" )
    {
        _standardValue->setText( QString("(%1)").arg(val.toString()) );
    }
}



DblSpinBoxControl::DblSpinBoxControl( double step, QString pvName, QString label, QWidget *parent ):
ValueControl(parent),
_min(0.0),
_max(0.0)
{
    _spinBox=new QDoubleSpinBox;
	_spinBox->setMinimum( step );
	_spinBox->setSingleStep(step);

	if( step < 0.00001 )
		_spinBox->setDecimals(6);
	else if( step < 0.0001 )
		_spinBox->setDecimals(5);
	else if( step < 0.001 )
		_spinBox->setDecimals(4);
	else if( step < 0.01 )
		_spinBox->setDecimals(3);
	else if( step < 0.1 )
		_spinBox->setDecimals(2);
	else if( step < 1 )
		_spinBox->setDecimals(1);

	_spinBox->setMinimumWidth(75);
    setLayout(  new QHBoxLayout );
    layout()->setMargin(1);
    QLabel * text = new QLabel(label.isEmpty()?pvName:label);
	_standardValue = new QLabel("");
    text->setStyleSheet("color: rgb(255, 250, 255); font: 10pt;");
    layout()->addWidget ( text );
    layout()->addWidget ( _spinBox );
	layout()->addItem( new QSpacerItem(10,10, QSizePolicy::Expanding)  );	
    connect( _spinBox, SIGNAL( valueChanged(double) ), this, SLOT( processValueChange( double) ) );
    attachTo( pvName );
}

void DblSpinBoxControl::processValueChange( double v)
{
    DataPool::instance().set( _pvName, v, this );
}

QVariant DblSpinBoxControl::value() const
{
	return _spinBox->value();
}

void DblSpinBoxControl::setValue( QVariant v )
{
    _spinBox->setValue(v.toDouble());
}

void DblSpinBoxControl::setConstraint(QString constrName, QVariant val)
{
    if ( constrName == "Min" )
    {
		double minval =val.toDouble();
		_spinBox->setMinimum( minval );
		if( minval < 0.00001 )
			_spinBox->setDecimals(6);
		else if( minval < 0.0001 )
			_spinBox->setDecimals(5);
		else if( minval < 0.001 )
			_spinBox->setDecimals(4);
		else if( minval < 0.01 )
			_spinBox->setDecimals(3);
		else if( minval < 0.1 )
			_spinBox->setDecimals(2);
		else if( minval < 1 )
			_spinBox->setDecimals(1);

    }
    else if ( constrName == "Max" )
    {
        _spinBox->setMaximum( val.toDouble() );       
    }
	else if ( constrName == "Std" )
    {
        _standardValue->setText( QString("(%1)").arg(val.toString()) );
    }
}