#include <Image.hpp>
#include <Timer.hpp>
#include <Window.hpp>

#include <iostream>

using namespace sr;

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

    Window window { L"02 - Triangle", WINDOW_WIDTH, WINDOW_HEIGHT };

    Image monaLisa = Image::fromFile( "assets/textures/Mona_Lisa.jpg" );

    Image image { WINDOW_WIDTH, WINDOW_HEIGHT };

    window.show();

    Timer    timer;
    double   totalTime  = 0.0;
    uint64_t frameCount = 0ull;

    while ( window )
    {
        image.clear( Color::Black );

        //image.line( 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Color::White );
        //image.line( 0, WINDOW_HEIGHT, WINDOW_WIDTH, 0, Color::White );
        //image.line( WINDOW_WIDTH / 2, 0, WINDOW_WIDTH / 2, WINDOW_HEIGHT, Color::White );
        //image.line( 0, WINDOW_HEIGHT / 2, WINDOW_WIDTH, WINDOW_HEIGHT / 2, Color::White );
        image.triangle( { WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.25 }, { WINDOW_WIDTH * 0.75f, WINDOW_HEIGHT * 0.75f }, { WINDOW_WIDTH * 0.25f, WINDOW_HEIGHT * 0.75f }, Color::Yellow );
        //image.triangle( { 0, 0 }, { WINDOW_WIDTH * 2.0f, 0.0f }, { 0.0f, WINDOW_HEIGHT * 2.0f }, Color::Yellow );
        
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