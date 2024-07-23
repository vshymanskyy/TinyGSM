#ifndef TinyGsmFifo_h
#define TinyGsmFifo_h

template <class T, unsigned N>
class TinyGsmFifo {
 public:
  /**
   * @brief Construct a new Tiny Gsm Fifo object, setting the head and tail to
   * 0.
   */
  TinyGsmFifo() {
    clear();
  }

  /**
   * @brief Clear the FIFO - set the read and write positions to 0
   */
  void clear() {
    _r = 0;
    _w = 0;
  }

  // writing thread/context API
  //-------------------------------------------------------------

  /**
   * @brief Check if the buffer is writable - that is if it has any space left
   *
   * @return *true* The buffer has free space.
   * @return *false* There is no space left in the buffer.
   */
  bool writeable(void) {
    return free() > 0;
  }

  /**
   * @brief Check the number of free positions in the buffer.
   *
   * @return *int*  The number number of free positions in the buffer
   */
  int free(void) {
    int s = _r - _w;     // Check if the read is ahead of the write
    if (s <= 0) s += N;  // if not wrap
    return s - 1;  // return the difference between r and w, accounting for wrap
  }

  /**
   * @brief Add a single item to the buffer. This is non-blocking.
   *
   * @param c Reference of the item of type 'T' to add to the buffer
   * @return *true* The item was successfully added to the buffer
   * @return *false* Nothing was added to the buffer
   */
  bool put(const T& c) {
    int i = _w;       // check the write position
    int j = i;        // set the spot for the new item to the write position
    i     = _inc(i);  // check where the next increment of the write will be
    if (i == _r)  // make sure the next spot isn't the position of the read (ie,
                  // the buffer is full)
      return false;
    _b[j] = c;  // add the item at position j
    _w    = i;  // bump the write position
    return true;
  }

  /**
   * @brief Add multiple items to be buffer
   *
   * @param p Pointer to the items to add
   * @param n The number of items to add
   * @param t Whether to block while waiting for space enough space to clear to
   * add all items
   * @return *int* The number of items successfully added
   */
  int put(const T* p, int n, bool t = false) {
    int c = n;
    while (c) {
      int f;
      while ((f = free()) == 0)  // wait for space
      {
        if (!t) return n - c;  // no more space and not blocking
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

  bool readable(void) {
    return (_r != _w);
  }

  size_t size(void) {
    int s = _w - _r;
    if (s < 0) s += N;
    return s;
  }

  bool get(T* p) {
    int r = _r;
    if (r == _w)  // !readable()
      return false;
    *p = _b[r];
    _r = _inc(r);
    return true;
  }

  int get(T* p, int n, bool t = false) {
    int c = n;
    while (c) {
      int f;
      for (;;)  // wait for data
      {
        f = size();
        if (f) break;          // free space
        if (!t) return n - c;  // no space and not blocking
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

  uint8_t peek() {
    return _b[_r];
  }

 private:
  /**
   * @brief Get the next increment spot in the buffer, accounting for the size
   * of each item in the buffer
   *
   * @param i
   * @param n
   * @return *int*
   */
  int _inc(int i, int n = 1) {
    return (i + n) % N;
  }

  T   _b[N];  /// The buffer, containing 'N' items of type 'T'
  int _w;     /// The write position in the buffer
  int _r;     /// The read position in the buffer
};

#endif
