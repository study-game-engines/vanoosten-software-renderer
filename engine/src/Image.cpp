#include "Image.hpp"
#include "Font.hpp"
#include "Sprite.hpp"
#include "Vertex.hpp"
#include "stb_image.h"

#include <Math/AABB.hpp>
#include <Math/Math.hpp>

#include <algorithm>
#include <omp.h>
#include <optional>

using namespace sr;
using namespace Math;

Image Image::fromFile( const std::filesystem::path& fileName )
{
    int            x, y, n;
    unsigned char* data = stbi_load( fileName.string().c_str(), &x, &y, &n, STBI_rgb_alpha );
    if ( !data )
        return {};

    // Convert ARGB
    unsigned char* p = data;
    for ( size_t i = 0; i < x * y; ++i )
    {
        unsigned char c = p[0];
        p[0]            = p[2];
        p[2]            = c;
        p += 4;
    }

    Image image = fromMemory( reinterpret_cast<Color*>( data ), x, y );

    stbi_image_free( data );

    return image;
}

Image Image::fromMemory( const Color* data, uint32_t width, uint32_t height )
{
    if ( data == nullptr )
        return {};

    Image image { width, height };
    memcpy_s( image.data(), static_cast<rsize_t>( image.m_width ) * image.m_height * sizeof( Color ), data, static_cast<rsize_t>( width ) * height * sizeof( Color ) );

    return image;
}

Image::Image() = default;

Image::Image( const Image& copy )
{
    resize( copy.m_width, copy.m_height );
    memcpy_s( data(), static_cast<rsize_t>( m_width ) * m_height * sizeof( Color ), copy.data(), static_cast<rsize_t>( copy.m_width ) * copy.m_height * sizeof( Color ) );
}

Image::Image( Image&& move ) noexcept
: m_width { move.m_width }
, m_height { move.m_height }
, m_AABB { { 0, 0, 0 }, { m_width - 1, m_height - 1, 0 } }
, m_data { std::move( move.m_data ) }
{
    move.m_width  = 0u;
    move.m_height = 0u;
}

Image::Image( uint32_t width, uint32_t height )
{
    resize( width, height );
}

Image& Image::operator=( const Image& image )
{
    resize( image.m_width, image.m_height );
    memcpy_s( data(), static_cast<rsize_t>( m_width ) * m_height * sizeof( Color ), image.data(), static_cast<rsize_t>( image.m_width ) * image.m_height * sizeof( Color ) );

    return *this;
}

Image& Image::operator=( Image&& image ) noexcept
{
    m_width  = image.m_width;
    m_height = image.m_height;
    m_AABB   = AABB { { 0, 0, 0 }, { m_width - 1, m_height - 1, 0 } };

    m_data = std::move( image.m_data );

    image.m_width  = 0u;
    image.m_height = 0u;

    return *this;
}

void Image::resize( uint32_t width, uint32_t height )
{
    if ( m_width == width && m_height == height )
        return;

    m_width  = width;
    m_height = height;
    m_AABB   = {
          { 0, 0, 0 },
          { m_width - 1, m_height - 1, 0 }
    };

    // Align color buffer to 64-byte boundary for better cache alignment on 64-bit architectures.
    m_data = make_aligned_unique<Color[], 64>( static_cast<uint64_t>( width ) * height );
}

void Image::clear( const Color& color ) noexcept
{
    Color* p = data();

#pragma omp parallel for
    for ( int i = 0; i < static_cast<int>( m_width * m_height ); ++i )
        p[i] = color;
}

void Image::copy( const Image& srcImage, std::optional<Math::RectI> srcRect, std::optional<Math::RectI> dstRect, const BlendMode& blendMode )
{
    // If the source rectangle is not provided, use the entire source image.
    AABB srcAABB = AABB::fromRect( srcRect ? *srcRect : srcImage.getRect() );
    // If the destination rect is not provided, use the entire source image.
    // I assume that the "expected behaviour" of this method to copy the source image to the
    // destination image (without scaling) even if that results in clipping of the source image.
    AABB dstAABB = AABB::fromRect( dstRect ? *dstRect : srcImage.getRect() );

    // If the source AABB doesn't intersect with the source image bounds.
    // In other words, the source image rectangle doesn't cover any part of the source image.
    if ( !srcImage.m_AABB.intersect( srcAABB ) )
        return;

    // Clamp the source AABB to the AABB of the source image (to prevent sampling outside of the source image bounds).
    srcAABB.clamp( srcImage.m_AABB );

    // Source width
    const int sW = static_cast<int>( srcAABB.width() );
    // Source height
    const int sH = static_cast<int>( srcAABB.height() );

    // If the destination AABB doesn't intersect with this image bounds...
    // In other words, the destination bounds is completely offscreen.
    if ( !m_AABB.intersect( dstAABB ) )
        return;

    // Destination width
    const int dW = static_cast<int>( dstAABB.width() );
    // Destination height
    const int dH = static_cast<int>( dstAABB.height() );

    // Clamp the dstAABB to the bounds of this image (to prevent writing outside of this image's bounds).
    AABB dstImage = dstAABB.clamped( m_AABB );

    // Clamped image width.
    const int iW = static_cast<int>( dstImage.width() );
    // Clamped image height.
    const int iH = static_cast<int>( dstImage.height() );
    // Clamped image area
    const int iA = iW * iH;

    // Pointer to source image data.
    const Color* src = srcImage.data();
    // Pointer to destination image data.
    Color* dst = data();

#pragma omp parallel for firstprivate( srcAABB, dstAABB, dstImage, sW, sH, dW, dH, iW, iH )
    for ( int i = 0; i < iA; ++i )
    {
        const int x  = i % iW;
        const int y  = i / iW;
        const int dx = x + static_cast<int>( dstImage.min.x );
        const int dy = y + static_cast<int>( dstImage.min.y );
        const int sx = ( x * sW / dW ) + static_cast<int>( srcAABB.min.x );
        const int sy = ( y * sH / dH ) + static_cast<int>( srcAABB.min.y );

        const Color sC = src[sy * srcImage.getWidth() + sx];
        const Color dC = dst[dy * m_width + dx];

        dst[dy * m_width + dx] = blendMode.Blend( sC, dC );
    }
}

void Image::copy( const Image& srcImage, int x, int y )
{
    // Source image coords.
    int sX = x < 0 ? -x : 0;
    int sY = y < 0 ? -y : 0;
    int sW = static_cast<int>( srcImage.getWidth() ) - sX;
    int sH = static_cast<int>( srcImage.getHeight() ) - sY;

    // Check if source image is offscreen.
    if ( sW <= 0 || sH <= 0 )
        return;

    // Destination coords.
    int dX = x < 0 ? 0 : x;
    int dY = y < 0 ? 0 : y;
    int dW = static_cast<int>( m_width ) - dX;
    int dH = static_cast<int>( m_height ) - dY;

    // Check if the destination range is offscreen.
    if ( dW <= 0 || dH <= 0 )
        return;

    // The destination copy region is the minimum of the source
    // and destination dimensions.
    int w = std::min( sW, dW );
    int h = std::min( sH, dH );

    const uint32_t srcWidth = srcImage.getWidth();
    const Color*   src      = srcImage.data();
    Color*         dst      = data();

#pragma parallel for firstprivate( w, h, sX, dX )
    for ( int i = 0; i < h; ++i )
        memcpy_s( dst + ( i + dY ) * m_width + dX, w * sizeof( Color ), src + ( i + sY ) * srcWidth + sX, w * sizeof( Color ) );
}

void Image::drawLine( int x0, int y0, int x1, int y1, const Color& color, const BlendMode& blendMode ) noexcept
{
    const int dx = std::abs( x1 - x0 );
    const int dy = -std::abs( y1 - y0 );
    const int sx = x0 < x1 ? 1 : -1;
    const int sy = y0 < y1 ? 1 : -1;

    int err = dx + dy;

    while ( true )
    {
        plot( x0, y0, color, blendMode );
        const int e2 = err * 2;

        if ( e2 >= dy )
        {
            if ( x0 == x1 )
                break;

            err += dy;
            x0 += sx;
        }
        if ( e2 <= dx )
        {
            if ( y0 == y1 )
                break;

            err += dx;
            y0 += sy;
        }
    }
}

void Image::drawTriangle( const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2, const Color& color, const BlendMode& blendMode, FillMode fillMode ) noexcept
{
    // Create an AABB for the triangle.
    AABB aabb = AABB::fromTriangle( { p0, 0 }, { p1, 0 }, { p2, 0 } );

    // Check if the triangle is on screen.
    if ( !m_AABB.intersect( aabb ) )
        return;

    switch ( fillMode )
    {
    case FillMode::WireFrame:
    {
        drawLine( p0, p1, color, blendMode );
        drawLine( p1, p2, color, blendMode );
        drawLine( p2, p0, color, blendMode );
    }
    break;
    case FillMode::Solid:
    {
        // Clamp the triangle AABB to the screen bounds.
        aabb.clamp( m_AABB );

        const int width  = static_cast<int>( aabb.width() );
        const int height = static_cast<int>( aabb.height() );
        const int area   = width * height;

#pragma omp parallel for firstprivate( aabb, width, height, area )
        for ( int i = 0; i < area; ++i )
        {
            const int x = ( i % width ) + static_cast<int>( aabb.min.x );
            const int y = ( i / width ) + static_cast<int>( aabb.min.y );
            if ( pointInsideTriangle( { x, y }, p0, p1, p2 ) )
                plot( x, y, color, blendMode );
        }
    }
    break;
    }
}

void Image::drawQuad( const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const Color& color, const BlendMode& blendMode, FillMode fillMode ) noexcept
{
    AABB aabb = AABB::fromQuad( { p0, 0 }, { p1, 0 }, { p2, 0 }, { p3, 0 } );

    // Check if the triangle is on screen.
    if ( !m_AABB.intersect( aabb ) )
        return;

    switch ( fillMode )
    {
    case FillMode::WireFrame:
    {
        drawLine( p0, p1, color, blendMode );
        drawLine( p1, p2, color, blendMode );
        drawLine( p2, p3, color, blendMode );
        drawLine( p3, p0, color, blendMode );
    }
    break;
    case FillMode::Solid:
    {
        glm::vec2 verts[] = {
            p0, p1, p2, p3
        };

        // Index buffer for the two triangles of the quad.
        const uint32_t indicies[] = {
            0, 1, 3,
            1, 2, 3
        };

        // Clamp to the size of the screen.
        aabb.clamp( m_AABB );
        
#pragma omp parallel for schedule( dynamic ) firstprivate( aabb, indicies, verts )
        for ( int y = static_cast<int>( aabb.min.y ); y <= static_cast<int>( aabb.max.y ); ++y )
        {
            for ( int x = static_cast<int>( aabb.min.x ); x <= static_cast<int>( aabb.max.x ); ++x )
            {
                for ( uint32_t i = 0; i < std::size( indicies ); i += 3 )
                {
                    const uint32_t i0 = indicies[i + 0];
                    const uint32_t i1 = indicies[i + 1];
                    const uint32_t i2 = indicies[i + 2];

                    glm::vec3 bc = barycentric( verts[i0], verts[i1], verts[i2], { x, y } );
                    if ( barycentricInside( bc ) )
                    {
                        plot( static_cast<uint32_t>( x ), static_cast<uint32_t>( y ), color, blendMode );
                    }
                }
            }
        }
    }
    break;
    }
}

void Image::drawAABB( AABB aabb, const Color& color, const BlendMode& blendMode, FillMode fillMode ) noexcept
{
    if ( !m_AABB.intersect( aabb ) )
        return;

    switch ( fillMode )
    {
    case FillMode::WireFrame:
    {
        const glm::ivec2 min     = aabb.min;
        const glm::ivec2 max     = aabb.max;
        const glm::ivec2 verts[] = { { min.x, min.y }, { max.x, min.y }, { max.x, max.y }, { min.x, max.y } };

        for ( int i = 0; i < 3; ++i )
        {
            drawLine( verts[i], verts[( i + 1 ) % 4], color, blendMode );
        }
    }
    break;
    case FillMode::Solid:
    {
        // Clamp to screen bounds.
        aabb.clamp( m_AABB );

        const int width  = static_cast<int>( aabb.width() );
        const int height = static_cast<int>( aabb.height() );
        const int area   = width * height;

#pragma omp parallel for firstprivate( aabb, width, height, area )
        for ( int i = 0; i < area; ++i )
        {
            const int x = ( i % width ) + static_cast<int>( aabb.min.x );
            const int y = ( i / width ) + static_cast<int>( aabb.min.y );
            plot( x, y, color, blendMode );
        }
    }
    break;
    }
}

void Image::drawSprite( const Sprite& sprite, const Math::Transform2D& transform ) noexcept
{
    const Image* image = sprite.getImage();
    if ( !image )
        return;

    const Color       color     = sprite.getColor();
    const BlendMode   blendMode = sprite.getBlendMode();
    const glm::ivec2& uv        = sprite.getUV();
    const glm::ivec2& size      = sprite.getSize();

    const glm::mat3& matrix = transform.getTransform();

    Vertex verts[] = {
        Vertex { { 0, 0 }, { uv.x, uv.y }, color },                             // Top-left
        Vertex { { size.x, 0 }, { uv.x + size.x, 0 }, color },                  // Top-right
        Vertex { { 0, size.y }, { 0, uv.y + size.y }, color },                  // Bottom-left
        Vertex { { size.x, size.y }, { uv.x + size.x, uv.y + size.y }, color }  // Bottom-right
    };

    // Transform verts.
    for ( Vertex& v: verts )
    {
        v.position = matrix * glm::vec3 { v.position, 1.0f };
    }

    // Compute an AABB over the sprite quad.
    AABB aabb {
        { verts[0].position, 0.0f },
        { verts[1].position, 0.0f },
        { verts[2].position, 0.0f },
        { verts[3].position, 0.0f }
    };

    // Check if the AABB of the sprite is on screen.
    if ( !m_AABB.intersect( aabb ) )
        return;

    // Clamp to the size of the screen.
    aabb.clamp( m_AABB );

    // Index buffer for the two triangles of the quad.
    const uint32_t indicies[] = {
        0, 1, 2,
        1, 3, 2
    };

#pragma omp parallel for schedule( dynamic ) firstprivate( aabb, indicies, verts, color, blendMode )
    for ( int y = static_cast<int>( aabb.min.y ); y <= static_cast<int>( aabb.max.y ); ++y )
    {
        for ( int x = static_cast<int>( aabb.min.x ); x <= static_cast<int>( aabb.max.x ); ++x )
        {
            for ( uint32_t i = 0; i < std::size( indicies ); i += 3 )
            {
                const uint32_t i0 = indicies[i + 0];
                const uint32_t i1 = indicies[i + 1];
                const uint32_t i2 = indicies[i + 2];

                glm::vec3 bc = barycentric( verts[i0].position, verts[i1].position, verts[i2].position, { x, y } );
                if ( barycentricInside( bc ) )
                {
                    // Compute interpolated UV
                    const glm::ivec2 texCoord = verts[i0].texCoord * bc.x + verts[i1].texCoord * bc.y + verts[i2].texCoord * bc.z;
                    // Sample the sprite's texture.
                    const Color c = image->sample( texCoord.x, texCoord.y ) * color;
                    // Plot.
                    plot( static_cast<uint32_t>( x ), static_cast<uint32_t>( y ), c, blendMode );
                }
            }
        }
    }
}

void Image::drawText( const Font& font, int x, int y, std::string_view text, const Color& color ) noexcept
{
    font.drawText( *this, text, x, y, color );
}

const Color& Image::sample( int u, int v, AddressMode addressMode ) const noexcept
{
    const int w = static_cast<int>( m_width );
    const int h = static_cast<int>( m_height );

    switch ( addressMode )
    {
    case AddressMode::Wrap:
    {
        u = u % w;
        v = v % h;
    }
    break;
    case AddressMode::Mirror:
    {
        const int U = u / w;
        const int V = v / h;

        u = ( U % 2 == 0 ) ? u % w : w - ( u % w );
        v = ( V % 2 == 0 ) ? v % h : h - ( v % h );
    }
    break;
    case AddressMode::Clamp:
    {
        u = std::clamp( u, 0, w );
        v = std::clamp( v, 0, h );
    }
    break;
    }

    assert( u >= 0 && u < w );
    assert( v >= 0 && v < h );

    return m_data[static_cast<uint64_t>( v ) * m_width + u];
}
