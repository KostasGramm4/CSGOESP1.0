#include <GLFW/glfw3.h>
#include "process.h"
#include "helper.h"
#include "offsets.h"

int main(){

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_FLOATING, true);
    glfwWindowHint(GLFW_MAXIMIZED, true);
    glfwWindowHint(GLFW_RESIZABLE, false);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, true);
    glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, true);


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1920, 1080, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }


    int procId = GetProcessId(L"csgo.exe");

    if (!procId)
    {
        std::cout << "Process not found" << std::endl;
    }

    uintptr_t moduleBase = GetModuleBaseAddress(procId, L"client.dll");

    uintptr_t localPlayer = moduleBase + offsets::aLocalPlayer;

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, procId);

    std::vector<unsigned int>teamOffsets{ offsets::oTeamNum };
    uintptr_t teamAddr = FindDMAAddy(hProc, localPlayer, teamOffsets);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        ViewMatrix Matrix = Memory::Read<ViewMatrix>(hProc, (moduleBase + offsets::aViewMatrix));

        for (int i = 0; i < 64; ++i)
        {
            uintptr_t Entity = Memory::Read<uintptr_t>(hProc, (moduleBase + offsets::aEntityList) + i * 0x10);

            if (Entity == NULL) continue;

            uintptr_t Health = Memory::Read<uintptr_t>(hProc, (Entity + offsets::oHealth));

            if (Health <= 0) continue;

            uintptr_t EntityTeam = Memory::Read<uintptr_t>(hProc, Entity + offsets::oTeamNum);

            uintptr_t LocalTeam = Memory::Read<uintptr_t>(hProc, teamAddr);

            Vec3 vecOrigin = Memory::Read<Vec3>(hProc, Entity + offsets::oVecOrigin);

            Vec3 headOrigin;
            headOrigin.X = vecOrigin.X;
            headOrigin.Y = vecOrigin.Y;
            headOrigin.Z = vecOrigin.Z + 75.0f;

            Vec2 HeadCoords;
            Vec2 FeetCoords;

            if (!WorldToScreen(vecOrigin, FeetCoords, Matrix.Matrix)) continue;

            if (!WorldToScreen(headOrigin, HeadCoords, Matrix.Matrix)) continue;

            float height = HeadCoords.Y - FeetCoords.Y;
            float width = height / 3.8f;

            if (EntityTeam != LocalTeam)
            {
                glBegin(GL_LINES);
                glColor3f(1.0f, 0.0f, 0.0f);
                glVertex2f(0.0f, -1.0f);
                glVertex2f(FeetCoords.X, FeetCoords.Y);
                glEnd();

                glBegin(GL_LINES);
                glVertex2f(FeetCoords.X - (width / 2), FeetCoords.Y);
                glVertex2f(FeetCoords.X - (width / 2), HeadCoords.Y);
                glEnd();

                glBegin(GL_LINES);
                glVertex2f(FeetCoords.X - (width / 2), HeadCoords.Y);
                glVertex2f(FeetCoords.X + (width / 2), HeadCoords.Y);
                glEnd();

                glBegin(GL_LINES);
                glVertex2f(FeetCoords.X + (width / 2), HeadCoords.Y);
                glVertex2f(FeetCoords.X + (width / 2), FeetCoords.Y);
                glEnd();

                glBegin(GL_LINES);
                glVertex2f(FeetCoords.X + (width / 2), FeetCoords.Y);
                glVertex2f(FeetCoords.X - (width / 2), FeetCoords.Y);
                glEnd();
            }
            else
            {
                glBegin(GL_LINES);
                glColor3f(0.0f, 1.0f, 0.0f);
                glVertex2f(0.0f, -1.0f);
                glVertex2f(FeetCoords.X, FeetCoords.Y);
                glEnd();

                glBegin(GL_LINES);
                glVertex2f(FeetCoords.X - (width / 2), FeetCoords.Y);
                glVertex2f(FeetCoords.X - (width / 2), HeadCoords.Y);
                glEnd();

                glBegin(GL_LINES);
                glVertex2f(FeetCoords.X - (width / 2), HeadCoords.Y);
                glVertex2f(FeetCoords.X + (width / 2), HeadCoords.Y);
                glEnd();

                glBegin(GL_LINES);
                glVertex2f(FeetCoords.X + (width / 2), HeadCoords.Y);
                glVertex2f(FeetCoords.X + (width / 2), FeetCoords.Y);
                glEnd();

                glBegin(GL_LINES);
                glVertex2f(FeetCoords.X + (width / 2), FeetCoords.Y);
                glVertex2f(FeetCoords.X - (width / 2), FeetCoords.Y);
                glEnd();
            }
        }

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}