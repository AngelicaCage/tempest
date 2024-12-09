
RectI
window_get_rect(GLFWwindow *window)
{
    RectI result;
    glfwGetWindowPos(window, &result.x, &result.y);
    glfwGetWindowSize(window, &result.w, &result.h);
    return result;
}

Void
window_toggle_fullscreen(GLFWwindow *window, RectI windowed_rect, Bool *current_state)
{
    if(*current_state)
    {
        *current_state = false;
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        glfwSetWindowMonitor(window, NULL,
                             windowed_rect.x,
                             windowed_rect.y,
                             windowed_rect.w,
                             windowed_rect.h,
                             mode->refreshRate);
    }
    else
    {
        *current_state = true;
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    }
}