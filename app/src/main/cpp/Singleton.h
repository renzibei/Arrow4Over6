//
// Created by Fan Qu on 5/16/20.
//

#ifndef ARROW4OVER6_SINGLETON_H
#define ARROW4OVER6_SINGLETON_H

#include <mutex>

template<typename T>
class Singleton {
protected:
    static T* _instance;
    static std::mutex singleMutex;

    virtual ~Singleton() {}

public:
    static T* getInstance() {
        if(_instance == nullptr) {
            std::lock_guard<std::mutex> lockGuard(singleMutex);
            if(_instance == nullptr)
                _instance = new T();
        }
        return _instance;
    }
};

template<typename T>
T* Singleton<T>::_instance = nullptr;

template<typename T>
std::mutex Singleton<T>::singleMutex;


#endif //ARROW4OVER6_SINGLETON_H
