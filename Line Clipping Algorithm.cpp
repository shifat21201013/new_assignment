#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

// Window dimensions
const int WIDTH = 1200;
const int HEIGHT = 800;

// Data structures
struct Point {
    int x, y;
};

struct ClipWindow {
    int xmin, ymin, xmax, ymax;
};

// Global variables
ClipWindow clipWindow = {200, 200, 800, 600};
vector<pair<Point, Point>> clipLines;
Point tempClipLine = {-1, -1};

// ============= HELPER FUNCTIONS =============

void putPixel(int x, int y) {
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

// Bresenham's line for drawing
void bresenhamLine(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    int x = x1, y = y1;

    while (true) {
        putPixel(x, y);

        if (x == x2 && y == y2) break;

        int e2 = 2 * err;

        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }

        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

// Thick line for clipped portion
void bresenhamThickLine(int x1, int y1, int x2, int y2, int thickness) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    double length = sqrt(dx * dx + dy * dy);

    if (length == 0) return;

    double perpX = -dy / length;
    double perpY = dx / length;

    int halfThick = thickness / 2;
    for (int t = -halfThick; t <= halfThick; t++) {
        int offsetX = (int)(t * perpX);
        int offsetY = (int)(t * perpY);
        bresenhamLine(x1 + offsetX, y1 + offsetY, x2 + offsetX, y2 + offsetY);
    }
}

// ============= LIANG-BARSKY ALGORITHM =============

// Liang-Barsky Line Clipping Algorithm
// Uses parametric representation: P(u) = P1 + u(P2 - P1), where 0 <= u <= 1
bool liangBarskyClip(double x1, double y1, double x2, double y2,
                     double& cx1, double& cy1, double& cx2, double& cy2,
                     ClipWindow w) {

    // Calculate differences
    double dx = x2 - x1;
    double dy = y2 - y1;

    // Define p and q arrays for the 4 boundaries
    double p[4], q[4];

    // Left, Right, Bottom, Top edges
    p[0] = -dx;           // Moving left
    p[1] = dx;            // Moving right
    p[2] = -dy;           // Moving down
    p[3] = dy;            // Moving up

    q[0] = x1 - w.xmin;   // Distance from left edge
    q[1] = w.xmax - x1;   // Distance from right edge
    q[2] = y1 - w.ymin;   // Distance from bottom edge
    q[3] = w.ymax - y1;   // Distance from top edge

    // Initialize parameters
    double u1 = 0.0;  // Entry parameter
    double u2 = 1.0;  // Exit parameter

    // Process each edge
    for (int i = 0; i < 4; i++) {
        if (p[i] == 0) {
            // Line parallel to boundary
            if (q[i] < 0) {
                // Line is completely outside
                return false;
            }
        } else {
            double t = q[i] / p[i];

            if (p[i] < 0) {
                // Entry point (potentially entering)
                if (t > u2) return false;  // Line is outside
                if (t > u1) u1 = t;        // Update entry
            } else {
                // Exit point (potentially leaving)
                if (t < u1) return false;  // Line is outside
                if (t < u2) u2 = t;        // Update exit
            }
        }
    }

    // Calculate clipped coordinates
    cx1 = x1 + u1 * dx;
    cy1 = y1 + u1 * dy;
    cx2 = x1 + u2 * dx;
    cy2 = y1 + u2 * dy;

    return true;  // Line is visible (at least partially)
}

// ============= DISPLAY AND UI =============

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Title
    glColor3f(1.0, 1.0, 1.0);
    glRasterPos2i(10, HEIGHT - 20);
    string title = "Task 3: Liang-Barsky Line Clipping Algorithm";
    for (char c : title) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    // Instructions
    glRasterPos2i(10, HEIGHT - 40);
    string inst = "Click 2 points for line | Arrow keys move window | Gray=Original, Green=Clipped | C clear";
    for (char c : inst) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }

    // Cases info
    glRasterPos2i(10, HEIGHT - 60);
    string cases = "Handles: Fully inside, Fully outside, Partially intersecting | Red dots = clipped endpoints";
    for (char c : cases) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
    }

    // Draw clipping window (yellow rectangle)
    glColor3f(1.0, 1.0, 0.0);
    glLineWidth(3);
    glBegin(GL_LINE_LOOP);
    glVertex2i(clipWindow.xmin, clipWindow.ymin);
    glVertex2i(clipWindow.xmax, clipWindow.ymin);
    glVertex2i(clipWindow.xmax, clipWindow.ymax);
    glVertex2i(clipWindow.xmin, clipWindow.ymax);
    glEnd();
    glLineWidth(1);

    // Process and draw all lines
    for (auto& line : clipLines) {
        // Draw ORIGINAL LINE in gray (thin)
        glColor3f(0.5, 0.5, 0.5);
        bresenhamLine(line.first.x, line.first.y, line.second.x, line.second.y);

        // Apply Liang-Barsky clipping
        double cx1, cy1, cx2, cy2;
        bool isVisible = liangBarskyClip(
            line.first.x, line.first.y,
            line.second.x, line.second.y,
            cx1, cy1, cx2, cy2, clipWindow
        );

        if (isVisible) {
            // Line is at least partially visible
            // Draw CLIPPED PORTION in bright green (thick)
            glColor3f(0.0, 1.0, 0.0);
            bresenhamThickLine((int)cx1, (int)cy1, (int)cx2, (int)cy2, 3);

            // Mark clipped endpoints with red dots
            glColor3f(1.0, 0.0, 0.0);
            glPointSize(7);
            putPixel((int)cx1, (int)cy1);
            putPixel((int)cx2, (int)cy2);
            glPointSize(1);
        }
        // If not visible, only gray line is shown (fully outside case)
    }

    // Draw temporary point
    if (tempClipLine.x != -1) {
        glColor3f(1.0, 1.0, 0.0);
        glPointSize(8);
        putPixel(tempClipLine.x, tempClipLine.y);
        glPointSize(1);
    }

    glFlush();
    glutSwapBuffers();
}

// ============= INPUT HANDLING =============

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        y = HEIGHT - y;  // Convert to OpenGL coordinates

        if (tempClipLine.x == -1) {
            tempClipLine.x = x;
            tempClipLine.y = y;
        } else {
            Point p2 = {x, y};
            clipLines.push_back({tempClipLine, p2});
            tempClipLine = {-1, -1};
        }

        glutPostRedisplay();
    }
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'c':
        case 'C':
            clipLines.clear();
            tempClipLine = {-1, -1};
            break;
        case 27:  // ESC
            exit(0);
            break;
    }

    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    int step = 15;
    switch (key) {
        case GLUT_KEY_LEFT:
            clipWindow.xmin -= step;
            clipWindow.xmax -= step;
            break;
        case GLUT_KEY_RIGHT:
            clipWindow.xmin += step;
            clipWindow.xmax += step;
            break;
        case GLUT_KEY_UP:
            clipWindow.ymin += step;
            clipWindow.ymax += step;
            break;
        case GLUT_KEY_DOWN:
            clipWindow.ymin -= step;
            clipWindow.ymax -= step;
            break;
    }
    glutPostRedisplay();
}

// ============= INITIALIZATION =============

void init() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIDTH, 0, HEIGHT);
    glPointSize(1);
}

// ============= MAIN =============

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Task 3: Liang-Barsky Clipping");

    init();

    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);

    cout << "========================================" << endl;
    cout << "Task 3: Liang-Barsky Line Clipping" << endl;
    cout << "========================================" << endl;
    cout << "Algorithm: Parametric line clipping" << endl;
    cout << "Features:" << endl;
    cout << "  - Takes multiple line segments" << endl;
    cout << "  - Clips against rectangular window" << endl;
    cout << "  - Gray lines = Original" << endl;
    cout << "  - Green lines = Clipped portions" << endl;
    cout << "  - Red dots = Clipped endpoints" << endl;
    cout << "  - Handles all cases:" << endl;
    cout << "    * Fully inside" << endl;
    cout << "    * Fully outside" << endl;
    cout << "    * Partially intersecting" << endl;
    cout << "========================================" << endl;
    cout << "Controls:" << endl;
    cout << "  Click two points : Draw line" << endl;
    cout << "  Arrow keys       : Move window" << endl;
    cout << "  C                : Clear" << endl;
    cout << "  ESC              : Exit" << endl;
    cout << "========================================" << endl;

    glutMainLoop();
    return 0;
}
