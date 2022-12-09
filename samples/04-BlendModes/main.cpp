#include <Image.hpp>
#include <Sprite.hpp>
#include <Timer.hpp>
#include <Window.hpp>

#include <iostream>

using namespace sr;
using namespace Math;

int main( int argc, char* argv[] )
{
    // Parse command-line arguments.
    if ( argc > 1 )
    {
        for ( int i = 0; i < argc; ++i )
        {
            if ( strcmp( argv[i], "-cwd" ) == 0 )
            {
                std::string workingDirectory = argv[++i];
                std::filesystem::current_path( workingDirectory );
            }
        }
    }

    const int WINDOW_WIDTH  = 800;
    const int WINDOW_HEIGHT = 600;

    Window window { L"04 - Blend Modes", WINDOW_WIDTH, WINDOW_HEIGHT };

    Image       monaLisa = Image::fromFile( "assets/textures/Mona_Lisa.jpg" );
    Sprite      sprite { monaLisa, BlendMode::AlphaBlend };
    Transform2D transform;

    // Rotate around the center of the screen.
    transform.setAnchor( { monaLisa.getWidth() / 2.0f, monaLisa.getHeight() / 2.0f } );
    transform.setPosition( { WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f } );

    transform.scale( { 0.1f, 0.1f } );

    // Image to render to.
    Image image { WINDOW_WIDTH, WINDOW_HEIGHT };

    window.show();

    Timer    timer;
    double   totalTime  = 0.0;
    uint64_t frameCount = 0ull;

    while ( window )
    {
        image.clear( Color::Black );

        // Rotate the sprite.
        transform.rotate( timer.elapsedSeconds() );
        image.drawSprite( sprite, transform );

        // Draw a semi-transparent yellow AABB.
        Color      color = Color::Yellow.withAlpha( ( static_cast<float>( std::sin( timer.totalSeconds() ) + 1.0f ) / 2.0f ) );
        const AABB aabb { { WINDOW_WIDTH * 0.25f, WINDOW_HEIGHT * 0.25f, 0.0f }, { WINDOW_WIDTH * 0.75f, WINDOW_HEIGHT * 0.75f, 0.0f } };
        image.drawAABB( aabb, color, BlendMode::AlphaBlend );

        window.present( image );

        Event e;
        while ( window.popEvent( e ) )
        {
            switch ( e.type )
            {
            case Event::Close:
                window.destroy();
                break;
            case Event::KeyPressed:
                switch ( e.key.code )
                {
                case KeyCode::Escape:
                    window.destroy();
                    break;
                }
                break;
            }
        }

        timer.tick();
        ++frameCount;

        totalTime += timer.elapsedSeconds();
        if ( totalTime > 1.0 )
        {
            std::cout << std::format( "FPS: {:.3f}\n", frameCount / totalTime );

            frameCount = 0;
            totalTime  = 0.0;
        }
    }
}