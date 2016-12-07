#ifndef TinyGsmFifo_h
#define TinyGsmFifo_h

template <class T, unsigned N>
class TinyGsmFifo
{
public:
    TinyGsmFifo()
    {
        clear();
    }

    void clear()
    {
        _r = 0;
        _w = 0;
    }

    // writing thread/context API
    //-------------------------------------------------------------

    bool writeable(void)
    {
        return free() > 0;
    }

    int free(void)
    {
        int s = _r - _w;
        if (s <= 0)
            s += N;
        return s - 1;
    }

    bool put(const T& c)
    {
        int i = _w;
        int j = i;
        i = _inc(i);
        if (i == _r) // !writeable()
            return false;
        _b[j] = c;
        _w = i;
        return true;
    }

    int put(const T* p, int n, bool t = false)
    {
        int c = n;
        while (c)
        {
            int f;
            while ((f = free()) == 0) // wait for space
            {
                if (!t) return n - c; // no more space and not blocking
                /* nothing / just wait */;
            }
            // check free space
            if (c < f) f = c;
            int w = _w;
            int m = N - w;
            // check wrap
            if (f > m) f = m;
            memcpy(&_b[w], p, f);
            _w = _inc(w, f);
            c -= f;
            p += f;
        }
        return n - c;
    }

    // reading thread/context API
    // --------------------------------------------------------

    bool readable(void)
    {
        return (_r != _w);
    }

    size_t size(void)
    {
        int s = _w - _r;
        if (s < 0)
            s += N;
        return s;
    }

    bool get(T* p)
    {
        int r = _r;
        if (r == _w) // !readable()
            return false;
        *p = _b[r];
        _r = _inc(r);
        return true;
    }

    int get(T* p, int n, bool t = false)
    {
        int c = n;
        while (c)
        {
            int f;
            for (;;) // wait for data
            {
                f = size();
                if (f)  break;        // free space
                if (!t) return n - c; // no space and not blocking
                /* nothing / just wait */;
            }
            // check available data
            if (c < f) f = c;
            int r = _r;
            int m = N - r;
            // check wrap
            if (f > m) f = m;
            memcpy(p, &_b[r], f);
            _r = _inc(r, f);
            c -= f;
            p += f;
        }
        return n - c;
    }

private:
    int _inc(int i, int n = 1)
    {
        return (i + n) % N;
    }

    T    _b[N];
    int  _w;
    int  _r;
};

#endif
