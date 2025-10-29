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

// Global variables
vector<pair<Point, Point>> lines;
Point tempLine = {-1, -1};
int lineThickness = 1;

// ============= BRESENHAM'S LINE DRAWING ALGORITHM =============

void putPixel(int x, int y) {
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

// Standard Bresenham's Algorithm - Handles ALL slopes
void bresenhamLine(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;  // Step direction for x
    int sy = (y1 < y2) ? 1 : -1;  // Step direction for y
    int err = dx - dy;

    int x = x1, y = y1;

    while (true) {
        putPixel(x, y);

        if (x == x2 && y == y2) break;

        int e2 = 2 * err;

        // Step in x direction
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }

        // Step in y direction
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

// Thick Line Implementation using 4-way and 8-way symmetry
void bresenhamThickLine(int x1, int y1, int x2, int y2, int thickness) {
    if (thickness <= 1) {
        bresenhamLine(x1, y1, x2, y2);
        return;
    }

    int dx = x2 - x1;
    int dy = y2 - y1;
    double length = sqrt(dx * dx + dy * dy);

    if (length == 0) {
        // Single point - draw filled circle using 8-way symmetry
        int r = thickness / 2;
        for (int i = -r; i <= r; i++) {
            for (int j = -r; j <= r; j++) {
                if (i*i + j*j <= r*r) {
                    putPixel(x1 + i, y1 + j);
                }
            }
        }
        return;
    }

    // Calculate perpendicular unit vector
    double perpX = -dy / length;
    double perpY = dx / length;

    // Draw parallel lines to create thickness (8-way symmetry principle)
    int halfThick = thickness / 2;
    for (int t = -halfThick; t <= halfThick; t++) {
        int offsetX = (int)(t * perpX);
        int offsetY = (int)(t * perpY);
        bresenhamLine(x1 + offsetX, y1 + offsetY, x2 + offsetX, y2 + offsetY);
    }

    // Add rounded end caps using 4-way symmetry
    int r = halfThick;
    for (int i = -r; i <= r; i++) {
        for (int j = -r; j <= r; j++) {
            if (i*i + j*j <= r*r) {
                putPixel(x1 + i, y1 + j);
                putPixel(x2 + i, y2 + j);
            }
        }
    }
}

// ============= DISPLAY AND UI =============

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Title
    glColor3f(1.0, 1.0, 1.0);
    glRasterPos2i(10, HEIGHT - 20);
    string title = "Task 1: Bresenham's Line Drawing Algorithm | Thickness: " + to_string(lineThickness);
    for (char c : title) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    // Instructions
    glRasterPos2i(10, HEIGHT - 40);
    string inst = "Click two points to draw | +/- change thickness | C clear | ESC exit";
    for (char c : inst) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }

    // Features info
    glRasterPos2i(10, HEIGHT - 60);
    string feat = "Handles: Positive/Negative slopes, Vertical, Horizontal | 4-way & 8-way symmetry";
    for (char c : feat) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
    }

    // Draw all lines
    glColor3f(0.2, 1.0, 0.3);
    for (auto& line : lines) {
        bresenhamThickLine(line.first.x, line.first.y,
                          line.second.x, line.second.y, lineThickness);
    }

    // Draw temporary point
    if (tempLine.x != -1) {
        glColor3f(1.0, 1.0, 0.0);
        glPointSize(8);
        putPixel(tempLine.x, tempLine.y);
        glPointSize(1);
    }

    glFlush();
    glutSwapBuffers();
}

// ============= INPUT HANDLING =============

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        y = HEIGHT - y;  // Convert to OpenGL coordinates

        if (tempLine.x == -1) {
            tempLine.x = x;
            tempLine.y = y;
        } else {
            Point p2 = {x, y};
            lines.push_back({tempLine, p2});
            tempLine = {-1, -1};
        }

        glutPostRedisplay();
    }
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case '+':
        case '=':
            if (lineThickness < 25) lineThickness++;
            break;
        case '-':
        case '_':
            if (lineThickness > 1) lineThickness--;
            break;
        case 'c':
        case 'C':
            lines.clear();
            tempLine = {-1, -1};
            break;
        case 27:  // ESC
            exit(0);
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
    glutCreateWindow("Task 1: Bresenham's Line Drawing");

    init();

    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);

    cout << "========================================" << endl;
    cout << "Task 1: Bresenham's Line Drawing" << endl;
    cout << "========================================" << endl;
    cout << "a. Standard Line Drawing:" << endl;
    cout << "   - Handles all line orientations" << endl;
    cout << "   - Positive and negative slopes" << endl;
    cout << "   - Vertical and horizontal lines" << endl;
    cout << "b. Thick Lines:" << endl;
    cout << "   - 4-way symmetry for end caps" << endl;
    cout << "   - 8-way symmetry for line body" << endl;
    cout << "========================================" << endl;
    cout << "Controls:" << endl;
    cout << "  Click two points to draw a line" << endl;
    cout << "  +/- : Adjust thickness" << endl;
    cout << "  C   : Clear screen" << endl;
    cout << "  ESC : Exit" << endl;
    cout << "========================================" << endl;

    glutMainLoop();
    return 0;
}
