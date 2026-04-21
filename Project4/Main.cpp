#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <cmath>
#include <vector>
#include <random>
#include <string>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

// Modern OpenGL function pointers
typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;

#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_PROGRAM_POINT_SIZE 0x8642

typedef void (WINAPI* PFNGLGENBUFFERSPROC)(GLsizei n, GLuint* buffers);
typedef void (WINAPI* PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void (WINAPI* PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
typedef void (WINAPI* PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint* buffers);
typedef GLuint(WINAPI* PFNGLCREATESHADERPROC)(GLenum type);
typedef void (WINAPI* PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
typedef void (WINAPI* PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void (WINAPI* PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint* params);
typedef void (WINAPI* PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef GLuint(WINAPI* PFNGLCREATEPROGRAMPROC)(void);
typedef void (WINAPI* PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void (WINAPI* PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void (WINAPI* PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint* params);
typedef void (WINAPI* PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void (WINAPI* PFNGLUSEPROGRAMPROC)(GLuint program);
typedef void (WINAPI* PFNGLDELETESHADERPROC)(GLuint shader);
typedef void (WINAPI* PFNGLDELETEPROGRAMPROC)(GLuint program);
typedef GLint(WINAPI* PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar* name);
typedef void (WINAPI* PFNGLUNIFORMMATRIX4FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (WINAPI* PFNGLUNIFORM3FPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (WINAPI* PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef void (WINAPI* PFNGLDISABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef void (WINAPI* PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
typedef void (WINAPI* PFNGLBUFFERSUBDATAPROC)(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
typedef void (WINAPI* PFNGLBINDATTRIBLOCATIONPROC)(GLuint program, GLuint index, const GLchar* name);
typedef GLint(WINAPI* PFNGLGETATTRIBLOCATIONPROC)(GLuint program, const GLchar* name);

PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC glBufferData = nullptr;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;
PFNGLCREATESHADERPROC glCreateShader = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
PFNGLATTACHSHADERPROC glAttachShader = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
PFNGLDELETESHADERPROC glDeleteShader = nullptr;
PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = nullptr;
PFNGLUNIFORM3FPROC glUniform3f = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
PFNGLBUFFERSUBDATAPROC glBufferSubData = nullptr;
PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation = nullptr;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation = nullptr;

// Random number generator
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(0.0f, 1.0f);

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

// Fire Particle structure
struct FireParticle {
    Vector3 position;
    Vector3 velocity;
    float life;
    float size;
    float r, g, b, a;

    FireParticle() : life(0), size(0), r(1), g(0.5f), b(0), a(1) {}

    void reset(Vector3 origin, float scale) {
        position = origin;
        position.x += (dis(gen) - 0.5f) * 0.5f * scale;
        position.z += (dis(gen) - 0.5f) * 0.5f * scale;

        velocity = Vector3(
            (dis(gen) - 0.5f) * 0.5f * scale,
            (1.0f + dis(gen) * 2.0f) * scale,
            (dis(gen) - 0.5f) * 0.5f * scale
        );

        life = 3.0f;  // Live for 3 seconds
        size = 0.1f + dis(gen) * 0.2f;

        // Start with bright yellow-orange
        r = 1.0f;
        g = 0.6f + dis(gen) * 0.4f;
        b = 0.0f;
        a = 0.8f;
    }

    void update(float deltaTime) {
        if (life <= 0) return;

        life -= deltaTime;
        position = position + velocity * deltaTime;
        velocity.y -= 0.5f * deltaTime; // Slight gravity

        // Fade to red then dark as it rises
        float lifeRatio = life / 3.0f;  // Normalize by max life
        r = 1.0f;
        g = lifeRatio * 0.5f;
        b = 0.0f;
        a = lifeRatio * 0.8f;

        size += deltaTime * 0.1f;
    }

    bool isAlive() const { return life > 0; }
};

// Global variables
Camera* g_camera = nullptr;
std::vector<Object3D> g_objects;
std::vector<FireParticle> g_fireParticles;
Vector3 g_firePosition(0, 0.1f, 0);
const int MAX_PARTICLES = 100000;  // 100k particles with GPU rendering!
bool g_keys[256] = { false };
HWND g_hwnd = nullptr;
HDC g_hdc = nullptr;
HGLRC g_hglrc = nullptr;
bool g_mouseLook = true;
int g_lastMouseX = 0;
int g_lastMouseY = 0;
bool g_firstMouse = true;
float g_mouseSensitivity = 0.1f;
float g_fireScale = 1.0f;  // Fire size multiplier (Up/Down arrows to adjust)

// GPU rendering variables
GLuint g_vbo = 0;
GLuint g_shaderProgram = 0;
GLint g_mvpLocation = -1;
GLint g_cameraPosLocation = -1;

// Vertex shader - runs on GPU for each particle
const char* vertexShaderSource = R"(
#version 120
attribute vec3 position;
attribute vec4 color;
attribute float pointSize;

uniform mat4 mvp;
uniform vec3 cameraPos;

varying vec4 vColor;

void main() {
    gl_Position = mvp * vec4(position, 1.0);

    // Distance-based point size attenuation
    float distance = length(position - cameraPos);
    gl_PointSize = pointSize * 600.0 / (1.0 + distance * 0.1);

    vColor = color;
}
)";

// Fragment shader - runs on GPU for each pixel
const char* fragmentShaderSource = R"(
#version 120
varying vec4 vColor;

void main() {
    // Make circular points
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);
    if (dist > 0.5) discard;

    // Soft edges
    float alpha = vColor.a * (1.0 - dist * 2.0);
    gl_FragColor = vec4(vColor.rgb, alpha);
}
)";

// Particle data structure for GPU
struct ParticleVertex {
    float x, y, z;      // position
    float r, g, b, a;   // color
    float size;         // point size
};

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

// Load OpenGL extensions
void loadOpenGLExtensions() {
    glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
    glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
    glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
    glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
    glUniform3f = (PFNGLUNIFORM3FPROC)wglGetProcAddress("glUniform3f");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
    glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glDisableVertexAttribArray");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
    glBufferSubData = (PFNGLBUFFERSUBDATAPROC)wglGetProcAddress("glBufferSubData");
    glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)wglGetProcAddress("glBindAttribLocation");
    glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)wglGetProcAddress("glGetAttribLocation");
}

// Compile shader
GLuint compileShader(const char* source, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        MessageBoxA(nullptr, infoLog, "Shader Compilation Error", MB_OK);
    }
    return shader;
}

// Create shader program
GLuint createShaderProgram() {
    GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    // Bind attribute locations BEFORE linking
    glBindAttribLocation(program, 0, "position");
    glBindAttribLocation(program, 1, "color");
    glBindAttribLocation(program, 2, "pointSize");

    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        MessageBoxA(nullptr, infoLog, "Shader Linking Error", MB_OK);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

// Initialize OpenGL
void initOpenGL(int width, int height) {
    loadOpenGLExtensions();

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

    // Enable point sprites for particles
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    // Create shader program
    g_shaderProgram = createShaderProgram();
    g_mvpLocation = glGetUniformLocation(g_shaderProgram, "mvp");
    g_cameraPosLocation = glGetUniformLocation(g_shaderProgram, "cameraPos");

    // Create VBO for particle data
    glGenBuffers(1, &g_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(ParticleVertex), nullptr, GL_DYNAMIC_DRAW);

    // Initialize fire particles
    g_fireParticles.resize(MAX_PARTICLES);
    for (auto& particle : g_fireParticles) {
        particle.reset(g_firePosition, g_fireScale);
    }
}

// Draw a billboard (always facing camera)
void drawBillboard(const Vector3& pos, float size, float r, float g, float b, float a) {
    glPushMatrix();
    glTranslatef(pos.x, pos.y, pos.z);

    // Billboard faces camera by using the camera's right and up vectors
    Vector3 right = g_camera->getRight();
    Vector3 up = g_camera->up;

    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex3f(-(right.x * size) - (up.x * size), -(right.y * size) - (up.y * size), -(right.z * size) - (up.z * size));
    glVertex3f((right.x * size) - (up.x * size), (right.y * size) - (up.y * size), (right.z * size) - (up.z * size));
    glVertex3f((right.x * size) + (up.x * size), (right.y * size) + (up.y * size), (right.z * size) + (up.z * size));
    glVertex3f(-(right.x * size) + (up.x * size), -(right.y * size) + (up.y * size), -(right.z * size) + (up.z * size));
    glEnd();

    glPopMatrix();
}

// Update fire particles
void updateFire(float deltaTime) {
    int particlesToSpawn = (int)(deltaTime * 20000); // Spawn rate - 20000 per second

    for (auto& particle : g_fireParticles) {
        if (particle.isAlive()) {
            particle.update(deltaTime);
        }
        else if (particlesToSpawn > 0) {
            particle.reset(g_firePosition, g_fireScale);
            particlesToSpawn--;
        }
    }
}

// Render fire - optimized immediate mode
void renderFire() {
    // Disable lighting for fire particles
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for fire glow
    glDepthMask(GL_FALSE); // Don't write to depth buffer

    glPointSize(30.0f); // Large fixed size for visibility

    // Batch render all particles in one glBegin/glEnd
    glBegin(GL_POINTS);
    for (const auto& particle : g_fireParticles) {
        if (particle.isAlive()) {
            glColor4f(particle.r, particle.g, particle.b, particle.a);
            glVertex3f(particle.position.x, particle.position.y, particle.position.z);
        }
    }
    glEnd();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

// Render scene
void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    g_camera->applyCamera();

    // Draw grid
    drawGrid(20, 2.0f);

    // Draw fire
    renderFire();

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

    // Up/Down arrows to adjust fire size
    if (g_keys[VK_UP]) {
        g_fireScale += 2.0f * deltaTime;  // Increase fire size
        if (g_fireScale > 5.0f) g_fireScale = 5.0f;  // Max 5x
    }
    if (g_keys[VK_DOWN]) {
        g_fireScale -= 2.0f * deltaTime;  // Decrease fire size
        if (g_fireScale < 0.1f) g_fireScale = 0.1f;  // Min 0.1x
    }
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
        0, className, L"3D Environment - Mouse: Look | WASD: Move | Space: Up | Q: Down | Up/Down: Fire Size | ESC: Exit",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
        nullptr, nullptr, hInstance, nullptr
    );

    // Camera starts at good vantage point facing fire at origin
    g_camera = new Camera(Vector3(0, 3, -8));
    g_camera->yaw = 0;   // Face forward (towards positive Z)
    g_camera->pitch = -10;  // Look slightly down at fire
    g_camera->updateVectors();

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
        updateFire(deltaTime);
        renderScene();
    }

    return 0;
}
