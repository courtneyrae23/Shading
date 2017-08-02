#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>

//include header file for glfw library so that we can use OpenGL
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <windows.h>



#ifdef _WIN32
static DWORD lastTime;
#else
static struct timeval lastTime;
#endif

#define PI 3.14159265 // Should be used from mathlib

using namespace std;

//****************************************************
// Global Variables
//****************************************************
GLfloat translation[3] = { 0.0f, 0.0f, 0.0f };
bool auto_strech = false;
int Width_global = 400;
int Height_global = 400;

std::vector<vector<float>> lights = std::vector<vector<float>>();
float ka[3] = { 0.0, 0.0, 0.0 };
float kd[3] = { 0.0, 0.0, 0.0 };
float ks[3] = { 0.0, 0.0, 0.0 };
float cartoon_rgb[3] = { 0.0, 0.0, 0.0 };
float cartoon_xyz[3] = { 0.0, 0.0, 0.0 };
bool cartoon = false;
float sp = 0.0;
float spu = 0.0;
float spv = 0.0;
bool asm_shade = false;

inline float sqr(float x) { return x*x; }


//****************************************************
// Simple init function
//****************************************************
void initializeRendering()
{
    glfwInit();
}


//****************************************************
// A routine to set a pixel by drawing a GL point.  This is not a
// general purpose routine as it assumes a lot of stuff specific to
// this example.
//****************************************************
void setPixel(float x, float y, GLfloat r, GLfloat g, GLfloat b) {
    glColor3f(r, g, b);
    glVertex2f(x + 0.5, y + 0.5);  // The 0.5 is to target pixel centers
                                   // Note: Need to check for gap bug on inst machines.
}

//****************************************************
// Keyboard inputs
//****************************************************
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch (key) {

    case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
    case GLFW_KEY_Q: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
    case GLFW_KEY_LEFT:
        if (action) translation[0] -= 0.01f * Width_global; break;
    case GLFW_KEY_RIGHT:
        if (action) translation[0] += 0.01f * Width_global; break;
    case GLFW_KEY_UP:
        if (action) translation[1] += 0.01f * Height_global; break;
    case GLFW_KEY_DOWN:
        if (action) translation[1] -= 0.01f * Height_global; break;
    case GLFW_KEY_F:
        if (action) auto_strech = !auto_strech; break;
    case GLFW_KEY_SPACE: break;

    default: break;
    }

}

// Vector a + Vector b // 
vector<float> sum(vector<float> a, vector<float> b) {
    return std::vector<float> {a[0] + b[0], a[1] + b[1], a[2] + b[2]};
}

vector<float> normalize(vector<float> vector) {
    float x = vector[0];
    float y = vector[1];
    float z = vector[2];
    float length = sqrt(sqr(x) + sqr(y) + sqr(z));

    if (length == 0.0) {
        return std::vector<float> {0.0, 0.0, 0.0};
    }

    return std::vector<float> {x / length, y / length, z / length};
}


vector<float> half_angle(vector<float> l, vector<float> v) {
    l = normalize(l);
    v = normalize(v);
    float x = l[0] + v[0];
    float y = l[1] + v[1];
    float z = l[2] + v[2];
    float length = sqrt(sqr(x) + sqr(y) + sqr(z));
    return std::vector<float> {x / length, y / length, z / length};
}

vector<float> cartoonShade(float *normal, bool edge) {

    if (edge) {
        return std::vector<float> {0.0, 0.0, 0.0};
    }

    std::vector<float> n = { normal[0], normal[1], normal[2] };
    std::vector<float> v = { 0.0, 0.0, 1.0 };
    n = normalize(n);

    std::vector<float> l = { cartoon_xyz[0] - n[0], cartoon_xyz[1] - n[1], cartoon_xyz[2] - n[2] };
    l = normalize(l);

    float dot_l_n = (l[0] * n[0]) + (l[1] * n[1]) + (l[2] * n[2]);

    float intensity[] = { 2.0, 2.0, 2.0 };

    // AMBIENT
    std::vector<float> ambient = { cartoon_rgb[0], cartoon_rgb[1], cartoon_rgb[2] };

    //DIFFUSE
    float diffuse_max = max(dot_l_n, (float) 0.0);
    std::vector<float> diffuse;
    if (diffuse_max > 0.0) {
        diffuse = { 0.2f, 0.2f, 0.2f };
    }
    else {
        diffuse = { 0.0f, 0.0f, 0.0f };
    }

    //SPECULAR
    std::vector<float> r = { ((2 * dot_l_n)*n[0] - l[0]), ((2 * dot_l_n)*n[1] - l[1]), ((2 * dot_l_n)*n[2] - l[2]) };
    float spec_max = pow(max((r[0] * v[0]) + (r[1] * v[1]) + (r[2] * v[2]), (float) 0.0), 200.0f);
    std::vector<float> specular;
    if (spec_max > 0.0) {
        specular = { 0.2f, 0.2f, 0.2f };
    }
    else {
        specular = { 0.0f, 0.0f, 0.0f };
    }

    std::vector<float> light_total = sum(ambient, sum(diffuse, specular));

    return light_total;
}


vector<float> addLight(float *light, float *normal) {

    // NORMALIZED NORMAL VECTOR
    std::vector<float> n = { normal[0], normal[1], normal[2] };
    n = normalize(n);

    // NORMALIZED LIGHT VECTOR
    std::vector<float> l = { light[0], light[1], light[2] };
    if (light[6]) { // Point light vector
        l = { l[0] - n[0], l[1] - n[1], l[2] - n[2] };
    }
    else { // Directional light vector
        l = { -l[0], -l[1], -l[2] };
    }
    l = normalize(l);

    // NORMALIZED VIEW (EYE) VECTOR
    std::vector<float> e = { 0.0, 0.0, 1.0 };

    // NORMALIZED HALF-ANGLE VECTOR
    std::vector<float> h = half_angle(l, e);

    // ANISOTROPY SPECULAR EXPONENT
    if (spv != 0.0 || spu != 0.0) {

        float dot_n_y = (0 * n[0]) + (1 * n[1]) + (0 * n[2]);
        std::vector<float> v = { 0 - (n[0] * dot_n_y), 1 - (n[1] * dot_n_y), 0 - (n[2] * dot_n_y) };
        std::vector<float> u = { v[1] * n[2] - v[2] * n[1], v[2] * n[0] - v[0] * n[2], v[0] * n[1] - n[0] * v[1] };
        v = normalize(v);
        u = normalize(u);
        float dot_h_u = (h[0] * u[0]) + (h[1] * u[1]) + (h[2] * u[2]);
        float dot_h_v = (h[0] * v[0]) + (h[1] * v[1]) + (h[2] * v[2]);
        float dot_h_n = (h[0] * n[0]) + (h[1] * n[1]) + (h[2] * n[2]);

        sp = (spu*dot_h_u*dot_h_u + spv*dot_h_v*dot_h_v) / (1 - (dot_h_n*dot_h_n));
    }

    float dot_l_n = (l[0] * n[0]) + (l[1] * n[1]) + (l[2] * n[2]);
    float intensity[] = { light[3], light[4], light[5] };

    // AMBIENT
    std::vector<float> ambient = { intensity[0] * ka[0], intensity[1] * ka[1], intensity[2] * ka[2] };

    //DIFFUSE
    float diffuse_max = max(dot_l_n, (float) 0.0);
    std::vector<float> diffuse = { diffuse_max * intensity[0] * kd[0], diffuse_max * intensity[1] * kd[1], diffuse_max * intensity[2] * kd[2] };

    //SPECULAR
    std::vector<float> r = normalize(std::vector<float> {((2 * dot_l_n)*n[0] - l[0]), ((2 * dot_l_n)*n[1] - l[1]), ((2 * dot_l_n)*n[2] - l[2])});
    float spec_max = pow(max((r[0] * e[0]) + (r[1] * e[1]) + (r[2] * e[2]), 0.0f), sp);
    std::vector<float> specular = { spec_max * intensity[0] * ks[0], spec_max * intensity[1] * ks[1], spec_max * intensity[2] * ks[2] };

    return sum(ambient, sum(diffuse, specular));
}

vector<float> addLightASM(float *light, float *normal) {

    // NORMALIZED NORMAL VECTOR
    std::vector<float> n = { normal[0], normal[1], normal[2] };
    n = normalize(n);

    // NORMALIZED LIGHT VECTOR
    std::vector<float> l = { light[0], light[1], light[2] };
    if (light[6]) { // Point light vector
        l = { l[0] - n[0], l[1] - n[1], l[2] - n[2] };
    }
    else { // Directional light vector
        l = { -l[0], -l[1], -l[2] };
    }
    l = normalize(l);

    // NORMALIZED VIEW (EYE) VECTOR
    std::vector<float> e = { 0.0, 0.0, 1.0 };

    // NORMALIZED HALF-ANGLE VECTOR
    std::vector<float> h = half_angle(l, e);

    // ANISOTROPY SPECULAR EXPONENT
    if (spv == 0.0  && spu == 0.0) {
        spv = sp;
        spu = sp;
    }

    float dot_n_y = (0 * n[0]) + (1 * n[1]) + (0 * n[2]);
    std::vector<float> v = { 0 - (n[0] * dot_n_y), 1 - (n[1] * dot_n_y), 0 - (n[2] * dot_n_y) };
    std::vector<float> u = { v[1] * n[2] - v[2] * n[1], v[2] * n[0] - v[0] * n[2], v[0] * n[1] - n[0] * v[1] };
    v = normalize(v);
    u = normalize(u);
    float dot_h_u = (h[0] * u[0]) + (h[1] * u[1]) + (h[2] * u[2]);
    float dot_h_v = (h[0] * v[0]) + (h[1] * v[1]) + (h[2] * v[2]);
    float dot_h_n = (h[0] * n[0]) + (h[1] * n[1]) + (h[2] * n[2]);

    sp = (spu*dot_h_u*dot_h_u + spv*dot_h_v*dot_h_v) / (1 - (dot_h_n*dot_h_n));

    float dot_l_n = (l[0] * n[0]) + (l[1] * n[1]) + (l[2] * n[2]);
    float dot_e_n = (e[0] * n[0]) + (e[1] * n[1]) + (e[2] * n[2]);
    float dot_e_h = (e[0] * h[0]) + (e[1] * h[1]) + (e[2] * h[2]);
    float intensity[] = { light[3], light[4], light[5] };

    //DIFFUSE
    float d_term1 = 28 / (23 * PI);
    float d_term2 = 1 - pow(1 - (dot_e_n / 2), 5);
    float d_term3 = 1 - pow(1 - (dot_l_n / 2), 5);
    std::vector<float> diffuse = { d_term1*kd[0] * (1 - ks[0])*d_term2*d_term3, d_term1*kd[1] * (1 - ks[1])*d_term2*d_term3, d_term1*kd[2] * (1 - ks[2])*d_term2*d_term3 };


    //SPECULAR
    float s_term1 = (sqrt((spu + 1)*(spv + 1))) / (8 * PI);
    float s_term2 = (pow(dot_h_n, sp)) / (dot_e_h * max(dot_l_n, dot_e_n));
    std::vector<float> Fresnel = { ks[0] + (1.0f - ks[0])*pow((1.0f - dot_e_h), 5.0f), ks[1] + (1.0f - ks[1])*pow((1.0f - dot_e_h), 5.0f), ks[2] + (1.0f - ks[2])*pow((1.0f - dot_e_h), 5.0f) };
    std::vector<float> specular = { Fresnel[0] * s_term1*s_term2, Fresnel[1] * s_term1*s_term2, Fresnel[2] * s_term1*s_term2 };

    std::vector<float> sum = {ka[0] + diffuse[0] + specular[0], ka[1] + diffuse[1] + specular[1], ka[2] + diffuse[2] + specular[2] };
    return std::vector<float> {sum[0] * intensity[0], sum[1] * intensity[1], sum[2] * intensity[2]};
}


//****************************************************
// Draw a filled circle.
//****************************************************
void drawCircle(float centerX, float centerY, float radius) {
    // Draw inner circle
    glBegin(GL_POINTS);

    // We could eliminate wasted work by only looping over the pixels
    // inside the sphere's radius.  But the example is more clear this
    // way.  In general drawing an object by loopig over the whole
    // screen is wasteful.

    int minI = max(0, (int)floor(centerX - radius));
    int maxI = min(Width_global - 1, (int)ceil(centerX + radius));

    int minJ = max(0, (int)floor(centerY - radius));
    int maxJ = min(Height_global - 1, (int)ceil(centerY + radius));

    for (int i = 0; i < Width_global; i++) {
        for (int j = 0; j < Height_global; j++) {

            // Location of the center of pixel relative to center of sphere
            float x = (i + 0.5 - centerX);
            float y = (j + 0.5 - centerY);

            float dist = sqrt(sqr(x) + sqr(y));

            if (dist <= radius) {

                // This is the front-facing Z coordinate
                float z = sqrt(radius*radius - dist*dist);

                float normal[] = { x, y, z };
                std::vector<float> total_color = { 0.0, 0.0, 0.0 };

                if (cartoon) {
                    total_color = cartoonShade(normal, (dist > (radius - 5.0)));
                }
                else {
                    vector< vector<float> >::iterator row;
                    vector<float>::iterator col;
                    for (row = lights.begin(); row != lights.end(); row++) {
                        int i = 0;
                        float light[7];
                        for (col = row->begin(); col != row->end(); col++) {
                            light[i] = *col;
                            i++;
                        }
                        if (asm_shade) {
                            total_color = sum(total_color, addLightASM(light, normal));
                        }
                        else {
                            total_color = sum(total_color, addLight(light, normal));
                        }
                    }
                }


                setPixel(i, j, total_color[0], total_color[1], total_color[2]);

                // This is amusing, but it assumes negative color values are treated reasonably.
                //setPixel(i,j, x/radius, y/radius, z/radius );

                // Just for fun, an example of making the boundary pixels yellow.
                // if (dist > (radius-1.0)) {
                //     setPixel(i, j, 1.0, 1.0, 0.0);
                // }
            }
        }
    }

    glEnd();
}


//****************************************************
// function that does the actual drawing of stuff
//***************************************************
void display(GLFWwindow* window)
{
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f); //clear background screen to black

    glClear(GL_COLOR_BUFFER_BIT);                // clear the color buffer (sets everything to black)

    glMatrixMode(GL_MODELVIEW);                  // indicate we are specifying camera transformations
    glLoadIdentity();                            // make sure transformation is "zero'd"

                                                 //----------------------- code to draw objects --------------------------
    glPushMatrix();
    glTranslatef(translation[0], translation[1], translation[2]);
    drawCircle(Width_global / 2.0, Height_global / 2.0, min(Width_global, Height_global) * 0.9 / 2.0);
    glPopMatrix();

    glfwSwapBuffers(window);

}

//****************************************************
// function that is called when window is resized
//***************************************************
void size_callback(GLFWwindow* window, int width, int height)
{
    // Get the pixel coordinate of the window
    // it returns the size, in pixels, of the framebuffer of the specified window
    glfwGetFramebufferSize(window, &Width_global, &Height_global);

    glViewport(0, 0, Width_global, Height_global);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, Width_global, 0, Height_global, 1, -1);

    display(window);
}


//****************************************************
// the usual stuff, nothing exciting here
//****************************************************
int main(int argc, char *argv[]) {
    //This initializes glfw
    initializeRendering();

    int i = 1;
    int n;
    string valid_commands[] = { "-ka", "-kd", "-ks", "-spu", "-spv", "-sp", "-pl", "-dl", "-cartoon", "-asm" };

    while (i < argc) {
        if (argv[i] == valid_commands[0]) {
            for (n = 1; n <= 3; n++) {
                if (i + n >= argc) {
                    cerr << "-ka requires 3 input arguments" << endl;
                    glfwTerminate();
                    return -1;
                }
                else {
                    if ((atof(argv[i + n]) < 0.0) || (atof(argv[i + n]) > 1.0)) {
                        cerr << "-ka requires input between 0.0 and 1.0" << endl;
                        glfwTerminate();
                        return -1;
                    }
                    ka[n - 1] = atof(argv[i + n]);
                }
            }
            i += 4;
        }
        else if (argv[i] == valid_commands[1]) {
            for (n = 1; n <= 3; n++) {
                if (i + n >= argc) {
                    cerr << "-kd requires 3 input arguments" << endl;
                    glfwTerminate();
                    return -1;
                }
                else {
                    if ((atof(argv[i + n]) < 0.0) || (atof(argv[i + n]) > 1.0)) {
                        cerr << "-kd requires input between 0.0 and 1.0" << endl;
                        glfwTerminate();
                        return -1;
                    }
                    kd[n - 1] = atof(argv[i + n]);
                    cout << kd[n - 1] << endl;
                }
            }
            i += 4;
        }
        else if (argv[i] == valid_commands[2]) {
            for (n = 1; n <= 3; n++) {
                if (i + n >= argc) {
                    cerr << "-ks requires 3 input arguments" << endl;
                    glfwTerminate();
                    return -1;
                }
                else {
                    if ((atof(argv[i + n]) < 0.0) || (atof(argv[i + n]) > 1.0)) {
                        cerr << "-ks requires input between 0.0 and 1.0" << endl;
                        glfwTerminate();
                        return -1;
                    }
                    ks[n - 1] = atof(argv[i + n]);
                }
            }
            i += 4;
        }
        else if (argv[i] == valid_commands[3]) {
            n = 1;
            if (i + n >= argc) {
                cerr << "-spu requires 1 input argument" << endl;
                glfwTerminate();
                return -1;
            }
            else {
                if (atof(argv[i + n]) < 0.0) {
                    cerr << "-spu must be at least 0.0" << endl;
                    glfwTerminate();
                    return -1;
                }
                spu = atof(argv[i + n]);
            }
            i += 2;
        }
        else if (argv[i] == valid_commands[4]) {
            n = 1;
            if (i + n >= argc) {
                cerr << "-spv requires 1 input argument" << endl;
                glfwTerminate();
                return -1;
            }
            else {
                if (atof(argv[i + n]) < 0.0) {
                    cerr << "-spv must be at least 0.0" << endl;
                    glfwTerminate();
                    return -1;
                }
                spv = atof(argv[i + n]);
            }
            i += 2;
        }
        else if (argv[i] == valid_commands[5]) {
            n = 1;
            if (i + n >= argc) {
                cerr << "-sp requires 1 input argument" << endl;
                glfwTerminate();
                return -1;
            }
            else {
                if (atof(argv[i + n]) < 0.0) {
                    cerr << "-sp must be at least 0.0" << endl;
                    glfwTerminate();
                    return -1;
                }
                sp = atof(argv[i + n]);
            }
            i += 2;
        }
        else if (argv[i] == valid_commands[6]) {
            std::vector<float> pt_light;
            for (n = 1; n <= 6; n++) {
                if (i + n >= argc) {
                    cerr << "-pl requires 6 input arguments" << endl;
                    glfwTerminate();
                    return -1;
                }
                else {
                    if (n > 3 && (atof(argv[i + n]) < 0.0)) {
                        cerr << "-pl requires rgb values be at least 0.0" << endl;
                        glfwTerminate();
                        return -1;
                    }
                    pt_light.push_back(atof(argv[i + n]));
                }
            }
            pt_light.push_back(1.0);
            lights.push_back(pt_light);
            i += 7;
        }
        else if (argv[i] == valid_commands[7]) {
            std::vector<float> dir_light;
            for (n = 1; n <= 6; n++) {
                if (i + n >= argc) {
                    cerr << "-dl requires 6 input arguments" << endl;
                    glfwTerminate();
                    return -1;
                }
                else {
                    if (n > 3 && (atof(argv[i + n]) < 0.0)) {
                        cerr << "-dl requires rgb values be at least 0.0" << endl;
                        glfwTerminate();
                        return -1;
                    }
                    dir_light.push_back(atof(argv[i + n]));
                }
            }
            dir_light.push_back(0.0);
            lights.push_back(dir_light);
            i += 7;
        }
        else if (argv[i] == valid_commands[8]) {
            for (n = 1; n <= 6; n++) {
                if (i + n >= argc) {
                    cerr << "-cartoon requires 6 input arguments" << endl;
                    glfwTerminate();
                    return -1;
                }
                else {
                    if (n <= 3) {
                        cartoon_xyz[n - 1] = atof(argv[i + n]);
                    }
                    else {
                        if (atof(argv[i + n]) < 0.0 || atof(argv[i + n]) > 1.0) {
                            cerr << "-cartoon requires rgb values be between 0.0 and 1.0" << endl;
                            glfwTerminate();
                            return -1;
                        }
                        cartoon_rgb[n - 4] = atof(argv[i + n]);
                    }
                }
            }
            cartoon = true;
            i += 7;
        }
        else if (argv[i] == valid_commands[9]) {
            asm_shade = true;
            i += 1;
        }
        else {
            cerr << argv[i] << " not a valid command" << endl;
            glfwTerminate();
            return -1;
        }
    }

    GLFWwindow* window = glfwCreateWindow(Width_global, Height_global, "CS184", NULL, NULL);
    if (!window)
    {
        cerr << "Error on window creating" << endl;
        glfwTerminate();
        return -1;
    }

    const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if (!mode)
    {
        cerr << "Error on getting monitor" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Get the pixel coordinate of the window
    // it returns the size, in pixels, of the framebuffer of the specified window
    glfwGetFramebufferSize(window, &Width_global, &Height_global);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, Width_global, 0, Height_global, 1, -1);

    glfwSetWindowTitle(window, "CS184");
    glfwSetWindowSizeCallback(window, size_callback);
    glfwSetKeyCallback(window, key_callback);

    while (!glfwWindowShouldClose(window)) // infinite loop to draw object again and again
    {   // because once object is draw then window is terminated

        display(window);

        if (auto_strech) {
            glfwSetWindowSize(window, mode->width, mode->height);
            glfwSetWindowPos(window, 0, 0);
        }

        glfwPollEvents();

    }

    return 0;
}
