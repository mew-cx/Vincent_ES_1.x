//==============================================================================
//
// Symbian TInt64 extension
//
//------------------------------------------------------------------------------
//
// 11-05-2004   Iwan Junianto       initial version
//
//==============================================================================

#ifndef __INT64X_H__
#define __INT64X_H__

#include <e32std.h>

class TInt64X: public TInt64 {
public:
    inline TInt64X();
    inline TInt64X(const TInt64& aVal);
    inline TInt64X(TInt aVal);
    inline operator TInt() const;
    inline TInt64X operator*(const TInt64X& aVal) const;
    inline TInt64X operator*(TInt aVal) const;
    inline TInt64X operator/(const TInt64X& aVal) const;
    inline TInt64X operator/(TInt aVal) const;
    inline TInt64X operator+(const TInt64X& aVal) const;
    inline TInt64X operator-(const TInt64X& aVal) const;
    inline TInt64X operator>>(TInt aShift) const;
    inline TInt64X operator<<(TInt aShift) const;
    inline TInt operator<(TInt aVal) const;
    inline TInt operator>(TInt aVal) const;
};

inline TInt64X::TInt64X(): TInt64(0,0)
{
}

inline TInt64X::TInt64X(TInt aVal): TInt64(aVal)
{
}

inline TInt64X::TInt64X(const TInt64& aVal): TInt64(aVal)
{
}

inline TInt64X::operator TInt() const
{
	return GetTInt();
}

inline TInt64X TInt64X::operator*(const TInt64X& aVal) const
{
    return TInt64X(TInt64::operator*(static_cast<const TInt64&>(aVal)));
}

inline TInt64X TInt64X::operator*(TInt aVal) const
{
    return TInt64X(TInt64::operator*(aVal));
}

inline TInt64X TInt64X::operator/(const TInt64X& aVal) const
{
    return TInt64X(TInt64::operator/(aVal));
}

inline TInt64X TInt64X::operator/(TInt aVal) const
{
    return TInt64X(TInt64::operator/(aVal));
}

inline TInt64X TInt64X::operator+(const TInt64X& aVal) const
{
    return TInt64X(TInt64::operator+(aVal));
}

inline TInt64X TInt64X::operator-(const TInt64X& aVal) const
{
    return TInt64X(TInt64::operator-(static_cast<const TInt64&>(aVal)));
}

inline TInt64X TInt64X::operator>>(TInt aShift) const
{
    return TInt64X(TInt64::operator>>(aShift));
}

inline TInt64X TInt64X::operator<<(TInt aShift) const
{
    return TInt64X(TInt64::operator<<(aShift));
}

inline TInt TInt64X::operator<(TInt aVal) const
{
    return TInt64::operator<(TInt64(aVal));
}

inline TInt TInt64X::operator>(TInt aVal) const
{
    return TInt64::operator>(TInt64(aVal));
}

#endif
