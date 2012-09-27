//========================================================================
// Video mode test
// Copyright (c) Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
//
// This test enumerates or verifies video modes
//
//========================================================================

#include <GL/glfw3.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "getopt.h"

static GLFWwindow window_handle = NULL;

enum Mode
{
    LIST_MODE,
    TEST_MODE
};

static void usage(void)
{
    printf("Usage: modes [-t]\n");
    printf("       modes -h\n");
}

static const char* format_mode(GLFWvidmode* mode)
{
    static char buffer[512];

    sprintf(buffer,
            "%i x %i x %i (%i %i %i)",
            mode->width, mode->height,
            mode->redBits + mode->greenBits + mode->blueBits,
            mode->redBits, mode->greenBits, mode->blueBits);

    buffer[sizeof(buffer) - 1] = '\0';
    return buffer;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void window_size_callback(GLFWwindow window, int width, int height)
{
    printf("Window resized to %ix%i\n", width, height);

    glViewport(0, 0, width, height);
}

static int window_close_callback(GLFWwindow window)
{
    window_handle = NULL;
    return GL_TRUE;
}

static void key_callback(GLFWwindow window, int key, int action)
{
    if (key == GLFW_KEY_ESCAPE)
    {
        glfwDestroyWindow(window);
        window_handle = NULL;
    }
}

static void list_modes(GLFWmonitor monitor)
{
    int count, i;
    GLFWvidmode mode;
    GLFWvidmode* modes = glfwGetVideoModes(monitor, &count);

    glfwGetVideoMode(monitor, &mode);

    printf("Name: %s\n", glfwGetMonitorString(monitor, GLFW_MONITOR_NAME));
    printf("Current mode: %s\n", format_mode(&mode));
    printf("Virtual position: %i %i\n",
           glfwGetMonitorParam(monitor, GLFW_MONITOR_SCREEN_POS_X),
           glfwGetMonitorParam(monitor, GLFW_MONITOR_SCREEN_POS_Y));
    printf("Physical size: %i x %i\n",
           glfwGetMonitorParam(monitor, GLFW_MONITOR_PHYSICAL_WIDTH),
           glfwGetMonitorParam(monitor, GLFW_MONITOR_PHYSICAL_HEIGHT));

    printf("Modes:\n");

    for (i = 0;  i < count;  i++)
    {
        printf("%3u: %s", (unsigned int) i, format_mode(modes + i));

        if (memcmp(&mode, modes + i, sizeof(GLFWvidmode)) == 0)
            printf(" (current mode)");

        putchar('\n');
    }
}

static void test_modes(GLFWmonitor monitor)
{
    int i, count;
    GLFWvidmode* modes = glfwGetVideoModes(monitor, &count);

    glfwSetWindowSizeCallback(window_size_callback);
    glfwSetWindowCloseCallback(window_close_callback);
    glfwSetKeyCallback(key_callback);

    for (i = 0;  i < count;  i++)
    {
        GLFWvidmode* mode = modes + i;
        GLFWvidmode current;

        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);

        printf("Testing mode %u on monitor %s: %s\n",
               (unsigned int) i,
               glfwGetMonitorString(monitor, GLFW_MONITOR_NAME),
               format_mode(mode));

        window_handle = glfwCreateWindow(mode->width, mode->height,
                                         "Video Mode Test",
                                         glfwGetPrimaryMonitor(),
                                         NULL);
        if (!window_handle)
        {
            printf("Failed to enter mode %u: %s\n",
                   (unsigned int) i,
                   format_mode(mode));
            continue;
        }

        glfwMakeContextCurrent(window_handle);
        glfwSwapInterval(1);

        glfwSetTime(0.0);

        while (glfwGetTime() < 5.0)
        {
            glClear(GL_COLOR_BUFFER_BIT);
            glfwSwapBuffers(window_handle);
            glfwPollEvents();

            if (!window_handle)
            {
                printf("User terminated program\n");
                exit(EXIT_SUCCESS);
            }
        }

        glGetIntegerv(GL_RED_BITS, &current.redBits);
        glGetIntegerv(GL_GREEN_BITS, &current.greenBits);
        glGetIntegerv(GL_BLUE_BITS, &current.blueBits);

        glfwGetWindowSize(window_handle, &current.width, &current.height);

        if (current.redBits != mode->redBits ||
            current.greenBits != mode->greenBits ||
            current.blueBits != mode->blueBits)
        {
            printf("*** Color bit mismatch: (%i %i %i) instead of (%i %i %i)\n",
                   current.redBits, current.greenBits, current.blueBits,
                   mode->redBits, mode->greenBits, mode->blueBits);
        }

        if (current.width != mode->width || current.height != mode->height)
        {
            printf("*** Size mismatch: %ix%i instead of %ix%i\n",
                   current.width, current.height,
                   mode->width, mode->height);
        }

        printf("Closing window\n");

        glfwDestroyWindow(window_handle);
        window_handle = NULL;
        glfwPollEvents();
    }
}

int main(int argc, char** argv)
{
    int ch, i, count, mode = LIST_MODE;
    GLFWmonitor* monitors;

    while ((ch = getopt(argc, argv, "th")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            case 't':
                mode = TEST_MODE;
                break;
            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    argc -= optind;
    argv += optind;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    monitors = glfwGetMonitors(&count);

    for (i = 0;  i < count;  i++)
    {
        if (mode == LIST_MODE)
            list_modes(monitors[i]);
        else if (mode == TEST_MODE)
            test_modes(monitors[i]);
    }

    exit(EXIT_SUCCESS);
}

