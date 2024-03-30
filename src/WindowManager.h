#ifndef OPENGLRENDERER_H
#define OPENGLRENDERER_H

#define IMG_OUT "screenshot.bmp"
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000

#include <stdio.h>
#include <functional>
#include <unordered_map>

#include "IFrameRender.h"
#include "Timer.h"

using Callback = std::function<void()>;

class WindowManager {
private:

    Color * pixels;
    IFrameRender * renderer;
    GLFWwindow * window;

    char windowTitle[50]={0};
    
    std::unordered_map<uint16_t, Callback> actions;

    /// @brief Processes input events
    void ProcessInput();

    /// @brief Handles occuring errors
    void HandleErrors();

    /// @brief Update window data
    void UpdateWindow();

public:

    WindowManager( RenderingContext * _context );

    /// @brief Sets first 9 characters of window title
    /// @param _title 
    void SetWindowTitle(const char _title[9]);

    /// @brief Sets Rendering service and first 9 characters of window title
    /// @param _service 
    /// @param _name 
    void SetRenderingService(IFrameRender * _service, const char _name[9]);

    /// @brief Binds and callback to given key within window context
    /// @param _key 
    /// @param _callback 
    void BindAction(const uint16_t & _key, Callback _callback);

    /// @brief Checks if button is being pressed
    /// @param _key 
    /// @return true or false
    bool IsButtonPressed(const uint16_t & _key);

    /// @brief Updates content of window using rendering service
    void Update();

    /// @brief Determines if window should be closed
    /// @return true or false
    bool ShouldClose();

    /// @brief Dumps current window content to bmp file
    void DumpContent();

    /// @brief Closes window after completing all internal actions
    void Close();
    
    ~WindowManager();
};

#endif
