#pragma once

#include <QColor>
#include <array>

enum class TetrominoType { None = 0, I, J, L, O, S, T, Z };

class Piece {
public:
  Piece();
  explicit Piece(TetrominoType type);

  TetrominoType getType() const { return m_type; }
  int getX() const { return m_x; }
  int getY() const { return m_y; }
  int getRotation() const { return m_rotation; }

  void setPosition(int x, int y) {
    m_x = x;
    m_y = y;
  }
  void move(int dx, int dy) {
    m_x += dx;
    m_y += dy;
  }
  void rotateClockwise();
  void rotateCounterClockwise();

  // Devuelve si hay bloque en la coordenada local (r, c) de la matriz 4x4
  bool isBlockAt(int row, int col, int rotation) const;
  bool isBlockAt(int row, int col) const {
    return isBlockAt(row, col, m_rotation);
  }

  // Paleta clásica pastel (7 colores)
  static QColor getColorForType(TetrominoType type);

  // Paleta de arena (4 colores de alto contraste para facilitar conexiones)
  static uint8_t getSandColorId(TetrominoType type);
  static QColor getSandColorForType(TetrominoType type);
  static QColor getSandColorById(uint8_t colorId);

  static Piece getRandomPiece();

private:
  TetrominoType m_type{TetrominoType::None};
  int m_x{0};
  int m_y{0};
  int m_rotation{0}; // 0 a 3
};