#pragma once

#include "BlendMode.hpp"
#include "Color.hpp"
#include "Config.hpp"
#include "Enums.hpp"
#include "aligned_unique_ptr.hpp"

#include <Math/AABB.hpp>
#include <Math/Transform2D.hpp>

#include <cassert>
#include <filesystem>
#include <memory>

#include <glm/vec2.hpp>

namespace sr
{
class Sprite;

struct SR_API Image
{
    static Image fromFile( const std::filesystem::path& fileName );
    static Image fromMemory( const Color* data, uint32_t width, uint32_t height );

    Image();
    Image( uint32_t width, uint32_t height );
    Image( const Image& copy );
    Image( Image&& move ) noexcept;
    ~Image() = default;

    Image& operator=( const Image& image );
    Image& operator=( Image&& image ) noexcept;

    void resize( uint32_t width, uint32_t height );

    /// <summary>
    /// Clear the image to a single color.
    /// </summary>
    /// <param name="color">The color to clear the screen to.</param>
    void clear( const Color& color ) noexcept;

    /// <summary>
    /// Draw a line on the image.
    /// </summary>
    /// <param name="x0">The x-coordinate of the start point of the line.</param>
    /// <param name="y0">The y-coordinate of the start point of the line.</param>
    /// <param name="x1">The x-coordinate of the end point of the line.</param>
    /// <param name="y1">The y-coordinate of the end point of the line.</param>
    /// <param name="color">The color of the line.</param>
    /// <param name="blendMode">The blend mode to use.</param>
    void drawLine( int x0, int y0, int x1, int y1, const Color& color, const BlendMode& blendMode = {} ) noexcept;

    /// <summary>
    /// Draw a line on the image.
    /// </summary>
    /// <param name="p0">The start point.</param>
    /// <param name="p1">The end point.</param>
    /// <param name="color">The color of the line.</param>
    /// <param name="blendMode">The blend mode to use.</param>
    void drawLine( const glm::ivec2& p0, const glm::ivec2& p1, const Color& color, const BlendMode& blendMode = {} ) noexcept
    {
        drawLine( p0.x, p0.y, p1.x, p1.y, color, blendMode );
    }

    /// <summary>
    /// Draw a line on the image.
    /// </summary>
    /// <param name="p0">The start point.</param>
    /// <param name="p1">The end point.</param>
    /// <param name="color">The color of the line.</param>
    /// <param name="blendMode">The blend mode to use.</param>
    void drawLine( const glm::vec2& p0, const glm::vec2& p1, const Color& color, const BlendMode& blendMode ) noexcept
    {
        drawLine( static_cast<int>( p0.x ), static_cast<int>( p0.y ), static_cast<int>( p1.x ), static_cast<int>( p1.y ), color, blendMode );
    }

    /// <summary>
    /// Plot a 2D triangle.
    /// </summary>
    /// <param name="p0">The first triangle coordinate.</param>
    /// <param name="p1">The second triangle coordinate.</param>
    /// <param name="p2">The third triangle coordinate.</param>
    /// <param name="color">The triangle color.</param>
    /// <param name="blendMode">The blend mode to apply.</param>
    /// <param name="fillMode">The fill mode to use when rendering.</param>
    void drawTriangle( const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2, const Color& color, const BlendMode& blendMode = {}, FillMode fillMode = FillMode::Solid ) noexcept;

    /// <summary>
    /// Draw an axis-aligned bounding box to the image.
    /// </summary>
    /// <param name="aabb">The AABB to draw.</param>
    /// <param name="color">The color of the AABB.</param>
    /// <param name="blendMode">The blend mode to use.</param>
    /// <param name="fillMode">The fill mode to use to draw the AABB.</param>
    void drawAABB( Math::AABB aabb, const Color& color, const BlendMode& blendMode = {}, FillMode fillMode = FillMode::Solid ) noexcept;

    /// <summary>
    /// Draw a rectangle to the image.
    /// </summary>
    /// <param name="rect">The rectangle to draw.</param>
    /// <param name="color">The color of the rectangle.</param>
    /// <param name="blendMode">The blend mode to use.</param>
    /// <param name="fillMode">The fill mode to use to draw the rectangle.</param>
    /// <returns></returns>
    void drawRect( const Math::RectI& rect, const Color& color, const BlendMode& blendMode = {}, FillMode fillMode = FillMode::Solid ) noexcept;

    /// <summary>
    /// Draw a sprite on the screen using the given transform.
    /// </summary>
    /// <param name="sprite">The sprite to draw.</param>
    /// <param name="transform">The transform to apply to the sprite.</param>
    void drawSprite( const Sprite& sprite, const Math::Transform2D& transform ) noexcept;

    /// <summary>
    /// Plot a single pixel to the image. Out-of-bounds coordinates are discarded.
    /// </summary>
    /// <param name="x">The x-coordinate to plot.</param>
    /// <param name="y">The y-coordinate to plot.</param>
    /// <param name="src">The source color of the pixel to plot.</param>
    /// <param name="blendMode">The blend mode to apply.</param>
    void plot( uint32_t x, uint32_t y, const Color& src, const BlendMode& blendMode = {} ) noexcept
    {
        if ( x >= m_width || y >= m_height )
            return;

        const size_t i   = static_cast<size_t>( y ) * m_width + x;
        const Color  dst = m_data[i];
        m_data[i]        = blendMode.Blend( src, dst );
    }

    /// <summary>
    /// Sample the image at integer coordinates.
    /// </summary>
    /// <param name="u">The U texture coordinate.</param>
    /// <param name="v">The V texture coordinate.</param>
    /// <param name="addressMode">Determines how to apply out-of-bounds texture coordinates.</param>
    /// <returns>The color of the texel at the given UV coordinates.</returns>
    const Color& sample( int u, int v, AddressMode addressMode = AddressMode::Wrap ) const noexcept;

    /// <summary>
    /// Sample the image at integer coordinates.
    /// </summary>
    /// <param name="uv">The texture coordinates.</param>
    /// <param name="addressMode">The address mode to use during sampling.</param>
    /// <returns>The color of the texel at the given UV coordinates.</returns>
    const Color& sample( const glm::ivec2& uv, AddressMode addressMode = AddressMode::Wrap ) const noexcept
    {
        return sample( uv.x, uv.y, addressMode );
    }

    /// <summary>
    /// Sample the image using normalized texture coordinates (in the range from [0..1]).
    /// </summary>
    /// <param name="u">The normalized U texture coordinate.</param>
    /// <param name="v">The normalized V texture coordinate.</param>
    /// <param name="addressMode">The addressing mode to use during sampling.</param>
    /// <returns>The color of the texel at the given UV texture coordinates.</returns>
    const Color& sample( float u, float v, AddressMode addressMode = AddressMode::Wrap ) const noexcept
    {
        return sample( static_cast<int>( u * static_cast<float>( m_width ) ), static_cast<int>( v * static_cast<float>( m_height ) ), addressMode );
    }

    /// <summary>
    /// Sample the image using normalized texture coordinates (in the range from [0..1]).
    /// </summary>
    /// <param name="uv">The normalized texture coordinates.</param>
    /// <param name="addressMode">The addressing mode to use during sampling.</param>
    /// <returns>The color of the texel at the given UV texture coordinates.</returns>
    const Color& sample( const glm::vec2& uv, AddressMode addressMode = AddressMode::Wrap ) const noexcept
    {
        return sample( uv.x, uv.y, addressMode );
    }

    const Color& operator()( uint32_t x, uint32_t y ) const
    {
        assert( x < m_width );
        assert( y < m_height );

        return m_data[static_cast<uint64_t>( y ) * m_width + x];
    }

    Color& operator()( uint32_t x, uint32_t y )
    {
        assert( x < m_width );
        assert( y < m_height );

        return m_data[static_cast<uint64_t>( y ) * m_width + x];
    }

    uint32_t getWidth() const noexcept
    {
        return m_width;
    }

    uint32_t getHeight() const noexcept
    {
        return m_height;
    }

    Color* data() noexcept
    {
        return m_data.get();
    }

    const Color* data() const noexcept
    {
        return m_data.get();
    }

private:
    uint32_t m_width  = 0u;
    uint32_t m_height = 0u;
    // Axis-aligned bounding box used for screen clipping.
    Math::AABB                  m_AABB;
    aligned_unique_ptr<Color[]> m_data;
};
}  // namespace sr