#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <cmath>
#include <vector>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

struct Vector3 {
    float x, y, z;

    Vector3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

    Vector3 operator+(const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
    Vector3 operator-(const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
    Vector3 operator*(float s) const { return Vector3(x * s, y * s, z * s); }
    Vector3 operator/(float s) const { return Vector3(x / s, y / s, z / s); }

    float length() const { return std::sqrt(x * x + y * y + z * z); }
    Vector3 normalize() const {
        float len = length();
        return len > 0 ? *this / len : Vector3(0, 0, 0);
    }

    static float dot(const Vector3& a, const Vector3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static Vector3 cross(const Vector3& a, const Vector3& b) {
        return Vector3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }
};

// Camera for 3D movement
class Camera {
public:
    Vector3 position;
    Vector3 forward;
    Vector3 up;
    float yaw;
    float pitch;

    Camera(Vector3 pos) : position(pos), yaw(-90), pitch(0) {
        updateVectors();
    }

    void move(const Vector3& direction) {
        position = position + direction;
    }

    void rotate(float yawDelta, float pitchDelta) {
        yaw += yawDelta;
        pitch += pitchDelta;

        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        updateVectors();
    }

    Vector3 getRight() const {
        Vector3 worldUp(0, 1, 0);
        return Vector3::cross(forward, worldUp).normalize();
    }

    void applyCamera() {
        Vector3 center = position + forward;
        gluLookAt(position.x, position.y, position.z,
            center.x, center.y, center.z,
            up.x, up.y, up.z);
    }

private:
    void updateVectors() {
        float yawRad = yaw * 3.14159f / 180.0f;
        float pitchRad = pitch * 3.14159f / 180.0f;

        forward = Vector3(
            std::cos(pitchRad) * std::cos(yawRad),
            std::sin(pitchRad),
            std::cos(pitchRad) * std::sin(yawRad)
        ).normalize();

        Vector3 worldUp(0, 1, 0);
        Vector3 right = Vector3::cross(forward, worldUp).normalize();
        up = Vector3::cross(right, forward).normalize();
    }
};

// 3D Object structure
struct Object3D {
    Vector3 position;
    Vector3 size;
    float r, g, b;

    Object3D(Vector3 pos, Vector3 s, float red, float green, float blue)
        : position(pos), size(s), r(red), g(green), b(blue) {}
};

// Global variables
Camera* g_camera = nullptr;
std::vector<Object3D> g_objects;
bool g_keys[256] = { false };
HWND g_hwnd = nullptr;
HDC g_hdc = nullptr;
HGLRC g_hglrc = nullptr;
bool g_mouseLook = true;
int g_lastMouseX = 0;
int g_lastMouseY = 0;
bool g_firstMouse = true;
float g_mouseSensitivity = 0.1f;

// Draw a 3D cube
void drawCube(float size) {
    float s = size / 2.0f;
    glBegin(GL_QUADS);

    // Front face
    glNormal3f(0, 0, 1);
    glVertex3f(-s, -s, s);
    glVertex3f(s, -s, s);
    glVertex3f(s, s, s);
    glVertex3f(-s, s, s);

    // Back face
    glNormal3f(0, 0, -1);
    glVertex3f(-s, -s, -s);
    glVertex3f(-s, s, -s);
    glVertex3f(s, s, -s);
    glVertex3f(s, -s, -s);

    // Top face
    glNormal3f(0, 1, 0);
    glVertex3f(-s, s, -s);
    glVertex3f(-s, s, s);
    glVertex3f(s, s, s);
    glVertex3f(s, s, -s);

    // Bottom face
    glNormal3f(0, -1, 0);
    glVertex3f(-s, -s, -s);
    glVertex3f(s, -s, -s);
    glVertex3f(s, -s, s);
    glVertex3f(-s, -s, s);

    // Right face
    glNormal3f(1, 0, 0);
    glVertex3f(s, -s, -s);
    glVertex3f(s, s, -s);
    glVertex3f(s, s, s);
    glVertex3f(s, -s, s);

    // Left face
    glNormal3f(-1, 0, 0);
    glVertex3f(-s, -s, -s);
    glVertex3f(-s, -s, s);
    glVertex3f(-s, s, s);
    glVertex3f(-s, s, -s);

    glEnd();
}

// Draw grid floor
void drawGrid(int size, float spacing) {
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_LINES);
    for (int i = -size; i <= size; i++) {
        glVertex3f(i * spacing, 0, -size * spacing);
        glVertex3f(i * spacing, 0, size * spacing);
        glVertex3f(-size * spacing, 0, i * spacing);
        glVertex3f(size * spacing, 0, i * spacing);
    }
    glEnd();
}

// Initialize OpenGL
void initOpenGL(int width, int height) {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat lightPos[] = { 5.0f, 10.0f, 5.0f, 1.0f };
    GLfloat lightAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / (double)height, 0.1, 1000.0);
    glMatrixMode(GL_MODELVIEW);

    // Create scene objects
    g_objects.push_back(Object3D(Vector3(0, 1, 0), Vector3(2, 2, 2), 1, 0, 0));      // Red cube
    g_objects.push_back(Object3D(Vector3(5, 1, 5), Vector3(1.5f, 1.5f, 1.5f), 0, 0, 1));  // Blue cube
    g_objects.push_back(Object3D(Vector3(-5, 1, 5), Vector3(1, 1, 1), 0, 1, 0));     // Green cube
    g_objects.push_back(Object3D(Vector3(0, 1, 10), Vector3(1, 3, 1), 1, 1, 0));     // Yellow tall cube
    g_objects.push_back(Object3D(Vector3(5, 0.5f, -5), Vector3(2, 1, 2), 0, 1, 1));  // Cyan wide cube
    g_objects.push_back(Object3D(Vector3(-5, 1, -5), Vector3(1, 1, 1), 1, 0, 1));    // Magenta cube
}

// Render scene
void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    g_camera->applyCamera();

    // Draw grid
    drawGrid(20, 2.0f);

    // Draw all objects
    for (const auto& obj : g_objects) {
        glPushMatrix();
        glTranslatef(obj.position.x, obj.position.y, obj.position.z);
        glColor3f(obj.r, obj.g, obj.b);
        glScalef(obj.size.x, obj.size.y, obj.size.z);
        drawCube(1.0f);
        glPopMatrix();
    }

    SwapBuffers(g_hdc);
}

// Handle input
void handleInput(float deltaTime) {
    float moveSpeed = 5.0f * deltaTime;

    if (g_keys['W']) g_camera->move(g_camera->forward * moveSpeed);
    if (g_keys['S']) g_camera->move(g_camera->forward * -moveSpeed);
    if (g_keys['A']) g_camera->move(g_camera->getRight() * -moveSpeed);
    if (g_keys['D']) g_camera->move(g_camera->getRight() * moveSpeed);
    if (g_keys[VK_SPACE]) g_camera->move(Vector3(0, moveSpeed, 0));
    if (g_keys['Q']) g_camera->move(Vector3(0, -moveSpeed, 0));
}

// Handle mouse movement
void handleMouseMove(int x, int y) {
    if (!g_mouseLook) return;

    if (g_firstMouse) {
        g_lastMouseX = x;
        g_lastMouseY = y;
        g_firstMouse = false;
        return;
    }

    int deltaX = x - g_lastMouseX;
    int deltaY = g_lastMouseY - y;  // Reversed since y-coordinates go from bottom to top

    g_lastMouseX = x;
    g_lastMouseY = y;

    float yawChange = deltaX * g_mouseSensitivity;
    float pitchChange = deltaY * g_mouseSensitivity;

    g_camera->rotate(yawChange, pitchChange);

    // Recenter cursor to prevent edge issues
    RECT rect;
    GetClientRect(g_hwnd, &rect);
    int centerX = rect.right / 2;
    int centerY = rect.bottom / 2;

    POINT pt = { centerX, centerY };
    ClientToScreen(g_hwnd, &pt);
    SetCursorPos(pt.x, pt.y);

    g_lastMouseX = centerX;
    g_lastMouseY = centerY;
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        {
            PIXELFORMATDESCRIPTOR pfd = {
                sizeof(PIXELFORMATDESCRIPTOR), 1,
                PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
                PFD_TYPE_RGBA, 32,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                24, 8, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
            };

            g_hdc = GetDC(hwnd);
            int pixelFormat = ChoosePixelFormat(g_hdc, &pfd);
            SetPixelFormat(g_hdc, pixelFormat, &pfd);
            g_hglrc = wglCreateContext(g_hdc);
            wglMakeCurrent(g_hdc, g_hglrc);

            RECT rect;
            GetClientRect(hwnd, &rect);
            initOpenGL(rect.right, rect.bottom);
        }
        break;

    case WM_SIZE:
        {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            glViewport(0, 0, width, height);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluPerspective(45.0, (double)width / (double)height, 0.1, 1000.0);
            glMatrixMode(GL_MODELVIEW);
        }
        break;

    case WM_KEYDOWN:
        g_keys[wParam] = true;
        if (wParam == VK_ESCAPE) PostQuitMessage(0);
        break;

    case WM_KEYUP:
        g_keys[wParam] = false;
        break;

    case WM_MOUSEMOVE:
        {
            int xPos = LOWORD(lParam);
            int yPos = HIWORD(lParam);
            handleMouseMove(xPos, yPos);
        }
        break;

    case WM_ACTIVATE:
        if (LOWORD(wParam) != WA_INACTIVE && g_mouseLook) {
            ShowCursor(FALSE);
            // Center cursor
            RECT rect;
            GetClientRect(hwnd, &rect);
            POINT pt = { rect.right / 2, rect.bottom / 2 };
            ClientToScreen(hwnd, &pt);
            SetCursorPos(pt.x, pt.y);
            g_firstMouse = true;
        }
        else {
            ShowCursor(TRUE);
        }
        break;

    case WM_CLOSE:
        ShowCursor(TRUE);
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(g_hglrc);
        ReleaseDC(hwnd, g_hdc);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Main entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const wchar_t* className = L"OpenGL3DEnvironment";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClass(&wc);

    g_hwnd = CreateWindowEx(
        0, className, L"3D Environment - Mouse: Look | WASD: Move | Space: Up | Q: Down | ESC: Exit",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
        nullptr, nullptr, hInstance, nullptr
    );

    g_camera = new Camera(Vector3(0, 2, -10));

    // Hide cursor and center it
    ShowCursor(FALSE);
    RECT rect;
    GetClientRect(g_hwnd, &rect);
    g_lastMouseX = rect.right / 2;
    g_lastMouseY = rect.bottom / 2;
    POINT pt = { g_lastMouseX, g_lastMouseY };
    ClientToScreen(g_hwnd, &pt);
    SetCursorPos(pt.x, pt.y);

    MSG msg;
    DWORD lastTime = GetTickCount();

    while (true) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                delete g_camera;
                return 0;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        DWORD currentTime = GetTickCount();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        handleInput(deltaTime);
        renderScene();
    }

    return 0;
}
