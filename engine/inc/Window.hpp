#pragma once

#include "Config.hpp"
#include "Events.hpp"
#include "Image.hpp"

#include <glm/vec2.hpp>

#include <memory>
#include <string>

namespace sr
{
    class WindowImpl;

    class SR_API Window
    {
    public:
        Window();
        Window(std::wstring_view title, int width, int height);
        ~Window();

        // Copies not allowed.
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        // Moves are allowed.
        Window(Window&&) noexcept;
        Window& operator=(Window&&) noexcept;

        // (Re)create the window.
        void create(std::wstring_view title, int width, int height);

        bool popEvent(Event& event);

        void show();

        void setVSync( bool enabled );

        bool isVSync() const noexcept;

        /// <summary>
        /// Clear the window contents.
        /// </summary>
        /// <param name="color">The color to clear the window to.</param>
        void clear( const Color& color );

        /// <summary>
        /// Present the image to the screen.
        /// </summary>
        /// <param name="image">The image to present.</param>
        void present(const Image& image);

        void destroy();

        int getWidth() const noexcept;

        int getHeight() const noexcept;

        glm::ivec2 getSize() const noexcept;

        /// <summary>
        /// Check to see if the window is valid.
        /// </summary>
        explicit operator bool() const;

    private:
        std::unique_ptr<WindowImpl> pImpl;
    };
}