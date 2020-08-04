#include "TeTransform.h"

namespace te
{
    Transform Transform::IDENTITY;

    Transform::Transform(const Vector3& position, const Quaternion& rotation, const Vector3& scale)
        : _position(position)
        , _rotation(rotation)
        , _scale(scale)
    { }

    void Transform::SetWorldPosition(const Vector3& position, const Transform& parent)
    {
        Vector3 invScale = parent.GetScale();
        if (invScale.x != 0) invScale.x = 1.0f / invScale.x;
        if (invScale.y != 0) invScale.y = 1.0f / invScale.y;
        if (invScale.z != 0) invScale.z = 1.0f / invScale.z;

        Quaternion invRotation = parent.GetRotation().Inverse();

        _position = invRotation.Rotate(position - parent.GetPosition()) * invScale;
    }

    void Transform::SetWorldRotation(const Quaternion& rotation, const Transform& parent)
    {
        Quaternion invRotation = parent.GetRotation().Inverse();
        _rotation = invRotation * rotation;
    }

    void Transform::SetWorldScale(const Vector3& scale, const Transform& parent)
    {
        Matrix4 parentMatrix = parent.GetMatrix();
        Matrix3 rotScale = parentMatrix.Get3x3();
        rotScale = rotScale.Inverse();

        Matrix3 scaleMat = Matrix3(Quaternion::IDENTITY, scale);
        scaleMat = rotScale * scaleMat;

        Quaternion rotation;
        Vector3 localScale;
        scaleMat.Decomposition(rotation, localScale);

        _scale = localScale;
    }

    Matrix4 Transform::GetMatrix() const
    {
        return Matrix4::TRS(_position, _rotation, _scale);
    }

    Matrix4 Transform::GetInvMatrix() const
    {
        return Matrix4::InverseTRS(_position, _rotation, _scale);
    }

    void Transform::MakeLocal(const Transform& parent)
    {
        SetWorldPosition(_position, parent);
        SetWorldRotation(_rotation, parent);
        SetWorldScale(_scale, parent);
    }

    void Transform::MakeWorld(const Transform& parent)
    {
        // Update orientation
        const Quaternion& parentOrientation = parent.GetRotation();
        _rotation = parentOrientation * _rotation;

        // Update scale
        const Vector3& parentScale = parent.GetScale();

        // Scale own position by parent scale, just combine as equivalent axes, no shearing
        _scale = parentScale * _scale;

        // Change position vector based on parent's orientation & scale
        _position = parentOrientation.Rotate(parentScale * _position);

        // Add altered position vector to parents
        _position += parent.GetPosition();
    }

    void Transform::LookAt(const Vector3& location, const Vector3& up)
    {
        Vector3 forward = location - GetPosition();

        Quaternion rotation = GetRotation();
        rotation.LookRotation(forward, up);
        SetRotation(rotation);
    }

    void Transform::Move(const Vector3& vec)
    {
        SetPosition(_position + vec);
    }

    void Transform::MoveRelative(const Vector3& vec)
    {
        // Transform the axes of the relative vector by camera's local axes
        Vector3 trans = _rotation.Rotate(vec);

        SetPosition(_position + trans);
    }

    void Transform::Rotate(const Vector3& axis, const Radian& angle)
    {
        Quaternion q;
        q.FromAxisAngle(axis, angle);
        Rotate(q);
    }

    void Transform::Rotate(const Quaternion& q)
    {
        // Note the order of the mult, i.e. q comes after

        // Normalize the quat to avoid cumulative problems with precision
        Quaternion qnorm = q;
        qnorm.Normalize();
        SetRotation(qnorm * _rotation);
    }

    void Transform::Roll(const Radian& angle)
    {
        // Rotate around local Z axis
        Vector3 zAxis = _rotation.Rotate(Vector3::UNIT_Z);
        Rotate(zAxis, angle);
    }

    void Transform::Yaw(const Radian& angle)
    {
        // Rotate around local Y axis
        Vector3 yAxis = _rotation.Rotate(Vector3::UNIT_Y);
        Rotate(yAxis, angle);
    }

    void Transform::Pitch(const Radian& angle)
    {
        // Rotate around local X axis
        Vector3 xAxis = _rotation.Rotate(Vector3::UNIT_X);
        Rotate(xAxis, angle);
    }

    void Transform::SetForward(const Vector3& forwardDir)
    {
        Quaternion currentRotation = GetRotation();
        currentRotation.LookRotation(forwardDir);
        SetRotation(currentRotation);
    }
}
