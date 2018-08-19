#pragma once

class Framework
{
public:
    Framework(UINT width, UINT height);
    virtual ~Framework() {}

    virtual void Init() = 0;
    virtual void Update() = 0;
    virtual void Render() = 0;
    virtual void Destroy() = 0;

    // Samples override the event handlers to handle specific messages.
    virtual void KeyDown(UINT8 /*key*/) {}
    virtual void KeyUp(UINT8 /*key*/) {}

    // Accessors.
    UINT GetWidth() const { return m_width; }
    UINT GetHeight() const { return m_height; }
    float GetAspectRatio() const { return m_aspectRatio; }

    void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc);

private:
    // Viewport dimensions.
    UINT m_width;
    UINT m_height;
    float m_aspectRatio;
};