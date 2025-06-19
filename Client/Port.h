#pragma once
#include <wx/wx.h>
#include <string>

class MainCanvas;

/**
 * @brief Port 클래스는 도형의 입출력 포트를 나타냅니다.
 *        도형 내부의 상대 위치와 화면상의 절대 위치를 계산하여 시각화할 수 있습니다.
 */
class Port {
private:
    MainCanvas* canvas = nullptr;
public:
    std::string id;                      ///< 포트 ID (연결 식별용)
    wxPoint2DDouble relativePos;         ///< 도형 내에서의 상대 좌표 (0.0 ~ 1.0)
    mutable wxPoint cachedScreenPos;     ///< 화면상의 절대 좌표 (렌더링용 캐시)

    /**
     * @brief 생성자
     * @param id 포트 고유 ID
     * @param relPos 도형 내부 상대 위치
     */
    Port(MainCanvas* canvas, const std::string& id, const wxPoint2DDouble& relPos);

    /**
     * @brief 포트의 화면 좌표 계산 및 캐시
     * @param shapePos 도형의 위치
     * @param width 도형 너비
     * @param height 도형 높이
     * @param scale 확대/축소 배율
     * @param offset 화면 오프셋
     * @return 실제 화면 좌표
     */
    wxPoint GetScreenPosition(const wxPoint2DDouble& shapePos, double width, double height, double scale, const wxPoint2DDouble& offset) const;

    /**
     * @brief 포트를 화면에 원 형태로 그리기
     * @param dc DC 객체
     * @param screenPos 화면 좌표
     */
    void Draw(wxDC& dc, const wxPoint& screenPos) const;
};
