#include "Piece.h"
#include <QRandomGenerator>

namespace Shapes {
using ShapeGrid = std::array<std::array<int, 4>, 4>;

// Tipo I
constexpr ShapeGrid I = {
    {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}}};

// Tipo J
constexpr ShapeGrid J = {
    {{1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}};

// Tipo L
constexpr ShapeGrid L = {
    {{0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}};

// Tipo O
constexpr ShapeGrid O = {
    {{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}};

// Tipo S
constexpr ShapeGrid S = {
    {{0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}};

// Tipo T
constexpr ShapeGrid T = {
    {{0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}};

// Tipo Z
constexpr ShapeGrid Z = {
    {{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}};
} // namespace Shapes

Piece::Piece() : Piece(TetrominoType::None) {}

Piece::Piece(TetrominoType type)
    : m_type(type), m_x(3), m_y(0), m_rotation(0) {}

void Piece::rotateClockwise() { m_rotation = (m_rotation + 1) % 4; }

void Piece::rotateCounterClockwise() { m_rotation = (m_rotation + 3) % 4; }

bool Piece::isBlockAt(int row, int col, int rotation) const {
  if (m_type == TetrominoType::None || row < 0 || row >= 4 || col < 0 ||
      col >= 4) {
    return false;
  }

  const Shapes::ShapeGrid *baseShape = nullptr;
  switch (m_type) {
  case TetrominoType::I:
    baseShape = &Shapes::I;
    break;
  case TetrominoType::J:
    baseShape = &Shapes::J;
    break;
  case TetrominoType::L:
    baseShape = &Shapes::L;
    break;
  case TetrominoType::O:
    baseShape = &Shapes::O;
    break;
  case TetrominoType::S:
    baseShape = &Shapes::S;
    break;
  case TetrominoType::T:
    baseShape = &Shapes::T;
    break;
  case TetrominoType::Z:
    baseShape = &Shapes::Z;
    break;
  default:
    return false;
  }

  int srcRow = row;
  int srcCol = col;
  switch (rotation % 4) {
  case 0:
    srcRow = row;
    srcCol = col;
    break;
  case 1:
    srcRow = 3 - col;
    srcCol = row;
    break;
  case 2:
    srcRow = 3 - row;
    srcCol = 3 - col;
    break;
  case 3:
    srcRow = col;
    srcCol = 3 - row;
    break;
  }

  return (*baseShape)[srcRow][srcCol] == 1;
}

// Paleta de colores pastel suaves y modernos (sin gradientes)
QColor Piece::getColorForType(TetrominoType type) {
  switch (type) {
  case TetrominoType::I:
    return QColor(100, 215, 230); // Celeste / Menta pastel
  case TetrominoType::J:
    return QColor(135, 175, 245); // Azul cielo pastel
  case TetrominoType::L:
    return QColor(255, 180, 130); // Naranja / Durazno pastel
  case TetrominoType::O:
    return QColor(255, 225, 130); // Amarillo suave pastel
  case TetrominoType::S:
    return QColor(150, 220, 175); // Verde salvia pastel
  case TetrominoType::T:
    return QColor(205, 155, 245); // Lavanda / Lila pastel
  case TetrominoType::Z:
    return QColor(250, 140, 160); // Rosa / Coral pastel
  default:
    return QColor(45, 48, 60);
  }
}

Piece Piece::getRandomPiece() {
  int randomInt = QRandomGenerator::global()->bounded(1, 8);
  return Piece(static_cast<TetrominoType>(randomInt));
}