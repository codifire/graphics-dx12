#pragma once
#include "Framework.h"
#include "graphics.h"

class Framework_DX12 : public Framework
{
public:
    Framework_DX12(UINT width, UINT height, bool useWarpDevice=false);
    ~Framework_DX12();

    virtual void Init() override;
    virtual void Update() override;
    virtual void Render() override;
    virtual void Destroy() override;

private:
    static const UINT FrameBufferCount = 2;

    // Pipeline objects.
    ComPtr<ID3D12GraphicsCommandList> m_commandList; // generally varies w.r.t number of threads recording drawing commands
    ComPtr<ID3D12CommandAllocator> m_commandAllocator; // a command allocator cannot be reused unless all of the commands that have been 
                                                       // recorded into the command allocator have finished executing on the GPU.
    // There must be at least one command allocator per render frame that is "in-flight" (at least one per command-list, per back buffer of the swap chain).

    ComPtr<ID3D12CommandQueue> m_commandQueue;
    
    ComPtr<ID3D12Device> m_device; // display adapter. a system can also have a software display adapter that emulates 3D hardware functionality.
    ComPtr<IDXGISwapChain4> m_swapChain;
    ComPtr<ID3D12Resource> m_backBuffers[FrameBufferCount]; // are basically textures (or render targets)
    ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap; // a "view" is a synonym for "descriptor". view (or descriptors) describe the resource to the GPU
    // since the swap chain contains multiple back buffer textures, one descriptor is needed to describe each back buffer texture.

    // the size of a descriptor in a descriptor heap is vendor specific(Intel, NVidia, and AMD may store descriptors differently).
    // in order to correctly offset the index into the descriptor heap, the size of a single element in the descriptor heap needs 
    // to be queried during initialization.
    UINT m_rtvDescriptorSize;

    ComPtr<ID3D12PipelineState> m_pipelineState;

    // Synchronization objects.
    UINT m_CurrentBackBufferIndex; // store the index of the current back buffer of the swap chain.
    
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;
    HANDLE m_fenceEvent;
    
    // Window rectangle (used to toggle fullscreen state).
    RECT g_WindowRect;
    bool m_useFullScreen;

    bool m_vSyncEnabled; // controls whether the swap chain's present method should wait for the next vertical refresh before presenting the rendered image to the screen.
    bool m_supportTearing;

    bool m_useWarpDevice; // controls whether to use a software rasterizer (Windows Advanced Rasterization Platform - WARP) or not
};