#include "terrain.h"

#include <math.h>
#include "gl/shaders/ShaderAttribLocations.h"

Terrain::Terrain() : m_numRows(100), m_numCols(m_numRows)
{
}


/**
 * Returns a pseudo-random value between -1.0 and 1.0 for the given row and column.
 */
float Terrain::randValue(int row, int col) {
    return -1.0 + 2.0 * glm::fract(sin(row * 127.1f + col * 311.7f) * 43758.5453123f);
}


/**
 * Returns the object-space position for the terrain vertex at the given row and column.
 */
glm::vec3 Terrain::getPosition(int row, int col) {
    glm::vec3 position;
    position.x = 10 * row/m_numRows - 5;
    position.y = 0;
    position.z = 10 * col/m_numCols - 5;

    // TODO: Adjust position.y using value noise.
    int freq = 5;
    float amp = 1.0;
    for (int i = 0; i < 3; ++i) {
        int nRows = m_numRows / freq, nCols = m_numCols / freq;
        float x_frac = glm::fract(col / static_cast<float>(nCols)), y_frac = glm::fract(row / static_cast<float>(nRows));
        x_frac = 3*x_frac*x_frac - 2*x_frac*x_frac*x_frac;
        y_frac = 3*y_frac*y_frac - 2*y_frac*y_frac*y_frac;
        float yA = randValue(row / nRows, col / nCols);
        float yB = randValue(row / nRows, col / nCols + 1);
        float yC = randValue(row / nRows + 1, col / nCols);
        float yD = randValue(row / nRows + 1, col / nCols + 1);
        position.y += amp * glm::mix(glm::mix(yA, yB, x_frac), glm::mix(yC, yD, x_frac), y_frac);
        freq *= 2;
        amp /= glm::pow(2.0f, i + 1.0f);
    }
    return position;
}


/**
 * Returns the normal vector for the terrain vertex at the given row and column.
 */
glm::vec3 Terrain::getNormal(int row, int col) {
    // TODO: Compute the normal at the given row and column using the positions of the
    //       neighboring vertices.

    glm::vec3 p = getPosition(row, col),
            n0 = getPosition(row, col + 1),
            n1 = getPosition(row + 1, col + 1),
            n2 = getPosition(row + 1, col),
            n3 = getPosition(row + 1, col - 1),
            n4 = getPosition(row, col - 1),
            n5 = getPosition(row - 1, col - 1),
            n6 = getPosition(row - 1, col),
            n7 = getPosition(row - 1, col + 1);

    glm::vec3 norm(0, 0, 0);
    norm += glm::normalize(glm::cross(n0 - p, n1 - p));
    norm += glm::normalize(glm::cross(n1 - p, n2 - p));
    norm += glm::normalize(glm::cross(n2 - p, n3 - p));
    norm += glm::normalize(glm::cross(n3 - p, n4 - p));
    norm += glm::normalize(glm::cross(n4 - p, n5 - p));
    norm += glm::normalize(glm::cross(n5 - p, n6 - p));
    norm += glm::normalize(glm::cross(n6 - p, n7 - p));
    norm += glm::normalize(glm::cross(n7 - p, n0 - p));

    return glm::normalize(norm);
}


/**
 * Initializes the terrain by storing positions and normals in a vertex buffer.
 */
void Terrain::init() {
    // TODO: Change from GL_LINE to GL_FILL in order to render full triangles instead of wireframe.
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


    // Initializes a grid of vertices using triangle strips.
    int numVertices = (m_numRows - 1) * (2 * m_numCols + 2);
    std::vector<glm::vec3> data(2 * numVertices);
    int index = 0;
    for (int row = 0; row < m_numRows - 1; row++) {
        for (int col = m_numCols - 1; col >= 0; col--) {
            data[index++] = getPosition(row, col);
            data[index++] = getNormal  (row, col);
            data[index++] = getPosition(row + 1, col);
            data[index++] = getNormal  (row + 1, col);
        }
        data[index++] = getPosition(row + 1, 0);
        data[index++] = getNormal  (row + 1, 0);
        data[index++] = getPosition(row + 1, m_numCols - 1);
        data[index++] = getNormal  (row + 1, m_numCols - 1);
    }

    // Initialize OpenGLShape.
    m_shape = std::make_unique<OpenGLShape>();
    m_shape->setVertexData(&data[0][0], data.size() * 3, VBO::GEOMETRY_LAYOUT::LAYOUT_TRIANGLE_STRIP, numVertices);
    m_shape->setAttribute(ShaderAttrib::POSITION, 3, 0, VBOAttribMarker::DATA_TYPE::FLOAT, false);
    m_shape->setAttribute(ShaderAttrib::NORMAL, 3, sizeof(glm::vec3), VBOAttribMarker::DATA_TYPE::FLOAT, false);
    m_shape->buildVAO();
}


/**
 * Draws the terrain.
 */
void Terrain::draw()
{
    m_shape->draw();
}
