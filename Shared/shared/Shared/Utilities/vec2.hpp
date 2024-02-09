#pragma once
#include <iostream> 
#include <stdexcept> 
#include <cmath>

namespace Utilities
{

#pragma region SIGNED_32BIT_INT_VEC2
    typedef union _ivec2
    {
        struct 
        {
            int32_t x;
            int32_t y;
        };  int32_t contents[2];


        inline _ivec2(const _ivec2& other) : x(other.x), y(other.y) 
        {
        }

        inline _ivec2() 
        {
            x = 0;
            y = 0;
        }

        inline _ivec2(const int32_t& _x, const int32_t& _y) 
        {
            x = _x;
            y = _y;
        }

        inline _ivec2(const int32_t& v) 
        {
            x = v;
            y = v;
        }

        // Copy assignment operator
        inline _ivec2& operator=(const _ivec2& _other) 
        {
            x = _other.x;
            y = _other.y;
            return *this;
        }

        inline _ivec2 operator+(const _ivec2& _other)
        {
            x += _other.x;
            y += _other.y;
            return *this;
        }

        inline _ivec2 operator-(const _ivec2& _other)
        {
            x -= _other.x;
            y -= _other.y;
            return *this;
        }

        inline _ivec2 operator*(const _ivec2& _other)
        {
            x *= _other.x;
            y *= _other.y;
            return *this;
        }

        inline _ivec2 operator/(const _ivec2& _other) //TODO: Possibly broken, needs unit test
        {
            try 
            {
                x /= _other.x;
                y /= _other.y;
            }
            // catch block catches exception if any 
            // of type Exception 
            catch (std::invalid_argument& e) 
            {
                // prints that exception has occurred 
                // calls the what function using object of 
                // the user defined class called Exception 
                std::cout << "Exception occurred" << std::endl << e.what();
            }


            return *this;
        }

        inline bool operator==(const _ivec2& a) const
        {
            return (x == a.x) && (y == a.y);
        }

        inline bool operator!=(const _ivec2& _other) const
        {
            return !(*this == _other);
        }

        inline bool operator<(const _ivec2& _other) const
        {
            return (x < _other.x) || (x == _other.x && y < _other.y);
        }

    } ivec2;

#pragma endregion

#pragma region FLOATPOINT_VEC2
    typedef union _vec2
    {
        struct
        {
            float x;
            float y;
        };  float contents[2];

        inline _vec2() 
        {
            x = 0;
            y = 0;
        }

        inline _vec2(const float& _x, const float& _y)
        {
            x = _x;
            y = _y;
        }

        inline _vec2(const float& v)
        {
            x = v;
            y = v;
        }

        inline _vec2 operator=(const _vec2& a)
        {
            x = a.x;
            y = a.y;
            return *this;
        }

        inline _vec2 operator+(const _vec2& a)
        {
            x += a.x;
            y += a.y;
            return *this;
        }

        inline _vec2 operator-(const _vec2& a)
        {
            x -= a.x;
            y -= a.y;
            return *this;
        }

        inline _vec2 operator*(const _vec2& a)
        {
            x *= a.x;
            y *= a.y;
            return *this;
        }

        inline _vec2 operator/(const _vec2& a) //TODO: Possibly broken, needs unit testing.
        {
            try
            {
                x /= a.x;
                y /= a.y;
            }
            // catch block catches exception if any 
            // of type Exception 
            catch (std::invalid_argument& e)
            {
                // prints that exception has occurred 
                // calls the what function using object of 
                // the user defined class called Exception 
                std::cout << "Exception occurred" << std::endl << e.what();
            }


            return *this;
        }

        inline bool operator==(const _vec2& a) const
        {
            return (x == a.x) && (y == a.y);
        }

        inline bool operator!=(const _vec2& a) const
        {
            return !(*this == a);
        }

    } vec2;
}
#pragma endregion

/// <summary>
/// Hashing of the vectors.
/// </summary>
namespace std
{
    template <>
    struct hash<Utilities::_ivec2> 
    {
        std::size_t operator()(const Utilities::_ivec2& s) const 
        {
            // Combine hash values of x and y using XOR (^)
            return hash<int32_t>()(s.x) ^ (hash<int32_t>()(s.y) << 1);
        }
    };
}