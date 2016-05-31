#ifndef SINGLETON_H
#define SINGLETON_H

template<typename T> class singleton{
public:
    static T& instance(){
        if(!_singleton)
            init();
        return &_singleton;
    }

    static bool init(){
        if(_singleton)
            return false;
        _singleton = new T();
        _singleton->init();
        return true;
    }
    static void clean(){
        if(!_singleton)
            return;
        delete _singleton;
        _singleton = 0;
    }

    static T* ptr(){
        return _singleton;
    }

private:
    static T* _singleton;
};

template<typename T>
T* singleton<T>::_singleton = 0;

#endif // SINGLETON_H
