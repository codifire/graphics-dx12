#pragma once
#include "Framework.h"
#include "graphics.h"
#include <chrono>

class Framework_DX12 : public Framework
{
protected:
    using D3D12DeviceInterface = ID3D12Device2;
    using D3D12CommandQueueInterface = ID3D12CommandQueue;
    using D3D12DescriptorHeapInterface = ID3D12DescriptorHeap;

    using DXGIAdapterInterface = IDXGIAdapter4;
    using DXGISwapChainInterface = IDXGISwapChain4;
    using DXGIFactoryInterface = IDXGIFactory6;
    
public:
    Framework_DX12(UINT width, UINT height, bool useWarpDevice=false);
    ~Framework_DX12();

    virtual void Init() override;
    virtual void Update() override;
    virtual void Render() override;
    virtual void Release() override;
    virtual void Resize(UINT32 width, UINT32 height) override;

    void EnableDebugLayer() const;
    ComPtr<D3D12DeviceInterface> CreateDevice(ComPtr<DXGIAdapterInterface> adapter) const;
    ComPtr<D3D12CommandQueueInterface> CreateCommandQueue(ComPtr<D3D12DeviceInterface> device, D3D12_COMMAND_LIST_TYPE type) const;
    ComPtr<DXGISwapChainInterface> CreateSwapChain(HWND hWnd, ComPtr<DXGIFactoryInterface> dxgiFactory, ComPtr<ID3D12CommandQueue> commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount, bool supportTearing) const;
    ComPtr<D3D12DescriptorHeapInterface> CreateDescriptorHeap(ComPtr<D3D12DeviceInterface> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors) const;
    void UpdateRenderTargetViews(ComPtr<D3D12DeviceInterface> device, ComPtr<DXGISwapChainInterface> swapChain, ComPtr<D3D12DescriptorHeapInterface> descriptorHeap, UINT nFrameBuffer);
    ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(ComPtr<D3D12DeviceInterface> device, D3D12_COMMAND_LIST_TYPE type) const;
    ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ComPtr<D3D12DeviceInterface> device, ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type) const;
    ComPtr<ID3D12Fence> CreateFence(ComPtr<D3D12DeviceInterface> device) const;
    UINT64 SignalFenceGPU(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, UINT64& fenceValue) const;
    void WaitForFenceValue(ComPtr<ID3D12Fence> fence, uint64_t targetFenceValue, HANDLE fenceEvent, std::chrono::milliseconds duration = std::chrono::milliseconds::max()) const;
    void FlushGPUCommandQueue(ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, UINT64& fenceValue, HANDLE fenceEvent) const;

    static HANDLE CreateEventHandle();

    protected:
    static const UINT FrameBufferCount { 4 };

    // Pipeline objects.
    ComPtr<ID3D12GraphicsCommandList> m_commandList; // generally varies w.r.t number of threads recording drawing commands
    ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameBufferCount]; // a command allocator cannot be reused unless all of the commands that have been 
                                                       // recorded into the command allocator have finished executing on the GPU.
    // There must be at least one command allocator per render frame that is "in-flight" (at least one per command-list, per back buffer of the swap chain).

    ComPtr<ID3D12CommandQueue> m_commandQueue;
    
    ComPtr<D3D12DeviceInterface> m_device; // display adapter. a system can also have a software display adapter that emulates 3D hardware functionality.
    ComPtr<DXGISwapChainInterface> m_swapChain;
    ComPtr<ID3D12Resource> m_backBuffers[FrameBufferCount]; // are basically textures (or render targets)
    UINT m_currentBackBufferIndex{ 0 }; // store the index of the current back buffer of the swap chain.

    ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap; // a "view" is a synonym for "descriptor". view (or descriptors) describe the resource to the GPU
    // since the swap chain contains multiple back buffer textures, one descriptor is needed to describe each back buffer texture.

    // the size of a descriptor in a descriptor heap is vendor specific(Intel, NVidia, and AMD may store descriptors differently).
    // in order to correctly offset the index into the descriptor heap, the size of a single element in the descriptor heap needs 
    // to be queried during initialization.
    UINT m_rtvDescriptorSize { 0 };

    ComPtr<ID3D12PipelineState> m_pipelineState;

    // Synchronization objects.
    // Each thread or GPU queue should have at least one fence object and a corresponding fence value.
    // The same fence object should not be signaled from more than one thread or GPU queue but more than one thread or queue can wait on the same fence to be signaled.
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue { 0 };
    uint64_t m_fenceValuesPerFrame[FrameBufferCount] = {};
    HANDLE m_fenceEvent { }; // An OS event handle is used to allow the CPU thread to wait until the fence has been signaled with a particular value.

    bool m_vSyncEnabled { false }; // controls whether the swap chain's present method should wait for the next vertical refresh before presenting the rendered image to the screen.
    bool m_supportTearing { false };

    bool m_useWarpDevice { false }; // controls whether to use a software rasterizer (Windows Advanced Rasterization Platform - WARP) or not

    bool m_enableDebugLayer { CONF_BOOL_ENABLE_D3D_DEBUG_LAYER };
};
