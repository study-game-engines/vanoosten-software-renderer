#pragma once

#include "Config.hpp"
#include "BlendMode.hpp"
#include "SpriteSheet.hpp"

#include <filesystem>
#include <optional>

namespace sr
{
class SR_API SpriteAnim
{
public:
    SpriteAnim() = default;
    ~SpriteAnim() = default;
    SpriteAnim( const SpriteAnim& ) = default;
    SpriteAnim( SpriteAnim&& )      = default;

    /// <summary>
    /// Create a SpriteAnim based on a sprite sheet and a frame-rate for the animation.
    /// </summary>
    /// <param name="fileName">The file that contains the sprite sheet for the animation.</param>
    /// <param name="fps">(optional) The frame-rate of the animation. Default: 30 FPS.</param>
    /// <param name="spriteWidth">(optional) The width of a sprite in the sprite sheet. Default: The width of the sprite sheet.</param>
    /// <param name="spriteHeight">(optional) The height of a sprite in the sprite sheet. Default: The height of the sprite sheet.</param>
    /// <param name="blendMode">(optional) The blend mode to use when rendering the sprites. Default: Alpha blended./param>
    explicit SpriteAnim( const std::filesystem::path& fileName, uint32_t fps = 30u, std::optional<uint32_t> spriteWidth = {}, std::optional<uint32_t> spriteHeight = {}, const BlendMode& blendMode = BlendMode::AlphaBlend );

    /// <summary>
    /// Allow conversion to sprite.
    /// </summary>
    /// <returns>The current Sprite frame to render for this Sprite animation.</returns>
    operator const Sprite&() const noexcept;

    SpriteAnim& operator=( const SpriteAnim& ) = default;
    SpriteAnim& operator=( SpriteAnim&& ) noexcept = default;

    /// <summary>
    /// Get a sprite from the sprite sheet.
    /// </summary>
    /// <param name="i">The index of the sprite in the sprite sheet.
    /// The sprite returned is `i % numSprites`.
    /// </param>
    /// <returns>The sprite in the sprite sheet.</returns>
    const Sprite& operator[]( size_t i ) const noexcept;

    /// <summary>
    /// Get the sprite at a specific moment in time.
    /// </summary>
    /// <param name="time">The animation time.</param>
    /// <returns>A reference to the sprite at the given time.</returns>
    const Sprite& at( float time ) const noexcept;

    /// <summary>
    /// Update the internal timer for the sprite animation.
    /// The frame of the sprite animation is chosen based on the animation timer.
    /// </summary>
    /// <param name="deltaTime">The elapsed time in seconds.</param>
    void update( float deltaTime ) noexcept;

    /// <summary>
    /// Reset the internal frame timer.
    /// </summary>
    void reset() noexcept;

    /// <summary>
    /// Get the duration (in seconds) of the animation.
    /// </summary>
    /// <returns>The duration of the animation (in seconds).</returns>
    float getDuration() const noexcept;

    /// <summary>
    /// For "one shot" sprite animations, the animation is done
    /// when all the frames in the sprite animation have played.
    /// </summary>
    /// <returns>`true` if all of the frames of the animation have played at least once, `false` otherwise.</returns>
    bool isDone() const noexcept;

private:
    SpriteSheet spriteSheet;
    uint32_t frameRate = 30u; // Default to 30 FPS.
    float    time = 0.0f;
};
}
