class EventArgs {};

typedef void(*EventFunc)(void*, EventArgs*);

template<typename F>
class Event;

template<class R>
class Event<R(*)(void*, EventArgs*)>
{
	typedef R(*FuncType)(void*, EventArgs*);
	FuncType ls;
public:
	Event<FuncType>& operator+=(FuncType t)
	{
		ls = t;
		return *this;
	}

	Event<FuncType>& operator-=(FuncType t)
	{
		ls = NULL;
		return *this;
	}

	void operator()(void* sender, EventArgs* b)
	{
		if (NULL != ls)
			(*ls)(sender, b);
	}
};

