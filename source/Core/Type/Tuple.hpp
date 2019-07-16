#pragma once
#include <Core/Root.hpp>

namespace Core
{
#pragma pack(1)
    /**/
    template<typename...> struct Tuple;

    /**/
    template<typename A>
    struct Tuple<A>
    {
        /**/
        Tuple() = default;
        Tuple( const A& a_ ):
            a(a_)
        {}

        // common value names for 1st field
        union
        {
            A a;
            A x;
            A width;
        };
    };

    /**/
    template<typename A, typename B>
    struct Tuple<A, B>:
        public Tuple<A>
    {
        /**/
        Tuple() = default;
        Tuple( const A& a_, const B& b_ ):
            Tuple<A>(a_),
            b(b_)
        {}

        using Tuple<A>::a;
        // common value names for 2nd field
        union
        {
            B b;
            B y;
            B height;
        };
    };

    /**/
    template<typename A, typename B, typename C>
    struct Tuple<A, B, C>:
        public Tuple<A, B>
    {
        /**/
        Tuple() = default;
        Tuple( const A& a_, const B& b_, const C& c_ ):
            Tuple<A, B>(a_, b_),
            c(c_)
        {}

        // common value names for 3rd field
        union
        {
            C c;
            C z;
            C depth;
        };
    };

    /**/
    template<typename A, typename B, typename C, typename D>
    struct Tuple<A, B, C, D>:
        public Tuple<A, B, C>
    {
        /**/
        Tuple() = default;
        Tuple( const A& a_, const B& b_, const C& c_, const D& d_ ):
            Tuple<A, B, C>(a_, b_, c_),
            d(d_)
        {}

        // common value names for 4th field
        union
        {
            D d;
            D w;
        };
    };

    /**/
    template<typename A, typename B, typename C, typename D, typename E>
    struct Tuple<A, B, C, D, E>:
        public Tuple<A, B, C, D>
    {
        /**/
        Tuple() = default;
        Tuple( const A& a_, const B& b_, const C& c_, const D& d_, const E& e_ ):
            Tuple<A, B, C, D>(a_, b_, c_, d_),
            e(e_)
        {}

        // common value names for 5th field
        union
        {
            E e;
        };
    };
#pragma pack()
}
