#include "Animation/TeAnimationCurve.h"

#include "Math/TeVector3.h"
#include "Math/TeVector2.h"
#include "Math/TeQuaternion.h"
#include "Math/TeMath.h"
#include "Animation/TeAnimationUtility.h"

namespace te
{
    namespace impl
    {
        template <class T>
        T GetZero() { return 0.0f; }

        template<>
        float GetZero<float>() { return 0.0f; }

        template<>
        Vector3 GetZero<Vector3>() { return Vector3(); }

        template<>
        Vector2 GetZero<Vector2>() { return Vector2(); }

        template<>
        Quaternion GetZero<Quaternion>() { return Quaternion(); }

        template<>
        INT32 GetZero<INT32>() { return 0; }

        template <class T>
        void GetMinMax(std::pair<T, T>& minmax, const T& value)
        {
            minmax.first = std::min(minmax.first, value);
            minmax.second = std::max(minmax.second, value);
        }

        template <>
        void GetMinMax(std::pair<Vector3, Vector3>& minmax, const Vector3& value)
        {
            minmax.first = Vector3::Min(minmax.first, value);
            minmax.second = Vector3::Max(minmax.second, value);
        }

        template <>
        void GetMinMax(std::pair<Vector2, Vector2>& minmax, const Vector2& value)
        {
            minmax.first = Vector2::Min(minmax.first, value);
            minmax.second = Vector2::Max(minmax.second, value);
        }

        template <>
        void GetMinMax(std::pair<Quaternion, Quaternion>& minmax, const Quaternion& value)
        {
            minmax.first = Quaternion::Min(minmax.first, value);
            minmax.second = Quaternion::Max(minmax.second, value);
        }

        template<class T>
        TKeyframe<T> EvaluateKey(const TKeyframe<T>& lhs, const TKeyframe<T>& rhs, float time)
        {
            float length = rhs.TimeInSpline - lhs.TimeInSpline;

            if (Math::ApproxEquals(length, 0.0f))
                return lhs;

            // Scale from arbitrary range to [0, 1]
            float invLength = 1.0f / length;
            float t = (time - lhs.TimeInSpline) * invLength;

            TKeyframe<T> output;
            output.TimeInSpline = time;
            output.Value = Math::Lerp(t, lhs.Value, rhs.Value);

            // TODO animation : not sure if it works
            return output;
        }

        template<>
        TKeyframe<INT32> EvaluateKey(const TKeyframe<INT32>& lhs, const TKeyframe<INT32>& rhs, float time)
        {
            TKeyframe<INT32> output;
            output.TimeInSpline = time;
            output.Value = time >= rhs.TimeInSpline ? rhs.Value : lhs.Value;

            return output;
        }

        template<class T>
        T Evaluate(const TKeyframe<T>& lhs, const TKeyframe<T>& rhs, float time)
        {
            float length = rhs.TimeInSpline - lhs.TimeInSpline;
            assert(length > 0.0f);

            float t;
            if (Math::ApproxEquals(length, 0.0f))
            {
                t = 0.0f;
            }
            else
            {
                // Scale from arbitrary range to [0, 1]
                t = (time - lhs.TimeInSpline) / length;
            }

            // TODO animation : not sure if it works
            return Math::Lerp(t, lhs.Value, rhs.Value);
        }

        template<>
        INT32 Evaluate(const TKeyframe<INT32>& lhs, const TKeyframe<INT32>& rhs, float time)
        {
            return time >= rhs.TimeInSpline ? rhs.Value : lhs.Value;
        }
    }

    template <class T>
    TAnimationCurve<T>::TAnimationCurve(const Vector<KeyFrame>& keyframes)
        : _keyframes(keyframes)
    { 
        // Ensure keyframes are sorted
        if(!keyframes.empty())
        {
            float time = keyframes[0].TimeInSpline;
            for (UINT32 i = 1; i < (UINT32)keyframes.size(); i++)
            {
                assert(keyframes[i].TimeInSpline >= time);
                time = keyframes[i].TimeInSpline;
            }
        }

        if (!keyframes.empty())
            _end = keyframes.back().TimeInSpline;
        else
            _end = 0.0f;

        _start = 0.0f;
        _length = _end;
    }

    template <class T>
    T TAnimationCurve<T>::Evaluate(float time, bool loop) const
    {
        if (_keyframes.empty())
            return impl::GetZero<T>();

        AnimationUtility::WrapTime(time, _start, _end, loop);

        UINT32 leftKeyIdx;
        UINT32 rightKeyIdx;

        FindKeys(time, leftKeyIdx, rightKeyIdx);

        // Evaluate curve as hermite cubic spline
        const KeyFrame& leftKey = _keyframes[leftKeyIdx];
        const KeyFrame& rightKey = _keyframes[rightKeyIdx];

        if (leftKeyIdx == rightKeyIdx)
            return leftKey.Value;

        return impl::Evaluate(leftKey, rightKey, time);
    }

    template <class T>
    TKeyframe<T> TAnimationCurve<T>::EvaluateKey(float time, bool loop) const
    {
        if (_keyframes.empty())
            return TKeyframe<T>();

        AnimationUtility::WrapTime(time, _start, _end, loop);

        UINT32 leftKeyIdx;
        UINT32 rightKeyIdx;

        FindKeys(time, leftKeyIdx, rightKeyIdx);

        const KeyFrame& leftKey = _keyframes[leftKeyIdx];
        const KeyFrame& rightKey = _keyframes[rightKeyIdx];

        if (leftKeyIdx == rightKeyIdx)
            return leftKey;

        return impl::EvaluateKey(leftKey, rightKey, time);
    }

    template <class T>
    std::pair<float, float> TAnimationCurve<T>::GetTimeRange() const
    {
        if (_keyframes.empty())
            return std::make_pair(0.0f, 0.0f);

        if (_keyframes.size() == 1)
            return std::make_pair(_keyframes[0].TimeInSpline, _keyframes[0].TimeInSpline);

        return std::make_pair(_keyframes[0].TimeInSpline, _keyframes[_keyframes.size() - 1].TimeInSpline);
    }

    template <class T>
    std::pair<T, T> TAnimationCurve<T>::CalculateRange() const
    {
        const auto numKeys = (UINT32)_keyframes.size();
        if (numKeys == 0)
            return std::make_pair(impl::GetZero<T>(), impl::GetZero<T>());

        std::pair<T, T> output = { std::numeric_limits<T>::infinity(), -std::numeric_limits<T>::infinity() };
        impl::GetMinMax(output, _keyframes[0].Value);

        for (UINT32 i = 1; i < numKeys; i++)
            impl::GetMinMax(output, _keyframes[i].Value);

        return output;
    }

    template <class T>
    void TAnimationCurve<T>::FindKeys(float time, UINT32& leftKey, UINT32& rightKey) const
    {
        INT32 start = 0;
        auto searchLength = (INT32)_keyframes.size();

        while (searchLength > 0)
        {
            INT32 half = searchLength >> 1;
            INT32 mid = start + half;

            if (time < _keyframes[mid].TimeInSpline)
            {
                searchLength = half;
            }
            else
            {
                start = mid + 1;
                searchLength -= (half + 1);
            }
        }

        leftKey = std::max(0, start - 1);
        rightKey = std::min(start, (INT32)_keyframes.size() - 1);
    }

    template <class T>
    TKeyframe<T> TAnimationCurve<T>::EvaluateKey(const KeyFrame& lhs, const KeyFrame& rhs, float time) const
    {
        return impl::EvaluateKey(lhs, rhs, time);
    }

    template <class T>
    bool TAnimationCurve<T>::operator==(const TAnimationCurve<T>& rhs) const
    {
        if (_length != rhs._length || _start != rhs._start || _end != rhs._end)
            return false;

        return _keyframes == rhs._keyframes;
    }

    template <class T>
    UINT32 TAnimationCurve<T>::FindKey(float time)
    {
        UINT32 leftKeyIdx;
        UINT32 rightKeyIdx;

        FindKeys(time, leftKeyIdx, rightKeyIdx);

        const KeyFrame& leftKey = _keyframes[leftKeyIdx];
        const KeyFrame& rightKey = _keyframes[rightKeyIdx];

        if (Math::Abs(leftKey.TimeInSpline - time) <= Math::Abs(rightKey.TimeInSpline - time))
            return leftKeyIdx;

        return rightKeyIdx;
    }

    template class TAnimationCurve<Vector3>;
    template class TAnimationCurve<Vector2>;
    template class TAnimationCurve<Quaternion>;
    template class TAnimationCurve<float>;
}
