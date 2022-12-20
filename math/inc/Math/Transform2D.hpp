#pragma once

#include "AABB.hpp"

#include <glm/mat3x3.hpp>
#include <glm/vec2.hpp>

namespace Math
{
class Transform2D
{
public:
    explicit Transform2D( const glm::vec2& position = glm::vec2 { 0 }, const glm::vec2& scale = glm::vec2 { 1 }, float rotation = 0.0f );

    /// <summary>
    /// Set the transform position.
    /// </summary>
    /// <param name="pos">The new position.</param>
    void setPosition( const glm::vec2& pos )
    {
        m_Position       = pos;
        m_TransformDirty = true;
    }

    /// <summary>
    /// Get the current position of the transform.
    /// </summary>
    /// <returns>The current position of the transform.</returns>
    const glm::vec2& getPosition() const
    {
        return m_Position;
    }

    /// <summary>
    /// Set the anchor position for the transforms.
    /// </summary>
    /// <param name="anchor">The new anchor point.</param>
    void setAnchor( const glm::vec2& anchor )
    {
        m_Anchor         = anchor;
        m_TransformDirty = true;
    }

    /// <summary>
    /// Get the anchor position of the transform.
    /// </summary>
    /// <returns>The anchor position of the transform.</returns>
    const glm::vec2& getAnchor() const
    {
        return m_Anchor;
    }

    void setRotation( float rot )
    {
        m_Rotation       = rot;
        m_TransformDirty = true;
    }

    float getRotation() const
    {
        return m_Rotation;
    }

    /// <summary>
    /// Add to the current translation of the transform.
    /// </summary>
    /// <param name="translation">The translation to apply to the position of the object.</param>
    void translate( const glm::vec2& translation )
    {
        m_Position += translation;
        m_TransformDirty = true;
    }

    /// <summary>
    /// Apply a scale to the object.
    /// The current scale of the object is multiplied by this scale factor.
    /// </summary>
    /// <param name="factor">The scale factor to apply to the current scale of the object.</param>
    void scale( const glm::vec2& factor )
    {
        m_Scale *= factor;
        m_TransformDirty = true;
    }

    void rotate( float rotation )
    {
        m_Rotation += rotation;
        m_TransformDirty = true;
    }

    /// <summary>
    /// Get the 3x3 transform matrix.
    /// </summary>
    /// <returns></returns>
    const glm::mat3& getTransform() const;

private:
    // The anchor point determines the origin of the applied transformations.
    glm::vec2 m_Anchor { 0 };
    // The position of the object in 2D space.
    glm::vec2 m_Position { 0 };
    // The scale of the object in the local X and Y axis.
    glm::vec2 m_Scale { 1 };
    // The rotation angle (in radians) around the Z axis.
    float m_Rotation { 0.0f };

    // The 2D transformation matrix that combines the position, scale, and rotation in one.
    mutable glm::mat3 m_Transform { 1 };
    mutable bool      m_TransformDirty = true;
};

/// <summary>
/// Transform a 2D AABB by a Transform2D.
/// </summary>
/// <param name="aabb">The AABB to transform.</param>
/// <param name="transform">The transformation to apply.</param>
/// <returns>The transformed AABB.</returns>
inline AABB operator*( const AABB& aabb, const Transform2D& transform )
{
    const auto mat = transform.getTransform();
    const auto min = glm::vec3 { aabb.min.x, aabb.min.y, 1 };
    const auto max = glm::vec3 { aabb.max.x, aabb.max.y, 1 };

    return { min * mat, max * mat };
}

/// <summary>
/// Transform a 2D AABB by a Transform2D.
/// </summary>
/// <param name="aabb">The AABB to transform.</param>
/// <param name="transform">The transformation to apply.</param>
/// <returns>The transformed AABB.</returns>
inline AABB& operator*=( AABB& aabb, const Transform2D& transform )
{
    const auto mat = transform.getTransform();

    aabb.min = mat * glm::vec3 { aabb.min.x, aabb.min.y, 1 };
    aabb.max = mat * glm::vec3 { aabb.max.x, aabb.max.y, 1 };

    return aabb;
}

}  // namespace Math
