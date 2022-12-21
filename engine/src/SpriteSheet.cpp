#include <SpriteSheet.hpp>

using namespace sr;

SpriteSheet::SpriteSheet( const std::filesystem::path& fileName, std::optional<uint32_t> spriteWidth, std::optional<uint32_t> spriteHeight, const BlendMode& blendMode )
{
    Image image = Image::fromFile( fileName );

    if ( !image )
        return;

    if ( !spriteWidth )
        spriteWidth = image.getWidth();

    if ( !spriteHeight )
        spriteHeight = image.getHeight();

    init( std::move( image ), *spriteWidth, *spriteHeight, blendMode );
}

SpriteSheet::SpriteSheet( const std::filesystem::path& fileName, std::span<const Math::RectI> rects, const BlendMode& blendMode )
: image { Image::fromFile( fileName ) }
, columns { static_cast<uint32_t>( rects.size() ) }
, rows { 1u }
{
    sprites.reserve( rects.size() );

    for ( auto& rect: rects )
    {
        sprites.emplace_back( image, rect, blendMode );
    }
}

SpriteSheet::SpriteSheet( const SpriteSheet& copy )
{
    if ( &copy == this || !copy.image )
        return;

    init( copy.image, copy.image.getWidth() / copy.columns, copy.image.getHeight() / copy.rows, copy.blendMode );
}

SpriteSheet::SpriteSheet( SpriteSheet&& other ) noexcept
{
    if ( &other == this || !other.image )
        return;

    const auto spriteWidth  = other.image.getWidth() / other.columns;
    const auto spriteHeight = other.image.getHeight() / other.rows;

    init( std::move( other.image ), spriteWidth, spriteHeight, other.blendMode );

    other.rows    = 0u;
    other.columns = 0u;
    other.sprites.clear();
}

SpriteSheet& SpriteSheet::operator=( const SpriteSheet& copy )
{
    if ( &copy == this )
        return *this;

    init( copy.image, copy.image.getWidth() / copy.columns, copy.image.getHeight() / copy.rows, copy.blendMode );

    return *this;
}

SpriteSheet& SpriteSheet::operator=( SpriteSheet&& other ) noexcept
{
    if ( &other == this || !other.image )
        return *this;

    const auto spriteWidth  = other.image.getWidth() / other.columns;
    const auto spriteHeight = other.image.getHeight() / other.rows;

    init( std::move( other.image ), spriteWidth, spriteHeight, other.blendMode );

    other.columns = 0u;
    other.rows    = 0u;
    other.sprites.clear();

    return *this;
}

const Sprite& SpriteSheet::operator[]( size_t index ) const noexcept
{
    if ( index < sprites.size() )
        return sprites[index];

    static const Sprite emptySprite;
    return emptySprite;
}

const Sprite& SpriteSheet::operator()( size_t i, size_t j ) const noexcept
{
    return operator[]( i* columns + j );
}

SpriteSheet SpriteSheet::fromGrid( const std::filesystem::path& fileName, uint32_t columns, uint32_t rows, const BlendMode& blendMode )
{
    Image image = Image::fromFile( fileName );

    // Make sure the image can be split up evenly by the number of sprites.
    assert( image.getWidth() % columns == 0 );
    assert( image.getHeight() % rows == 0 );

    const uint32_t spriteWidth  = image.getWidth() / columns;
    const uint32_t spriteHeight = image.getHeight() / rows;

    SpriteSheet sheet;
    sheet.init( std::move( image ), spriteWidth, spriteHeight, blendMode );

    return sheet;
}

void SpriteSheet::init( Image _image, uint32_t spriteWidth, uint32_t spriteHeight, const BlendMode& _blendMode )
{
    image = std::move( _image );

    blendMode = _blendMode;

    // Make sure the image can be split up evenly by the size of the sprites.
    assert( image.getWidth() % spriteWidth == 0 );
    assert( image.getHeight() % spriteHeight == 0 );

    columns = image.getWidth() / spriteWidth;
    rows    = image.getHeight() / spriteHeight;

    sprites.reserve( static_cast<size_t>( columns ) * rows );

    const int w = static_cast<int>( image.getWidth() ) / columns;
    const int h = static_cast<int>( image.getHeight() ) / rows;

    for ( int i = 0; i < rows; ++i )
    {
        for ( int j = 0; j < columns; ++j )
        {
            sprites.emplace_back( image, Math::RectI { j * w, i * h, w, h }, blendMode );
        }
    }
}
