#pragma once

namespace core
{

class INatvisHolder
{
protected:
    virtual ~INatvisHolder() = default;
};

template<typename T>
class NatvisHolder : public INatvisHolder
{
public:
    NatvisHolder(const T& v) : p(&v) {}

private:
    const T* p;
};

} // core