#ifndef ASTMPARSER_H
#define ASTMPARSER_H

#include <QtCore>

#ifdef BUILDING_CLIB_DLL
# ifndef CLIB_EXPORT
#   define CLIB_EXPORT Q_DECL_EXPORT
# endif
#else
# ifndef CLIB_EXPORT
#   define CLIB_EXPORT Q_DECL_IMPORT
# endif
#endif

class ASTM_Record;
typedef QSharedPointer<ASTM_Record> PASTM_Record;


class CLIB_EXPORT WrongTypeException :public std::exception{};
class CLIB_EXPORT ASTM_Record
{
protected:
	QMap<QString,QList<PASTM_Record> > _records;

 public: 
	virtual ~ASTM_Record(){ _records.clear(); }
	virtual bool parseData(QString recordData)=0;
    virtual QString fieldName(int idx)=0;
	virtual QString fieldSK(int idx)=0;
    virtual QString fieldValue(int idx)=0;
    virtual int fieldsCount()=0;
    QList<PASTM_Record> & records(QString rt)
	{
		if ( !_records.contains( rt) )
			throw ( WrongTypeException() );
		return _records[rt];
	}

	

	virtual bool toJSON( QTextStream & out, bool ignoreEmpty=true )
	{
		out <<"{ ";
		bool addComma=false;
		for (int i=1; i<fieldsCount(); i++)
		{
			if( ignoreEmpty && fieldValue(i).isEmpty() )
				continue;
			if( addComma )
			{
				out << ", ";
			}
			addComma=true;
			out << "\"" <<fieldSK(i)<< "\":\"" << fieldValue(i).replace("\\","\\\\") << "\"";			
		}
	
		for( QMap<QString,QList<PASTM_Record> >::iterator it = _records.begin(); it != _records.end(); it++ )
		{
			if ( !it.value().isEmpty() )
			{
				out << ", \""<<it.key()<< "\":[";
				bool addComma=false;
				for( QList<PASTM_Record>::iterator vit = it.value().begin(); vit != it.value().end(); vit++ )
				{
					if( addComma )
					{
						out << ", ";
					}
					addComma=true;
					(*vit)->toJSON( out );
				}
				out << "]";
			}
		}

		out <<" }";
		return true;
	}

};



class CLIB_EXPORT ASTM_Manufacturer : public ASTM_Record
{
    QString f_type;// "Record Type ID"
    QString f_seq;// "Sequence Number"
	QStringList _fieldNames;
public:
	QString f_mf1;// "Manufacturer Field 1"
    QString f_mf2;// "Manufacturer Field 2"
    QString f_mf3;// "Manufacturer Field 3"
    QString f_mf4;// "Manufacturer Field 4"
    QString f_mf5;// "Manufacturer Field 5"

	

	ASTM_Manufacturer()
	{
		_fieldNames << "Record Type ID"
             << "Sequence Number"
             << "Manufacturer Field 1"
             << "Manufacturer Field 2"
             << "Manufacturer Field 3"
             << "Manufacturer Field 4"
             << "Manufacturer Field 5";
		_records["C"] = QList<PASTM_Record>();
	}	

    int fieldsCount()
    {
        return _fieldNames.size();
    }

    QString fieldName(int idx)
    {
        idx--;
        if (idx < 0 || idx >= _fieldNames.size())
            return QString::null;
        return _fieldNames.at(idx);
    }

    QString fieldValue(int idx)
    {
        if (idx < 1 || idx > _fieldNames.size())
            return QString::null;
        switch (idx)
        {
            case (1): return f_type;
            case (2): return f_seq;
            case (3): return f_mf1;
            case (4): return f_mf2;
            case (5): return f_mf3;
            case (6): return f_mf4;
            case (7): return f_mf5;
            default: return QString::null;
        }
    }

	QString fieldSK(int idx)
    {
        if (idx < 1 || idx > _fieldNames.size())
            return QString::null;
        switch (idx)
        {
            case (1): return "type";
            case (2): return "seq";
            case (3): return "mf1";
            case (4): return "mf2";
            case (5): return "mf3";
            case (6): return "mf4";
            case (7): return "mf5";
            default: return QString::null;
        }
    }

	        

    bool addComment(QString recordData)
    {
        //TODO Log error
        return false;
    }

    bool addManufacturerInfo(QString recordData)
    {
        //TODO Log error
        return false;
    }

    bool parseData(QString recordData)
    {
        QStringList recordValues = recordData.split( "|" );
        int idx = 1;
        foreach (QString v,  recordValues)
        {
            switch (idx)
            {
                case (1): f_type = v; break;
                case (2): f_seq = v; break;
                case (3): f_mf1 = v; break;
                case (4): f_mf2 = v; break;
                case (5): f_mf3 = v; break;
                case (6): f_mf4 = v; break;
                case (7): f_mf5 = v; break;
                default: return false;
            }
            idx++;
        }
        return true;
    }
};

class CLIB_EXPORT ASTM_Comment : public ASTM_Record
{
    QString f_type;// "Record Type ID"
    QString f_seq;// "Sequence Number"
	QStringList _fieldNames;
public:
	QString f_source;// "Comment Source"
	QString f_data;// "Comment Text"
	QString f_ctype;// "Comment Type"

	ASTM_Comment()
	{
		_fieldNames << "Record Type ID"
             << "Sequence Number"
             << "Comment Source"
             << "Comment Text"
             << "Comment Type";
	}

    int fieldsCount()
    {
        return _fieldNames.size();
    }

    QString fieldName(int idx)
    {
        idx--;
        if (idx < 0 || idx >= _fieldNames.size())
            return QString::null;
        return _fieldNames.at(idx);
    }

    QString fieldValue(int idx)
    {
        if (idx < 1 || idx > _fieldNames.size())
            return QString::null;
        switch (idx)
        {
            case (1): return f_type;
            case (2): return f_seq;
            case (3): return f_source;
            case (4): return f_data;
            case (5): return f_ctype;
            default: return QString::null;
        }
    }

	QString fieldSK(int idx)
    {
        if (idx < 1 || idx > _fieldNames.size())
            return QString::null;
        switch (idx)
        {
            case (1): return "type";
            case (2): return "seq";
            case (3): return "source";
            case (4): return "data";
            case (5): return "ctype";
            default: return QString::null;
        }
    }



    bool addComment(QString recordData)
    {
        //TODO Log error
        return false;
    }

    bool addManufacturerInfo(QString recordData)
    {
        //TODO Log error
        return false;
    }

    bool parseData(QString recordData)
    {
        QStringList recordValues = recordData.split( "|" );
        int idx = 1;
        foreach (QString v,  recordValues)
        {
            switch (idx)
            {
                case (1): f_type = v; break;
                case (2): f_seq = v; break;
                case (3): f_source = v; break;
                case (4): f_data = v; break;
                case (5): f_ctype = v; break;
                default: return false;
            }
            idx++;
        }
        return true;
    }
};



class CLIB_EXPORT ASTM_Scientific : public ASTM_Record
{

    QString f_type;// "Record Type ID"
    QString f_seq;// "Sequence Number"
	QStringList _fieldNames;
public:
	QString f_anmeth;// "Analytical Method"
    QString f_instr;// "Instrumentation"
    QString f_reagents;// "Reagents"
    QString f_unitofmeas;// "Units of Measure"
    QString f_qc;// "Quality Control"
    QString f_spcmdescr;// "Specimen Descriptor"
    QString f_resrvd;// "Reserved Field"
    QString f_container;// "Container"
    QString f_spcmid;// "Specimen ID"
    QString f_analyte;// "Analyte"
    QString f_result;// "Result"
    QString f_resunts;// "Result Units"
    QString f_collctdt;// "Collection Date and Time"
    QString f_resdt;// "Result Date and Time"
    QString f_anlprocstp;// "Analytical Preprocessing Steps"
    QString f_patdiagn;// "Patient Diagnosis"
    QString f_patbd;// "Patient Birthdate"
    QString f_patsex;// "Patient Sex"
    QString f_patrace;// "Patient Race"

    ASTM_Scientific() {
            _fieldNames << "Record Type ID"
            << "Sequence Number"
            << "Analytical Method"
            << "Instrumentation"
            << "Reagents"
            << "Units of Measure"
            << "Quality Control"
            << "Specimen Descriptor"
            << "Reserved Field"
            << "Container"
            << "Specimen ID"
            << "Analyte"
            << "Result"
            << "Result Units"
            << "Collection Date and Time"
            << "Result Date and Time"
            << "Analytical Preprocessing Steps"
            << "Patient Diagnosis"
            << "Patient Birthdate"
            << "Patient Sex"
            << "Patient Race";

			_records["C"] = QList<PASTM_Record>();
			_records["M"] = QList<PASTM_Record>();
	}

    int fieldsCount()
    {
        return _fieldNames.size();
    }

    QString fieldName(int idx)
    {
        idx--;
        if (idx < 0 || idx >= _fieldNames.size())
            return QString::null;
        return _fieldNames.at(idx);
    }

    QString fieldValue(int idx)
    {
        if (idx < 1 || idx > _fieldNames.size())
            return QString::null;
        switch (idx)
        {
            case (1): return f_type;
            case (2): return f_seq;
            case (3): return f_anmeth;
            case (4): return f_instr;
            case (5): return f_reagents;
            case (6): return f_unitofmeas;
            case (7): return f_qc;
            case (8): return f_spcmdescr;
            case (9): return f_resrvd;
            case (10): return f_container;
            case (11): return f_spcmid;
            case (12): return f_analyte;
            case (13): return f_result;
            case (14): return f_resunts;
            case (15): return f_collctdt;
            case (16): return f_resdt;
            case (17): return f_anlprocstp;
            case (18): return f_patdiagn;
            case (19): return f_patbd;
            case (20): return f_patsex;
            case (21): return f_patrace;
            default: return QString::null;
        }
    }

	QString fieldSK(int idx)
    {
        if (idx < 1 || idx > _fieldNames.size())
            return QString::null;
        switch (idx)
        {
            case (1): return "type";
            case (2): return "seq";
            case (3): return "anmeth";
            case (4): return "instr";
            case (5): return "reagents";
            case (6): return "unitofmeas";
            case (7): return "qc";
            case (8): return "spcmdescr";
            case (9): return "resrvd";
            case (10): return "container";
            case (11): return "spcmid";
            case (12): return "analyte";
            case (13): return "result";
            case (14): return "resunts";
            case (15): return "collctdt";
            case (16): return "resdt";
            case (17): return "anlprocstp";
            case (18): return "patdiagn";
            case (19): return "patbd";
            case (20): return "patsex";
            case (21): return "patrace";
            default: return QString::null;
        }
    }

    bool parseData(QString recordData)
    {
        QStringList recordValues = recordData.split( "|" );
        int idx = 1;
        foreach (QString v,  recordValues)
        {
            switch (idx)
            {
                case (1): f_type = v; break;
                case (2): f_seq = v; break;
                case (3): f_anmeth = v; break;
                case (4): f_instr = v; break;
                case (5): f_reagents = v; break;
                case (6): f_unitofmeas = v; break;
                case (7): f_qc = v; break;
                case (8): f_spcmdescr = v; break;
                case (9): f_resrvd = v; break;
                case (10): f_container = v; break;
                case (11): f_spcmid = v; break;
                case (12): f_analyte = v; break;
                case (13): f_result = v; break;
                case (14): f_resunts = v; break;
                case (15): f_collctdt = v; break;
                case (16): f_resdt = v; break;
                case (17): f_anlprocstp = v; break;
                case (18): f_patdiagn = v; break;
                case (19): f_patbd = v; break;
                case (20): f_patsex = v; break;
                case (21): f_patrace = v; break;
                default: return false;
            }
            idx++;
        }
        return true;
    }
};

class CLIB_EXPORT ASTM_Order : public ASTM_Record
{

    QString f_type;// "Record Type ID"
    QString f_seq;// "Sequence Number"
	QStringList _fieldNames;
public:
	QString f_sample_id;// "Specimen ID"
    QString f_instrument;// "Instrument Specimen ID"
    QString f_test;// "Universal Test ID"
    QString f_priority;// "Priority"
    QString f_created_at;// "Requested/Ordered Date/Time"
    QString f_sampled_at;// "Specimen Collection Date/Time"
    QString f_collected_at;// "Collection End Time"
    QString f_volume;// "Collection Volume"
    QString f_collector;// "Collector ID"
    QString f_action_code;// "Action Code"
    QString f_danger_code;// "Danger Code"
    QString f_clinical_info;// "Relevant Information"
    QString f_delivered_at;// "Date/Time Specimen Received"
    QString f_biomaterial;// "Specimen Descriptor"
    QString f_physician;// "Ordering Physician"
    QString f_physician_phone;// "Physician's Telephone Number"
    QString f_user_field_1;// "User Field No. 1"
    QString f_user_field_2;// "User Field No. 2"
    QString f_laboratory_field_1;// "Laboratory Field No. 1"
    QString f_laboratory_field_2;// "Laboratory Field No. 2"
    QString f_modified_at;// "Date/Time Reported"
    QString f_instrument_charge;// "Instrument Charge"
    QString f_instrument_section;// "Instrument Section ID"
    QString f_report_type;// "Report Type"

	ASTM_Order()
	{
		_fieldNames << "Record Type ID"
            << "Sequence Number"
            << "Specimen ID"
            << "Instrument Specimen ID"
            << "Universal Test ID"
            << "Priority"
            << "Requested/Ordered Date/Time"
            << "Specimen Collection Date/Time"
            << "Collection End Time"
            << "Collection Volume"
            << "Collector ID"
            << "Action Code"
            << "Danger Code"
            << "Relevant Information"
            << "Date/Time Specimen Received"
            << "Specimen Descriptor"
            << "Ordering Physician"
            << "Physician's Telephone Number"
            << "User Field No. 1"
            << "User Field No. 2"
            << "Laboratory Field No. 1"
            << "Laboratory Field No. 2"
            << "Date/Time Reported"
            << "Instrument Charge"
            << "Instrument Section ID"
            << "Report Type";
			_records["C"] = QList<PASTM_Record>();
			_records["M"] = QList<PASTM_Record>();
			_records["R"] = QList<PASTM_Record>();
    }

    int fieldsCount()
    {
        return _fieldNames.size();
    }

    QString fieldName(int idx)
    {
        idx--;
        if (idx < 0 || idx >= _fieldNames.size())
            return QString::null;
        return _fieldNames.at(idx);
    }

    QString fieldValue(int idx)
    {
        if (idx < 1 || idx > _fieldNames.size())
            return QString::null;
        switch (idx)
        {
            case (1): return f_type;
            case (2): return f_seq;
            case (3): return f_sample_id;
            case (4): return f_instrument;
            case (5): return f_test;
            case (6): return f_priority;
            case (7): return f_created_at;
            case (8): return f_sampled_at;
            case (9): return f_collected_at;
            case (10): return f_volume;
            case (11): return f_collector;
            case (12): return f_action_code;
            case (13): return f_danger_code;
            case (14): return f_clinical_info;
            case (15): return f_delivered_at;
            case (16): return f_biomaterial;
            case (17): return f_physician;
            case (18): return f_physician_phone;
            case (19): return f_user_field_1;
            case (20): return f_user_field_2;
            case (21): return f_laboratory_field_1;
            case (22): return f_laboratory_field_2;
            case (23): return f_modified_at;
            case (24): return f_instrument_charge;
            case (25): return f_instrument_section;
            case (26): return f_report_type;
            default: return QString::null;
        }
    }

	QString fieldSK(int idx)
    {
        if (idx < 1 || idx > _fieldNames.size())
            return QString::null;
        switch (idx)
        {
            case (1): return "type";
            case (2): return "seq";
            case (3): return "sample_id";
            case (4): return "instrument";
            case (5): return "test";
            case (6): return "priority";
            case (7): return "created_at";
            case (8): return "sampled_at";
            case (9): return "collected_at";
            case (10): return "volume";
            case (11): return "collector";
            case (12): return "action_code";
            case (13): return "danger_code";
            case (14): return "clinical_info";
            case (15): return "delivered_at";
            case (16): return "biomaterial";
            case (17): return "physician";
            case (18): return "physician_phone";
            case (19): return "user_field_1";
            case (20): return "user_field_2";
            case (21): return "laboratory_field_1";
            case (22): return "laboratory_field_2";
            case (23): return "modified_at";
            case (24): return "instrument_charge";
            case (25): return "instrument_section";
            case (26): return "report_type";
            default: return QString::null;
        }
    }

    bool parseData(QString recordData)
    {
        QStringList recordValues = recordData.split( "|" );
        int idx = 1;
        foreach (QString v,  recordValues)
        {
            switch (idx)
            {
                case (1): f_type = v; break;
                case (2): f_seq = v; break;
                case (3): f_sample_id = v; break;
                case (4): f_instrument = v; break;
                case (5): f_test = v; break;
                case (6): f_priority = v; break;
                case (7): f_created_at = v; break;
                case (8): f_sampled_at = v; break;
                case (9): f_collected_at = v; break;
                case (10): f_volume = v; break;
                case (11): f_collector = v; break;
                case (12): f_action_code = v; break;
                case (13): f_danger_code = v; break;
                case (14): f_clinical_info = v; break;
                case (15): f_delivered_at = v; break;
                case (16): f_biomaterial = v; break;
                case (17): f_physician = v; break;
                case (18): f_physician_phone = v; break;
                case (19): f_user_field_1 = v; break;
                case (20): f_user_field_2 = v; break;
                case (21): f_laboratory_field_1 = v; break;
                case (22): f_laboratory_field_2 = v; break;
                case (23): f_modified_at = v; break;
                case (24): f_instrument_charge = v; break;
                case (25): f_instrument_section = v; break;
                case (26): f_report_type = v; break;
                default: return false;
            }
            idx++;
        }
        return true;
    }
};

class CLIB_EXPORT ASTM_Result : public ASTM_Record
{
    QString f_type;// "Record Type ID"
    QString f_seq;// "Sequence Number"
	QStringList _fieldNames;
public:
	QString f_test;// "Universal Test ID"
    QString f_value;// "Data or Measurement Value"
    QString f_units;// "Units"
    QString f_references;// "Reference Ranges"
    QString f_abnormal_flag;// "Result Abnormal Flags"
    QString f_abnormality_nature;// "Nature of Abnormal Testing"
    QString f_status;// "Results Status"
    QString f_norms_changed_at;// "Date of Changein Instrument"
    QString f_operator;// "Operator Identification"
    QString f_started_at;// "Date/Time Test Started"
    QString f_completed_at;// "Date/Time Test Complete"
    QString f_instrument;// "Instrument Identification"

	ASTM_Result()
	{
		_fieldNames << "Record Type ID"
            << "Sequence Number"
            << "Universal Test ID"
            << "Data or Measurement Value"
            << "Units"
            << "Reference Ranges"
            << "Result Abnormal Flags"
            << "Nature of Abnormal Testing"
            << "Results Status"
            << "Date of Changein Instrument"
            << "Operator Identification"
            << "Date/Time Test Started"
            << "Date/Time Test Complete"
            << "Instrument Identification";
			_records["C"] = QList<PASTM_Record>();
			_records["M"] = QList<PASTM_Record>();
    }

    int fieldsCount()
    {
        return _fieldNames.size();
    }

    QString fieldName(int idx)
    {
        idx--;
        if (idx < 0 || idx >= _fieldNames.size())
            return QString::null;
        return _fieldNames.at(idx);
    }

    QString fieldValue(int idx)
    {
        if (idx < 1 || idx > _fieldNames.size())
            return QString::null;
        switch (idx)
        {
            case (1): return f_type;
            case (2): return f_seq;
            case (3): return f_test;
            case (4): return f_value;
            case (5): return f_units;
            case (6): return f_references;
            case (7): return f_abnormal_flag;
            case (8): return f_abnormality_nature;
            case (9): return f_status;
            case (10): return f_norms_changed_at;
            case (11): return f_operator;
            case (12): return f_started_at;
            case (13): return f_completed_at;
            case (14): return f_instrument;
            default: return QString::null;
        }
    }

	QString fieldSK(int idx)
    {
        if (idx < 1 || idx > _fieldNames.size())
            return QString::null;
        switch (idx)
        {
            case (1): return "type";
            case (2): return "seq";
            case (3): return "test";
            case (4): return "value";
            case (5): return "units";
            case (6): return "references";
            case (7): return "abnormal_flag";
            case (8): return "abnormality_nature";
            case (9): return "status";
            case (10): return "norms_changed_at";
            case (11): return "operator";
            case (12): return "started_at";
            case (13): return "completed_at";
            case (14): return "instrument";
            default: return QString::null;
        }
    }

    bool parseData(QString recordData)
    {
        QStringList recordValues = recordData.split( "|" );
        int idx = 1;
        foreach (QString v,  recordValues)
        {
            switch (idx)
            {
                case (1): f_type = v; break;
                case (2): f_seq = v; break;
                case (3): f_test = v; break;
                case (4): f_value = v; break;
                case (5): f_units = v; break;
                case (6): f_references = v; break;
                case (7): f_abnormal_flag = v; break;
                case (8): f_abnormality_nature = v; break;
                case (9): f_status = v; break;
                case (10): f_norms_changed_at = v; break;
                case (11): f_operator = v; break;
                case (12): f_started_at = v; break;
                case (13): f_completed_at = v; break;
                case (14): f_instrument = v; break;
                default: return false;
            }
            idx++;
        }
        return true;
    }
};

class CLIB_EXPORT ASTM_Patient : public ASTM_Record
{
    QString f_type;// "Record Type ID"
    QString f_seq;// "Sequence Number"
	QStringList _fieldNames;
public:
	QString f_practice_id;// "Practice Assigned Patient ID"
    QString f_laboratory_id;// "Laboratory Assigned Patient ID"
    QString f_id;// "Patient ID"
    QString f_name;// "Patient Name"
    QString f_maiden_name;// "Mother?s Maiden Name"
    QString f_birthdate;// "Birthdate"
    QString f_sex;// "Patient Sex"
    QString f_race;// "Patient Race-, Ethnic Origin"
    QString f_address;// "Patient Address"
    QString f_reserved;// "Reserved Field"
    QString f_phone;// "Patient Telephone Number"
    QString f_physician_id;// "Attending Physician ID"
    QString f_special_1;// "Special Field No. 1"
    QString f_special_2;// "Special Field No. 2"
    QString f_height;// "Patient Height"
    QString f_weight;// "Patient Weight"
    QString f_diagnosis;// "Patient's Known Diagnosis"
    QString f_medication;// "Patient?s Active Medication"
    QString f_diet;// "Patient's Diet"
    QString f_practice_field_1;// "Practice Field No. 1"
    QString f_practice_field_2;// "Practice Field No. 2"
    QString f_admission_date;// "Admission/Discharge Dates"
    QString f_admission_status;// "Admission Status"
    QString f_location;// "Location"


	ASTM_Patient()
	{
		_fieldNames << "Record Type ID"
            << "Sequence Number"
            << "Practice Assigned Patient ID"
            << "Laboratory Assigned Patient ID"
            << "Patient ID"
            << "Patient Name"
            << "Mother?s Maiden Name"
            << "Birthdate"
            << "Patient Sex"
            << "Patient Race-, Ethnic Origin"
            << "Patient Address"
            << "Reserved Field"
            << "Patient Telephone Number"
            << "Attending Physician ID"
            << "Special Field No. 1"
            << "Special Field No. 2"
            << "Patient Height"
            << "Patient Weight"
            << "Patient's Known Diagnosis"
            << "Patient?s Active Medication"
            << "Patient's Diet"
            << "Practice Field No. 1"
            << "Practice Field No. 2"
            << "Admission/Discharge Dates"
            << "Admission Status"
            << "Location";
			_records["C"] = QList<PASTM_Record>();
			_records["M"] = QList<PASTM_Record>();
			_records["O"] = QList<PASTM_Record>();
    }

    int fieldsCount()
    {
        return _fieldNames.size();
    }

    QString fieldName(int idx)
    {
        idx--;
        if (idx < 0 || idx >= _fieldNames.size())
            return QString::null;
        return _fieldNames.at(idx);
    }

    QString fieldValue(int idx)
    {
        if (idx < 1 || idx > _fieldNames.size())
            return QString::null;
        switch (idx)
        {
            case (1): return f_type;
            case (2): return f_seq;
            case (3): return f_practice_id;
            case (4): return f_laboratory_id;
            case (5): return f_id;
            case (6): return f_name;
            case (7): return f_maiden_name;
            case (8): return f_birthdate;
            case (9): return f_sex;
            case (10): return f_race;
            case (11): return f_address;
            case (12): return f_reserved;
            case (13): return f_phone;
            case (14): return f_physician_id;
            case (15): return f_special_1;
            case (16): return f_special_2;
            case (17): return f_height;
            case (18): return f_weight;
            case (19): return f_diagnosis;
            case (20): return f_medication;
            case (21): return f_diet;
            case (22): return f_practice_field_1;
            case (23): return f_practice_field_2;
            case (24): return f_admission_date;
            case (25): return f_admission_status;
            case (26): return f_location;
            default: return QString::null;
        }
    }

	QString fieldSK(int idx)
    {
        if (idx < 1 || idx > _fieldNames.size())
            return QString::null;
        switch (idx)
        {
            case (1): return "type";
            case (2): return "seq";
            case (3): return "practice_id";
            case (4): return "laboratory_id";
            case (5): return "id";
            case (6): return "name";
            case (7): return "maiden_name";
            case (8): return "birthdate";
            case (9): return "sex";
            case (10): return "race";
            case (11): return "address";
            case (12): return "reserved";
            case (13): return "phone";
            case (14): return "physician_id";
            case (15): return "special_1";
            case (16): return "special_2";
            case (17): return "height";
            case (18): return "weight";
            case (19): return "diagnosis";
            case (20): return "medication";
            case (21): return "diet";
            case (22): return "practice_field_1";
            case (23): return "practice_field_2";
            case (24): return "admission_date";
            case (25): return "admission_status";
            case (26): return "location";
            default: return QString::null;
        }

    }
 

    bool parseData(QString recordData)
    {
        QStringList recordValues = recordData.split( "|" );
        int idx = 1;
        foreach (QString v,  recordValues)
        {
            switch (idx)
            {
                case (1): f_type = v; break;
                case (2): f_seq = v; break;
                case (3): f_practice_id = v; break;
                case (4): f_laboratory_id = v; break;
                case (5): f_id = v; break;
                case (6): f_name = v; break;
                case (7): f_maiden_name = v; break;
                case (8): f_birthdate = v; break;
                case (9): f_sex = v; break;
                case (10): f_race = v; break;
                case (11): f_address = v; break;
                case (12): f_reserved = v; break;
                case (13): f_phone = v; break;
                case (14): f_physician_id = v; break;
                case (15): f_special_1 = v; break;
                case (16): f_special_2 = v; break;
                case (17): f_height = v; break;
                case (18): f_weight = v; break;
                case (19): f_diagnosis = v; break;
                case (20): f_medication = v; break;
                case (21): f_diet = v; break;
                case (22): f_practice_field_1 = v; break;
                case (23): f_practice_field_2 = v; break;
                case (24): f_admission_date = v; break;
                case (25): f_admission_status = v; break;
                case (26): f_location = v; break;
                default: return false;
            }
            idx++;
        }
        return true;
    }
};

class CLIB_EXPORT ASTM_Request : public ASTM_Record
{
    QString f_type;// "Record Type ID"
    QString f_seq;// "Sequence Number"
	QStringList _fieldNames;
public:
	QString f_srangeid;// "Starting Range ID Number"
    QString f_erangeid;// "Ending Range ID Number"
    QString f_utestid;// "Universal Test ID"
    QString f_noreqtmlim;// "Nature of Request Time Limits"
    QString f_begreqresdt;// "Beginning Request Results Date and Time"
    QString f_endreqresdt;// "Ending Request Results Date and Time"
    QString f_reqphysname;// "Requesting Physician Name"
    QString f_reqphystel;// "Requesting Physician Telephone Number"
    QString f_userfld1;// "User Field No. 1"
    QString f_userfld2;// "User Field No. 2"
    QString f_statcodes;// "Request Information Status Codes"

	ASTM_Request()
	{
		_fieldNames << "Record Type ID"
            << "Sequence Number"
            << "Starting Range ID Number"
            << "Ending Range ID Number"
            << "Universal Test ID"
            << "Nature of Request Time Limits"
            << "Beginning Request Results Date and Time"
            << "Ending Request Results Date and Time"
            << "Requesting Physician Name"
            << "Requesting Physician Telephone Number"
            << "User Field No. 1"
            << "User Field No. 2"
            << "Request Information Status Codes";
			_records["C"] = QList<PASTM_Record>();
			_records["M"] = QList<PASTM_Record>();
    }

    int fieldsCount()
    {
        return _fieldNames.size();
    }

    QString fieldName(int idx)
    {
        idx--;
        if (idx < 0 || idx >= _fieldNames.size())
            return QString::null;
        return _fieldNames.at(idx);
    }

    QString fieldValue(int idx)
    {
        if (idx < 1 || idx > _fieldNames.size())
            return QString::null;
        switch (idx)
        {
            case (1): return f_type;
            case (2): return f_seq;
            case (3): return f_srangeid;
            case (4): return f_erangeid;
            case (5): return f_utestid;
            case (6): return f_noreqtmlim;
            case (7): return f_begreqresdt;
            case (8): return f_endreqresdt;
            case (9): return f_reqphysname;
            case (10): return f_reqphystel;
            case (11): return f_userfld1;
            case (12): return f_userfld2;
            case (13): return f_statcodes;
            default: return QString::null;
        }
    }

	QString fieldSK(int idx)
    {
        if (idx < 1 || idx > _fieldNames.size())
            return QString::null;
        switch (idx)
        {
            case (1): return "type";
            case (2): return "seq";
            case (3): return "srangeid";
            case (4): return "erangeid";
            case (5): return "utestid";
            case (6): return "noreqtmlim";
            case (7): return "begreqresdt";
            case (8): return "endreqresdt";
            case (9): return "reqphysname";
            case (10): return "reqphystel";
            case (11): return "userfld1";
            case (12): return "userfld2";
            case (13): return "statcodes";
            default: return QString::null;
        }
    }


    bool parseData(QString recordData)
    {
        QStringList recordValues = recordData.split( "|" );
        int idx = 1;
        foreach (QString v,  recordValues)
        {
            switch (idx)
            {
                case (1): f_type = v; break;
                case (2): f_seq = v; break;
                case (3): f_srangeid = v; break;
                case (4): f_erangeid = v; break;
                case (5): f_utestid = v; break;
                case (6): f_noreqtmlim = v; break;
                case (7): f_begreqresdt = v; break;
                case (8): f_endreqresdt = v; break;
                case (9): f_reqphysname = v; break;
                case (10): f_reqphystel = v; break;
                case (11): f_userfld1 = v; break;
                case (12): f_userfld2 = v; break;
                case (13): f_statcodes = v; break;
                default: return false;
            }
            idx++;
        }
        return true;
    }
};


class CLIB_EXPORT ASTM_Message : public ASTM_Record
{
    QString f_type;// "Record Type ID"
    QString f_delimeter;// "Delimiter Definition"
	QStringList _fieldNames;
public:
	QString f_message_id;// "Message Control ID"
    QString f_password;// "Access Password"
    QString f_sender;// "Sender Name or ID"
    QString f_address;// "Sender Street Address"
    QString f_reserved;// "Reserved Field"
    QString f_phone;// "Sender Telephone Number"
    QString f_caps;// "Characteristics of Sender"
    QString f_receiver;// "Receiver ID"
    QString f_comments;// "Comments"
    QString f_processing_id;// "Processing ID"
    QString f_version;// "Version Number"
    QString f_timestamp;// "Date/Timeof Message"

	ASTM_Message()
	{
		_fieldNames << "Record Type ID"
            << "Delimiter Definition"
            << "Message Control ID"
            << "Access Password"
            << "Sender Name or ID"
            << "Sender Street Address"
            << "Reserved Field"
            << "Sender Telephone Number"
            << "Characteristics of Sender"
            << "Receiver ID"
            << "Comments"
            << "Processing ID"
            << "Version Number"
            << "Date/Timeof Message";
			_records["C"] = QList<PASTM_Record>();
			_records["M"] = QList<PASTM_Record>();
			_records["S"] = QList<PASTM_Record>();
			_records["P"] = QList<PASTM_Record>();
			_records["Q"] = QList<PASTM_Record>();

    }

    int fieldsCount()
    {
        return _fieldNames.size();
    }

    QString fieldName(int idx)
    {
        idx--;
        if (idx < 0 || idx >= _fieldNames.size())
            return QString::null;
        return _fieldNames.at(idx);
    }

    QString fieldValue(int idx)
    {
        if (idx < 1 || idx > _fieldNames.size())
            return QString::null;
        switch (idx)
        {
            case (1): return f_type;
            case (2): return f_delimeter;
            case (3): return f_message_id;
            case (4): return f_password;
            case (5): return f_sender;
            case (6): return f_address;
            case (7): return f_reserved;
            case (8): return f_phone;
            case (9): return f_caps;
            case (10): return f_receiver;
            case (11): return f_comments;
            case (12): return f_processing_id;
            case (13): return f_version;
            case (14): return f_timestamp;
            default: return QString::null;
        }
    }

	QString fieldSK(int idx)
    {
        if (idx < 1 || idx > _fieldNames.size())
            return QString::null;
        switch (idx)
        {
            case (1): return "type";
            case (2): return "delimeter";
            case (3): return "message_id";
            case (4): return "password";
            case (5): return "sender";
            case (6): return "address";
            case (7): return "reserved";
            case (8): return "phone";
            case (9): return "caps";
            case (10): return "receiver";
            case (11): return "comments";
            case (12): return "processing_id";
            case (13): return "version";
            case (14): return "timestamp";
            default: return QString::null;
        }
    }

    bool parseData(QString recordData)
    {
        QStringList recordValues = recordData.split( "|" );
        int idx=1;
        foreach (QString v,  recordValues)
        {
            switch(idx)
            {
                case (1):f_type=v; break;
                case (2):f_delimeter=v; break;
                case (3):f_message_id=v; break;
                case (4):f_password=v; break;
                case (5):f_sender=v; break;
                case (6):f_address=v; break;
                case (7):f_reserved=v; break;
                case (8):f_phone=v; break;
                case (9):f_caps=v; break;
                case (10):f_receiver=v; break;
                case (11):f_comments=v; break;
                case (12):f_processing_id=v; break;
                case (13):f_version=v; break;
                case (14):f_timestamp=v; break;
                default: return false;
            }
            idx++;
        }
        return true;
    }
};

class CLIB_EXPORT ASTMParser
{
public:
	QList<PASTM_Record> _messages;


	void clear()
	{
		_messages.clear();
	}

	QList<ASTM_Message*> parse(QString astmData)
	{
		QStringList lines = astmData.split(QRegExp("\r\n|\r|\n"));
		ASTM_Message * m = 0;
		QList<ASTM_Message*> messages;
		ASTM_Record * curRec =0 ;
		foreach (QString l , lines)
		{
			QStringList entries = l.split("|");
			switch (entries.at(0).toStdString().c_str()[0])
			{
				case 'H':
					m= new ASTM_Message();
					_messages.append(PASTM_Record(m));
					curRec = m;
					curRec->parseData(l);
					break;
				case 'P':
					if (m)
					{
						ASTM_Patient *p = new ASTM_Patient();
    					m->records("P").append(PASTM_Record(p));
						curRec = p;
						curRec->parseData(l);
					}
					else
					{
						//TODO Error
					}
					break;
				case 'O':
					if (m)
					{
						ASTM_Order *o =  new ASTM_Order();
						m->records("P").last()->records("O").append(PASTM_Record(o));
						curRec = o;
						curRec->parseData(l);
					}
					else
					{
						//TODO Error
					}
					break;
				case 'R':
					if (m)
					{
						ASTM_Order *r =  new ASTM_Order();
						m->records("P").last()->records("O").last()->records("R").append( PASTM_Record(r) );
						curRec = r;
						curRec->parseData(l);
					}
					else
					{
						//TODO Error
					}
					break;
				case 'Q':
					if (m)
					{
						curRec =  new ASTM_Order();
						m->records("Q").append( PASTM_Record(curRec ) );
						curRec->parseData(l);
					}
					else
					{
						//TODO Error
					}
					break;
				case 'C':
					if (curRec )
					{
						curRec =  new ASTM_Comment();
						curRec->records("C").append( PASTM_Record(curRec ) );
						curRec->parseData(l);
					}
					else
					{
						//TODO Error
					}
					break;
				case 'M':
					if (curRec)
					{
						curRec =  new ASTM_Comment();
						curRec->records("M").append( PASTM_Record(curRec ) );
						curRec->parseData(l);
					}
					else
					{
						//TODO Error
					}
					break;
				case 'L':
					if (m)
					{
						curRec=0;
						messages.append(m);
						m = 0;
					}
					else
					{
						//TODO Error
					}
					break;

				default:
					//TODO Warning unknown record, store , message
					break;
			}
		}
		return messages;		
	}
};

#endif // ASTMPARSER_H
