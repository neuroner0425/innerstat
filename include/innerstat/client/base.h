#ifndef INNERSTAT_CLIENT_BASE_H
#define INNERSTAT_CLIENT_BASE_H

#ifndef INNERSTAT_BEGIN_NAMESPACE
#  define INNERSTAT_BEGIN_NAMESPACE \
    namespace innerstat {           \
    inline namespace v1 {
#  define INNERSTAT_END_NAMESPACE \
    }                       \
    }
#endif

INNERSTAT_BEGIN_NAMESPACE

constexpr int GRID_SIZE = 40;

// 도형의 크기 조절 핸들 타입
enum class HandleType {
  None = -1,
  TopLeft,
  Top,
  TopRight,
  Left,
  Right,
  BottomLeft,
  Bottom,
  BottomRight,
  Body
};

class Shape;
// 도형과 핸들 타입 정보를 묶은 구조체
struct ShapeHandle {
  Shape* shape = nullptr;
  HandleType handle_type = HandleType::None;

  ShapeHandle() = default;
  ShapeHandle(Shape* shape_ptr, HandleType handle)
      : shape(shape_ptr), handle_type(handle) {}
};

// Area의 논리적 타입
enum class ShapeType {
  None = 0,
  PS,
  OS,
  VM,
  Container,
  Network
};

// 유저가 MainCanvas에서 수행할 수 있는 액션
enum class UserAction {
  None = 0,
  Connecting,
  Dragging,
  Resizing,
  Panning
};
INNERSTAT_END_NAMESPACE

#endif