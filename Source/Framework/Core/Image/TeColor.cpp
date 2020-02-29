#include "TeColor.h"
#include "Math/TeMath.h"
#include "Math/TeVector4.h"

namespace te
{
    const Color Color::ZERO = Color(0.0f, 0.0f, 0.0f, 0.0f);
    const Color Color::Black = Color(0.0f, 0.0f, 0.0f);
    const Color Color::White = Color(1.0f, 1.0f, 1.0f);
    const Color Color::Red = Color(1.0f, 0.0f, 0.0f);
    const Color Color::Green = Color(0.0f, 1.0f, 0.0f);
    const Color Color::Blue = Color(0.0f, 0.0f, 1.0f);
    const Color Color::LightGray = Color(200.0f / 255.0f, 200.0f / 255.0f, 200.0f / 255.0f);

    float LinearToSRGB(float x)
    {
        if (x <= 0.0f)
            return 0.0f;
        else if (x >= 1.0f)
            return 1.0f;
        else if (x < 0.0031308f)
            return x * 12.92f;
        else
            return std::pow(x, 1.0f / 2.4f) * 1.055f - 0.055f;
    }

    float SRGBToLinear(float x)
    {
        if (x <= 0.0f)
            return 0.0f;
        else if (x >= 1.0f)
            return 1.0f;
        else if (x < 0.04045f)
            return x / 12.92f;
        else
            return std::pow((x + 0.055f) / 1.055f, 2.4f);
    }

    Color Color::FromRGBA(RGBA val)
    {
        Color output;
        const UINT32 val32 = val;

        output.a = ((val32 >> 24) & 0xFF) / 255.0f;
        output.b = ((val32 >> 16) & 0xFF) / 255.0f;
        output.g = ((val32 >> 8) & 0xFF) / 255.0f;
        output.r = (val32 & 0xFF) / 255.0f;

        return output;
    }

    bool Color::operator==(const Color& rhs) const
    {
        return (r == rhs.r &&
            g == rhs.g &&
            b == rhs.b &&
            a == rhs.a);
    }

    bool Color::operator!=(const Color& rhs) const
    {
        return !(*this == rhs);
    }

    RGBA Color::GetAsRGBA() const
    {
        UINT8 val8;
        UINT32 val32 = 0;

        // Convert to 32bit pattern
        // (ABRG = 8888)

        // Alpha
        val8 = static_cast<UINT8>(a * 255);
        val32 = val8 << 24;

        // Blue
        val8 = static_cast<UINT8>(b * 255);
        val32 += val8 << 16;

        // Green
        val8 = static_cast<UINT8>(g * 255);
        val32 += val8 << 8;

        // Red
        val8 = static_cast<UINT8>(r * 255);
        val32 += val8;

        return val32;
    }

    Vector4 Color::GetAsVector4() const
    {
        return Vector4(r, g, b, a);
    }

    Color Color::GetGamma() const
    {
        return Color(
            LinearToSRGB(r),
            LinearToSRGB(g),
            LinearToSRGB(b),
            a
        );
    }

    Color Color::GetLinear() const
    {
        return Color(
            SRGBToLinear(r),
            SRGBToLinear(g),
            SRGBToLinear(b),
            a
        );
    }
}