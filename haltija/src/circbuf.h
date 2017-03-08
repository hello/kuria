#ifndef _CIRCBUF_H_
#define _CIRCBUF_H_

#include <assert.h>

/*
   desired behavior is buf[0] is earliest sample
   buf[-1] is latest stample
   buf[-2] is the previous sample...
 */
template <class T>
class circular_buffer {
public:
   
    circular_buffer()
    :_size(0)
    , _idx(0)
    ,_fullness(0)
    ,_buf(NULL) {
    }
    
    circular_buffer(size_t size)
    :_size(size)
    , _idx(0)
    ,_fullness(0) {
        _buf = new T[_size];
    }
    
    ~circular_buffer() {
        if (_buf) {
            delete [] _buf;
        }
    }
    
    T & operator [] (const int index) {
        assert(_buf);
        int i = (index + _idx + _size) % _size;
        assert(i >= 0);
        return _buf[i];
    }
    
    void push_back(const T & item) {
        assert(_buf);
        if (++_fullness >= _size) {
            _fullness = _size;
        }
        
        _buf[_idx] = item;
        
        if (++_idx >= _size) {
            _idx = 0;
        }
    }
    
    bool is_full() const {
        return _fullness == _size;
    }
    
    bool resize(const size_t n) {
        if (_buf) {
            delete [] _buf;
        }
        
        _buf = new T[n];
        
        _size = n;
    }
    
    bool resize(const size_t n,const T & default_value) {
        if (_buf) {
            delete [] _buf;
        }
        
        _buf = new T[n];
        
        for (int i = 0; i < n; i++) {
            _buf[i] = default_value;
        }
        
        _size = n;
    }
    
private:
    size_t _size;
    int _idx;
    int _fullness;
    T * _buf;

};




#endif //_CIRCBUF_H_
