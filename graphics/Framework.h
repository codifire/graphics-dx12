#pragma once

class Framework
{
public:
    Framework(UINT width, UINT height);
    virtual ~Framework() {}

    virtual void Init() = 0;
    virtual void Update() = 0;
    virtual void Render() = 0;
    virtual void Release() = 0;
    virtual void Resize(UINT32 width, UINT32 height) {};
    virtual void KeyDown(UINT8 /*key*/) {}
    virtual void KeyUp(UINT8 /*key*/) {}

    // Accessors.
    bool HasInitialized() { return m_initialized; }
    UINT GetWidth() const { return m_width; }
    UINT GetHeight() const { return m_height; }
    float GetAspectRatio() const { return m_aspectRatio; }

    void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc);

protected:
    void SetWidthHeight(UINT w, UINT h);
    void SetInitialized() { m_initialized = true; }

private:
    // Viewport dimensions.
    UINT m_width;
    UINT m_height;
    float m_aspectRatio;
    
    bool m_initialized{ false };
};


inline void Framework::SetWidthHeight(UINT w, UINT h)
{
    m_width = w;
    m_height = h;
    m_aspectRatio = (static_cast<float>(w) / static_cast<float>(h));
}