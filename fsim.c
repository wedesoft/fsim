#define GLEW_STATIC

#include <math.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GL/glut.h>

const char *vertexSource = "#version 300 es\n\
in mediump vec3 point;\n\
uniform mat4 model;\n\
uniform mat4 projection;\n\
void main()\n\
{\n\
  gl_Position = projection * model * vec4(point, 1);\n\
}";

const char *fragmentSource = "#version 300 es\n\
out mediump vec3 fragColor;\n\
void main()\n\
{\n\
	fragColor = vec3(1,0,0);\n\
}";

GLuint vao;
GLuint vbo;
GLuint program;

float angle = 0;


void projection(const char *target)
{
  float d = 1 / tan(90 / 2 * M_PI / 180);
  float n = 1.5;
  float f = 2.5;
  float a = (n + f) / (n - f);
  float b = 2 * n * f / (n - f);
  float columns[4][4] = {{d, 0, 0, 0}, {0, d, 0, 0}, {0, 0, a, -1}, {0, 0, b, 0}};
  glUniformMatrix4fv(glGetUniformLocation(program, target), 1, GL_FALSE, &columns[0][0]);
}

void rotation(const char *target, float angle)
{
  float sin_angle = sin(angle * M_PI / 180);
  float cos_angle = cos(angle * M_PI / 180);
  float columns[4][4] = {{cos_angle, 0, sin_angle, 0}, {0, 1, 0, 0}, {-sin_angle, 0, cos_angle, 0}, {0, 0, -2, 1}};
  glUniformMatrix4fv(glGetUniformLocation(program, target), 1, GL_FALSE, &columns[0][0]);
}

void onDisplay(void)
{
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(program);
  projection("projection");
  rotation("model", angle);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glDisableVertexAttribArray(0);

  glutSwapBuffers();
}

void onResize(int width, int height)
{
  glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}

void onKey(int key, int x, int y)
{
  switch (key) {
  case GLUT_KEY_LEFT:
    angle -= 1;
    break;
  case GLUT_KEY_RIGHT:
    angle += 1;
    break;
  default:
    return;
  };
  glutPostRedisplay();
}

void showErrors(const char *step, GLuint context)
{
  GLint result = GL_FALSE;
  int infoLength = 0;
  glGetShaderiv(context, GL_COMPILE_STATUS, &result);
  glGetShaderiv(context, GL_INFO_LOG_LENGTH, &infoLength);
  if (result == GL_FALSE && infoLength > 0) {
    char *buffer = malloc(infoLength);
    glGetShaderInfoLog(context, infoLength, NULL, buffer);
    fprintf(stderr, "%s: %s\n", step, buffer);
    free(buffer);
  };
}

GLfloat vertices[] = {
  -0.5f, -0.5f, 0.0f,
   0.5f, -0.5f, 0.0f,
  -0.5f,  0.5f, 0.0f,
   0.5f,  0.5f, 0.0f
};

int main(int argc, char** argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(320, 320);
  glutCreateWindow("Triangle");

  glewExperimental = 1;
  glewInit();

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexSource, NULL);
  glCompileShader(vertexShader);
  showErrors("Vertex shader", vertexShader);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
  glCompileShader(fragmentShader);
  showErrors("Fragment shader", fragmentShader);

  program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);
  showErrors("Link stage", program);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glutDisplayFunc(onDisplay);
  glutReshapeFunc(onResize);
  glutSpecialFunc(onKey);
  glutMainLoop();

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &vbo);

  glBindVertexArray(0);
  glDeleteVertexArrays(1, &vao);

  glDetachShader(program, vertexShader);
  glDetachShader(program, fragmentShader);
  glDeleteProgram(program);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  return 0;
}
