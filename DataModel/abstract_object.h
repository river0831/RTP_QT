#ifndef ABSTRACTOBJECT_H
#define ABSTRACTOBJECT_H

template<typename T, typename W>
class AbstractObject
{
public:
    T obj;
    W attributes;

    AbstractObject(T object, W values)
    {
        obj = object;
        attributes = values;
    }
};

#endif // ABSTRACTOBJECT_H
