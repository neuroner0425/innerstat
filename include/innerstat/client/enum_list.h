#pragma once

class Shape;

/** @brief 도형의 크기 조절을 위한 핸들 타입 정의 */
enum class HandleType {
    None = -1,
    TopLeft, Top, TopRight,
    Left, Right,
    BottomLeft, Bottom, BottomRight, Body
};

class ShapeHandle{
public:
    ShapeHandle(Shape* shape = nullptr, HandleType handleType = HandleType::None)
        : shape(shape), handleType(handleType){};
    Shape* shape = nullptr;
    HandleType handleType = HandleType::None;
};