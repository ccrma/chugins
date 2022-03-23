#ifndef dbVST3HostSupport_h
#define dbVST3HostSupport_h

class dbVST3HostAttribute
{
public:
	enum Type {
		kInteger,
		kFloat,
		kString,
		kBinary
	};

	HostAttribute (int64 value)
		: _size (0)
		, _type (kInteger)
	{
		v.intValue = value;
	}

	HostAttribute (double value)
		: _size (0)
		, _type (kFloat)
	{
		v.floatValue = value;
	}

	HostAttribute (const Vst::TChar* value, uint32 size)
		: _size (size)
		, _type (kString)
	{
		v.stringValue = new Vst::TChar[_size + 1];
		memcpy (v.stringValue, value, _size * sizeof (Vst::TChar));
		v.stringValue[size] = 0;
	}

	HostAttribute (const void* value, uint32 size)
		: _size (size)
		, _type (kBinary)
	{
		v.binaryValue = new char[_size];
		memcpy (v.binaryValue, value, _size);
	}

	~HostAttribute ()
	{
		if (_size) {
			delete[] v.binaryValue;
		}
	}

	Type getType ()      const { return _type; }
	int64 intValue ()    const { return v.intValue; }
	double floatValue () const { return v.floatValue; }

	const Vst::TChar* stringValue (uint32& stringSize)
	{
		stringSize = _size;
		return v.stringValue;
	}

	const void* binaryValue (uint32& binarySize)
	{
		binarySize = _size;
		return v.binaryValue;
	}

protected:
	union v {
		int64       intValue;
		double      floatValue;
		Vst::TChar* stringValue;
		char*       binaryValue;
	} v;

	uint32 _size;
	Type   _type;

private:
	/* prevent copy construction */
	HostAttribute (HostAttribute const& other);
};

#endif